#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=tools/wasm/common.sh
source "$SCRIPT_DIR/common.sh"

wasm_paths
wasm_emsdk_env

REPO="$ZLANG_REPO"
DIST="$ZLANG_WASM_ROOT/dist"

CORE_SOURCES=(
  "$REPO/compiler/src/diagnostics.c"
  "$REPO/compiler/src/emitter.c"
  "$REPO/compiler/src/generics.c"
  "$REPO/compiler/src/lexer.c"
  "$REPO/compiler/src/optimizer.c"
  "$REPO/compiler/src/parser.c"
  "$REPO/compiler/src/planner.c"
  "$REPO/compiler/src/semantics.c"
  "$REPO/compiler/src/stdlib.c"
)

emcc -std=c99 -O2 \
  -I"$REPO/compiler/include" \
  -s MODULARIZE \
  -s EXPORT_ES6 \
  -s EXPORT_NAME=createZlangCompiler \
  -s ENVIRONMENT=worker \
  -s ALLOW_MEMORY_GROWTH \
  -s STACK_SIZE=4194304 \
  -s INITIAL_HEAP=134217728 \
  -s ASSERTIONS=0 \
  -s EXPORTED_FUNCTIONS='["_malloc","_free","_zlang_wasm_compile","_zlang_wasm_version"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","UTF8ToString","stringToUTF8","HEAPU8"]' \
  "$SCRIPT_DIR/zc_wasm.c" "${CORE_SOURCES[@]}" \
  -o "$DIST/zc.js"

echo "zc wasm ready in $DIST/zc.js and $DIST/zc.wasm"
