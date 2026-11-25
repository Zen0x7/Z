<p align="center"><a href="https://github.com/Zen0x7/Z" target="_blank"><img src="docs/logo.svg" width="400" alt="Zlang"></a></p>

<p align="center">
<a href="https://codecov.io/gh/Zen0x7/Z"><img src="https://codecov.io/gh/Zen0x7/Z/branch/master/graph/badge.svg?token=8TUYTBYEBJ" alt="Coverage"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/ci.yml"><img src="https://github.com/Zen0x7/Z/actions/workflows/ci.yml/badge.svg?branch=master" alt="CI"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/codeql.yml"><img src="https://github.com/Zen0x7/Z/actions/workflows/codeql.yml/badge.svg?branch=master" alt="CodeQL"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/dependabot/dependabot-updates"><img src="https://github.com/Zen0x7/Z/actions/workflows/dependabot/dependabot-updates/badge.svg?branch=master" alt="Dependabot Updates"></a>
</p>

Zlang is a small source-to-source language for bounded, deterministic programs. You write
`.z` source, `zc` emits readable C99, and your normal C toolchain builds the
executable. The runtime is header-only.

There is no VM, bytecode, garbage collector or hidden runtime allocator. Types, capacities and
storage decisions are resolved by the compiler.

## Quick start

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j
```

Create `hello.z`:

```zlang
write("Hello from Zlang");
write("\n");
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

## Design in three lines

- Header-only C99 runtime and no compiled library to install.
- Fixed-shape arrays with compile-time storage and bounds checks that trap on violation.
- Checked arithmetic, bounded strings and explicit `clone` / `reference` parameter qualifiers.

## Explore

- [Executable examples](examples/README.md)
- [Language specification](SPEC.md)
- [Language grammar](docs/grammar.md)
- [Live API reference](https://z.iantorres.cl)
- [API reference source](docs/index.md)
- [Roadmap and locked decisions](ROADMAP.md)
- [Contributor guide](AGENTS.md)
