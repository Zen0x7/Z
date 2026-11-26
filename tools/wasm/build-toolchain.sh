#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=tools/wasm/common.sh
source "$SCRIPT_DIR/common.sh"

wasm_paths

if [[ ! -d "$ZLANG_WASM_ROOT/src/llvm-project/.git" ]]; then
  git clone --depth 1 --branch "$LLVM_TAG" \
    https://github.com/llvm/llvm-project.git \
    "$ZLANG_WASM_ROOT/src/llvm-project"
fi

wasm_emsdk_env

LLVM_SRC="$ZLANG_WASM_ROOT/src/llvm-project"
LLVM_BUILD="$LLVM_SRC/build-wasm"

emcmake cmake -S "$LLVM_SRC/llvm" -B "$LLVM_BUILD" -GNinja \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCMAKE_C_FLAGS="-msimd128 -mbulk-memory" \
  -DCMAKE_CXX_FLAGS="-msimd128 -mbulk-memory" \
  -DCMAKE_EXE_LINKER_FLAGS="\
-s NO_INVOKE_RUN \
-s EXIT_RUNTIME \
-s ENVIRONMENT=worker \
-s STACK_SIZE=4194304 \
-s INITIAL_HEAP=134217728 \
-s ALLOW_MEMORY_GROWTH \
-s MODULARIZE \
-s EXPORT_ES6 \
-s EXPORT_NAME=createWasmToolModule \
-s MALLOC=dlmalloc \
-s EXPORTED_RUNTIME_METHODS=FS,callMain" \
  -DLLVM_ENABLE_PROJECTS="clang;lld" \
  -DLLVM_HOST_TRIPLE="wasm32-unknown-emscripten" \
  -DLLVM_DEFAULT_TARGET_TRIPLE="wasm32-unknown-wasi" \
  -DLLVM_TARGETS_TO_BUILD="WebAssembly" \
  -DLLVM_ENABLE_THREADS=OFF \
  -DLLVM_BUILD_TOOLS=OFF \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_INCLUDE_EXAMPLES=OFF \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DCLANG_ENABLE_ARCMT=OFF \
  -DCLANG_ENABLE_STATIC_ANALYZER=OFF

cmake --build "$LLVM_BUILD" --target clang lld -j "$JOBS"

cp "$LLVM_BUILD/bin/clang.js" "$ZLANG_WASM_ROOT/dist/clang.js"
cp "$LLVM_BUILD/bin/clang.wasm" "$ZLANG_WASM_ROOT/dist/clang.wasm"
cp "$LLVM_BUILD/bin/lld.js" "$ZLANG_WASM_ROOT/dist/lld.js"
cp "$LLVM_BUILD/bin/lld.wasm" "$ZLANG_WASM_ROOT/dist/lld.wasm"

"$SCRIPT_DIR/package-sysroot.sh"

echo "toolchain ready in $ZLANG_WASM_ROOT/dist"
