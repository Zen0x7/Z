#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=tools/wasm/common.sh
source "$SCRIPT_DIR/common.sh"

wasm_paths

DOWNLOADS="$ZLANG_WASM_ROOT/downloads"
SYSROOT="$DOWNLOADS/sysroot"
DIST="$ZLANG_WASM_ROOT/dist"
LLVM_BUILD="$ZLANG_WASM_ROOT/src/llvm-project/build-wasm"

SYSROOT_TAR="wasi-sysroot-${WASI_SDK_VERSION}.tar.gz"
BUILTINS_TAR="libclang_rt.builtins-wasm32-wasi-${WASI_SDK_VERSION}.tar.gz"
BASE_URL="https://github.com/WebAssembly/wasi-sdk/releases/download/${WASI_SDK_TAG}"

cd "$DOWNLOADS"

if [[ ! -f "$SYSROOT_TAR" ]]; then
  curl -fL -o "$SYSROOT_TAR" "$BASE_URL/$SYSROOT_TAR"
fi

if [[ ! -f "$BUILTINS_TAR" ]]; then
  curl -fL -o "$BUILTINS_TAR" "$BASE_URL/$BUILTINS_TAR"
fi

rm -rf "$SYSROOT"
tar -xzf "$SYSROOT_TAR"
mv "wasi-sysroot-${WASI_SDK_VERSION}" "$SYSROOT"

rm -rf "$DOWNLOADS/builtins"
mkdir -p "$DOWNLOADS/builtins"
tar -xzf "$BUILTINS_TAR" -C "$DOWNLOADS/builtins"
BUILTINS_LIB="$(find "$DOWNLOADS/builtins" -name 'libclang_rt.builtins*.a' -print -quit)"

if [[ -z "$BUILTINS_LIB" ]]; then
  echo "wasi builtins archive not found in $BUILTINS_TAR" >&2
  exit 1
fi

for _target in wasm32-wasi-threads wasm32-wasi-p1 wasm32-wasi-p1-threads wasm32-wasi-p2; do
  rm -rf "$SYSROOT/include/$_target" "$SYSROOT/lib/$_target"
done
rm -rf "$SYSROOT/share" "$SYSROOT/lib/wasm32-wasi/llvm-lto"

mkdir -p "$SYSROOT/lib/clang/$LLVM_MAJOR/lib/wasm32-unknown-wasi"
cp -R "$LLVM_BUILD/lib/clang/$LLVM_MAJOR/include" "$SYSROOT/lib/clang/$LLVM_MAJOR/include"
cp "$BUILTINS_LIB" "$SYSROOT/lib/clang/$LLVM_MAJOR/lib/wasm32-unknown-wasi/libclang_rt.builtins.a"

mkdir -p "$SYSROOT/include/zlang"
cp -R "$ZLANG_REPO/include/zlang/." "$SYSROOT/include/zlang/"

printf 'zlang wasm sysroot\nwasi-sdk: %s\nllvm: %s\n' \
  "$WASI_SDK_VERSION" "$LLVM_TAG" > "$SYSROOT/ZLANG_SYSROOT"

tar -czf "$DIST/sysroot.tar.gz" -C "$SYSROOT" .
echo "sysroot packaged: $DIST/sysroot.tar.gz"
