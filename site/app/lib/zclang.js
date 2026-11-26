/**
 * Independent browser driver for the Zlang WebAssembly toolchain.
 *
 * It owns three concerns: an in-memory filesystem backed by Emscripten's FS,
 * the clang/lld invocation sequence, and a minimal WASI host for running the
 * produced module. No project-specific compiler source is reused here.
 */

const OCTAL = /^[0-7]+$/;

function text(bytes, start, end) {
  let value = '';
  for (let index = start; index < end; index += 1) {
    const byte = bytes[index];
    if (byte === 0) break;
    value += String.fromCharCode(byte);
  }
  return value;
}

function octal(bytes, start, end) {
  const raw = text(bytes, start, end).trim();
  return OCTAL.test(raw) ? Number.parseInt(raw, 8) : 0;
}

export function readTar(buffer) {
  const bytes = new Uint8Array(buffer);
  const entries = [];
  let offset = 0;

  while (offset + 512 <= bytes.length) {
    const header = bytes.subarray(offset, offset + 512);
    if (header.every(byte => byte === 0)) break;

    const rawName = text(header, 0, 100);
    const name = rawName.replace(/^\.\//, '').replace(/\/$/, '');
    const size = octal(header, 124, 136);
    const type = String.fromCharCode(header[156]);
    const data = bytes.slice(offset + 512, offset + 512 + size);

    if (name && (type === '0' || type === '\0')) entries.push({ name, data });
    offset += 512 + Math.ceil(size / 512) * 512;
  }

  return entries;
}

function ensureParents(fs, path) {
  const parts = path.split('/').filter(Boolean);
  parts.pop();
  let current = '';
  for (const part of parts) {
    current += `/${part}`;
    try {
      fs.mkdir(current);
    } catch {
      // The directory already exists.
    }
  }
}

function writeFile(fs, path, data) {
  ensureParents(fs, path);
  fs.writeFile(path, data);
}

function installSysroot(fs, entries) {
  for (const entry of entries) writeFile(fs, `/${entry.name}`, entry.data);
}

function moduleOptions(module, options, log, program) {
  return {
    thisProgram: program,
    noExitRuntime: true,
    locateFile: path => options.base ? `${options.base}${path}` : path,
    print: line => log.push(line),
    printErr: line => log.push(line),
    wasmBinary: module ? module.slice(0) : undefined,
  };
}

class WasiExit extends Error {
  constructor(code) {
    super(`program exited with ${code}`);
    this.code = code;
  }
}

function createWasi({ args, stdout, stderr }) {
  const encoder = new TextEncoder();
  const memory = { view: null };

  function writeString(ptr, value) {
    const bytes = encoder.encode(`${value}\0`);
    memory.view.set(bytes, ptr);
  }

  const wasi = {
    args_sizes_get(argcPtr, sizePtr) {
      const values = ['main', ...args];
      new DataView(memory.view.buffer).setUint32(argcPtr, values.length, true);
      new DataView(memory.view.buffer).setUint32(sizePtr, values.reduce((sum, value) => sum + value.length + 1, 0), true);
      return 0;
    },
    args_get(argvPtr, bufPtr) {
      const values = ['main', ...args];
      const view = new DataView(memory.view.buffer);
      let cursor = bufPtr;
      values.forEach((value, index) => {
        view.setUint32(argvPtr + index * 4, cursor, true);
        writeString(cursor, value);
        cursor += value.length + 1;
      });
      return 0;
    },
    environ_sizes_get(countPtr, sizePtr) {
      const view = new DataView(memory.view.buffer);
      view.setUint32(countPtr, 0, true);
      view.setUint32(sizePtr, 0, true);
      return 0;
    },
    environ_get() { return 0; },
    fd_write(fd, iovsPtr, iovsLen, writtenPtr) {
      const view = new DataView(memory.view.buffer);
      let total = 0;
      for (let index = 0; index < iovsLen; index += 1) {
        const ptr = view.getUint32(iovsPtr + index * 8, true);
        const length = view.getUint32(iovsPtr + index * 8 + 4, true);
        const chunk = new TextDecoder().decode(memory.view.subarray(ptr, ptr + length));
        if (fd === 1) stdout(chunk);
        else if (fd === 2) stderr(chunk);
        total += length;
      }
      view.setUint32(writtenPtr, total, true);
      return 0;
    },
    fd_fdstat_get(fd, statPtr) {
      const view = new DataView(memory.view.buffer);
      view.setUint8(statPtr, 2); // character device
      view.setUint16(statPtr + 2, 0, true);
      const rights = fd === 0 ? 1n << 1n : 1n << 6n;
      view.setBigUint64(statPtr + 8, rights, true);
      view.setBigUint64(statPtr + 16, 0n, true);
      return 0;
    },
    fd_close() { return 0; },
    fd_seek() { return 8; },
    fd_read() { return 8; },
    proc_exit(code) { throw new WasiExit(code); },
    random_get(ptr, length) {
      crypto.getRandomValues(memory.view.subarray(ptr, ptr + length));
      return 0;
    },
    clock_time_get(clockId, precision, timePtr) {
      const nanos = BigInt(Date.now()) * 1000000n;
      new DataView(memory.view.buffer).setBigUint64(timePtr, nanos, true);
      return 0;
    },
  };

  return {
    imports: { wasi_snapshot_preview1: wasi },
    bind(instance) {
      memory.view = new Uint8Array(instance.exports.memory.buffer);
    },
  };
}

export async function createZlangToolchain(options = {}) {
  const log = [];
  const sysrootEntries = readTar(options.sysroot);

  async function freshModules() {
    const clang = await options.clangModule(moduleOptions(options.clangWasm, options, log, 'clang'));
    const lld = await options.lldModule(moduleOptions(options.lldWasm, options, log, 'wasm-ld'));
    installSysroot(clang.FS, sysrootEntries);
    installSysroot(lld.FS, sysrootEntries);
    return { clang, lld };
  }

  function compileSource(modules, source, defines = []) {
    const { clang } = modules;
    writeFile(clang.FS, '/main.c', new TextEncoder().encode(source));
    const args = [
      'clang', '-cc1', '-emit-obj', '-fno-common', '-Oz', '-x', 'c',
      '-D_WASI_EMULATED_MMAN', '-D_WASI_EMULATED_SIGNAL', '-D_POSIX_C_SOURCE=200809L',
      ...defines.map(define => `-D${define}`),
      '-isystem', '/include',
      '-isystem', '/include/wasm32-wasi',
      '-internal-isystem', '/lib/clang/20/include',
      '-o', 'main.o', 'main.c',
    ];
    const status = clang.callMain(args.slice(1));
    if (status !== 0) throw new Error(`clang failed with ${status}`);
    return clang.FS.readFile('/main.o');
  }

  function linkObjects(modules, objects, outputName = 'app.wasm') {
    const { lld } = modules;
    for (const [name, data] of objects) writeFile(lld.FS, `/${name}`, data);
    const args = [
      'wasm-ld', '--export-dynamic',
      '-z', 'stack-size=1048576',
      '-Llib/wasm32-wasi', 'lib/wasm32-wasi/crt1.o',
      ...objects.map(([name]) => name),
      '-lc', '-lwasi-emulated-signal', '-o', outputName,
    ];
    const status = lld.callMain(args.slice(1));
    if (status !== 0) throw new Error(`lld failed with ${status}`);
    return lld.FS.readFile(`/${outputName}`);
  }

  async function run(wasm, runOptions = {}) {
    const stdout = runOptions.stdout || (() => {});
    const stderr = runOptions.stderr || (() => {});
    const host = createWasi({ args: runOptions.args || [], stdout, stderr });
    const { instance } = await WebAssembly.instantiate(wasm, host.imports);
    host.bind(instance);
    try {
      instance.exports._start();
      return 0;
    } catch (error) {
      if (error instanceof WasiExit) return error.code;
      throw error;
    }
  }

  return {
    async compile(source, defines) {
      const modules = await freshModules();
      const object = compileSource(modules, source, defines);
      return linkObjects(modules, [['main.o', object]]);
    },
    run,
    log,
  };
}
