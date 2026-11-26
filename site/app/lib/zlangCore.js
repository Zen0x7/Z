/**
 * Toolchain runner intended to run inside a Web Worker.
 *
 * Toolchain assets are loaded only on the first run. Clang and lld are created
 * fresh for each compilation because their Emscripten modules are not
 * reentrant after `callMain`.
 */

const BASE = '/compiler/'
const OUTPUT_CAPACITY = 262144
const DIAGNOSTIC_CAPACITY = 8192
const MIN_COMPILE_MS = 3000

let loader = null

async function fetchBytes(path) {
  const response = await fetch(path)
  if (!response.ok) throw new Error(`cannot load ${path}`)
  return response.arrayBuffer()
}

async function fetchSysroot(path) {
  const compressed = await fetchBytes(path)
  const stream = new Blob([compressed]).stream().pipeThrough(new DecompressionStream('gzip'))
  return new Response(stream).arrayBuffer()
}

async function load() {
  if (loader) return loader
  loader = (async () => {
    const [zcModule, clangModule, lldModule, zclang] = await Promise.all([
      import(/* @vite-ignore */ `${BASE}zc.js`),
      import(/* @vite-ignore */ `${BASE}clang.js`),
      import(/* @vite-ignore */ `${BASE}lld.js`),
      import('./zclang'),
    ])

    const [zcWasm, clangWasm, lldWasm, sysroot] = await Promise.all([
      fetchBytes(`${BASE}zc.wasm`),
      fetchBytes(`${BASE}clang.wasm`),
      fetchBytes(`${BASE}lld.wasm`),
      fetchSysroot(`${BASE}sysroot.tar.gz`),
    ])

    const zc = await zcModule.default({ wasmBinary: zcWasm })
    const toolchain = await zclang.createZlangToolchain({
      clangModule: clangModule.default,
      lldModule: lldModule.default,
      clangWasm,
      lldWasm,
      sysroot,
      base: BASE,
    })

    return { zc, toolchain }
  })()
  return loader
}

function emitC(zc, source) {
  const output = zc._malloc(OUTPUT_CAPACITY)
  const diagnostics = zc._malloc(DIAGNOSTIC_CAPACITY)
  const input = zc._malloc(source.length + 1)
  zc.stringToUTF8(source, input, source.length + 1)

  const status = zc.ccall(
    'zlang_wasm_compile',
    'number',
    ['number', 'number', 'number', 'number', 'number', 'number'],
    [input, 0, output, OUTPUT_CAPACITY, diagnostics, DIAGNOSTIC_CAPACITY],
  )
  const messages = zc.UTF8ToString(diagnostics).trim()
  if (status !== 0) throw new Error(messages || 'Zlang compilation failed')
  return { cSource: zc.UTF8ToString(output), messages }
}

export async function runZlangCore(source, onStage = () => {}) {
  const startedAt = Date.now()
  onStage('compiling')

  const { zc, toolchain } = await load()
  const { cSource, messages } = emitC(zc, source)
  const wasm = await toolchain.compile(cSource)

  let output = ''
  const code = await toolchain.run(wasm, {
    stdout: chunk => { output += chunk },
    stderr: chunk => { output += chunk },
  })

  const remaining = MIN_COMPILE_MS - (Date.now() - startedAt)
  if (remaining > 0) {
    await new Promise(resolve => setTimeout(resolve, remaining))
  }

  const log = [
    ...(messages ? messages.split('\n') : []),
    ...toolchain.log,
    `app.wasm: ${wasm.length} bytes`,
    `exit: ${code}`,
  ]
  return { output, log, wasm }
}
