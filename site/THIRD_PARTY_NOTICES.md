# Third-party notices

The browser playground ships WebAssembly toolchain artifacts built from upstream projects. Their licenses remain with their respective owners.

## LLVM 20.1.2

`clang.wasm` and `lld.wasm` are built from `llvm-project`, licensed under the Apache License 2.0 with LLVM exceptions. See <https://llvm.org/LICENSE.txt>.

## Emscripten 4.0.23

The Emscripten toolchain generates the JavaScript glue and the WebAssembly builds of clang and lld. Emscripten is licensed under the University of Illinois/NCSA Open Source License and MIT. See <https://emscripten.org/docs/introducing_emscripten/about_emscripten.html#license>.

## wasi-sdk 25.0 / wasi-libc

`sysroot.tar` contains headers and libraries from wasi-sdk and wasi-libc. wasi-libc is licensed under the MIT License; wasi-sdk includes components under their own upstream licenses. See <https://github.com/WebAssembly/wasi-sdk> and <https://github.com/WebAssembly/wasi-libc>.

The sysroot also contains Zlang public headers from this repository, copied into `include/zlang/`.

## Zlang code

The compiler wrapper (`zc.wasm`), the driver (`zclang.js`) and the playground page are written for this project. No third-party driver source is redistributed.
