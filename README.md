<p align="center">
  <a href="https://github.com/Zen0x7/Z" target="_blank">
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="site/public/logo-dark.svg">
      <source media="(prefers-color-scheme: light)" srcset="site/public/logo-light.svg">
      <img src="site/public/logo-light.svg" width="400" alt="Zlang">
    </picture>
  </a>
</p>

<p align="center">
<a href="https://codecov.io/gh/Zen0x7/Z"><img src="https://codecov.io/gh/Zen0x7/Z/branch/master/graph/badge.svg?token=8TUYTBYEBJ" alt="Coverage"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/ci.yml"><img src="https://github.com/Zen0x7/Z/actions/workflows/ci.yml/badge.svg?branch=master" alt="CI"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/codeql.yml"><img src="https://github.com/Zen0x7/Z/actions/workflows/codeql.yml/badge.svg?branch=master" alt="CodeQL"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/dependabot/dependabot-updates"><img src="https://github.com/Zen0x7/Z/actions/workflows/dependabot/dependabot-updates/badge.svg?branch=master" alt="Dependabot Updates"></a>
</p>

Zlang is a small source-to-source language for bounded, deterministic programs. You write
`.z` source, `zc` emits readable C99, and your normal C toolchain builds the
executable. The runtime is header-only.

There is no VM, bytecode, garbage collector or hidden runtime allocator. Types, capacities,
storage decisions and parameter modes are resolved by the compiler. The language also has
compile-time generics and native `result_type<T, status_code>` layouts.

The documentation site is the published source of truth for the language:
<https://z.iantorres.cl>. `SPEC.md` and `ROADMAP.md` are working documents where changes
are accumulated before they are merged into the site specification.

## Quick start

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j
```

Create `hello.z`:

```zlang
import { write } from "@std/io";

function main(): int32 {
    write("Hello from Zlang");
    write("\n");

    return 0;
}
```

Then compile and run it:

```sh
./build/tools/zc/zc hello.z
./hello
```

Use `--emit-c` to write only the generated C file:

```sh
./build/tools/zc/zc hello.z --emit-c
```

## Browser playground

The site includes a client-side playground at <https://z.iantorres.cl/playground>.
`zc.wasm` turns Zlang into C, `clang.wasm` and `lld.wasm` compile it to WebAssembly, and a
small WASI host runs it in the browser. No source or binary is sent to a server.

The toolchain artifacts are built from upstream LLVM, Emscripten and wasi-sdk with the
scripts in `tools/wasm/`. They are served as static files from `site/public/compiler/`;
third-party notices live in `site/THIRD_PARTY_NOTICES.md`.

## Design in three lines

- Header-only C99 runtime and no compiled library to install.
- Fixed-shape arrays with compile-time storage and bounds checks that trap on violation.
- Explicit `clone` / `reference` modes, compile-time generics and bounded standard domains.

## Explore

- [Documentation site](https://z.iantorres.cl)
- [Language specification](https://z.iantorres.cl/docs/specification)
- [Browser playground](https://z.iantorres.cl/playground)
- [Executable examples](examples/README.md)
- [Working language notes](SPEC.md)
- [Roadmap and locked decisions](ROADMAP.md)
- [API reference source](docs/index.md)
- [Contributor guide](AGENTS.md)
