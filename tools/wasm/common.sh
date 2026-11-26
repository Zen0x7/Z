#!/usr/bin/env bash
set -euo pipefail

ZLANG_WASM_ROOT="${ZLANG_WASM_ROOT:-/home/ian/dev/z-wasm-toolchain}"
ZLANG_REPO="${ZLANG_REPO:-/home/ian/dev/z}"

EMSDK_VERSION="${EMSDK_VERSION:-4.0.23}"
LLVM_TAG="${LLVM_TAG:-llvmorg-20.1.2}"
WASI_SDK_TAG="${WASI_SDK_TAG:-wasi-sdk-25}"
WASI_SDK_VERSION="${WASI_SDK_VERSION:-25.0}"
LLVM_MAJOR="${LLVM_MAJOR:-20}"

JOBS="${JOBS:-24}"

wasm_paths() {
  mkdir -p "$ZLANG_WASM_ROOT"/{downloads,src,dist}
}

wasm_emsdk_env() {
  if [[ ! -d "$ZLANG_WASM_ROOT/emsdk/.git" ]]; then
    git clone --depth 1 https://github.com/emscripten-core/emsdk.git "$ZLANG_WASM_ROOT/emsdk"
  fi

  (
    cd "$ZLANG_WASM_ROOT/emsdk"
    ./emsdk install "$EMSDK_VERSION"
    ./emsdk activate "$EMSDK_VERSION"
  )

  # shellcheck source=/dev/null
  source "$ZLANG_WASM_ROOT/emsdk/emsdk_env.sh"
}
