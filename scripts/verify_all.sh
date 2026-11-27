#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
verify_root="${root}/build-verify"
jobs=${ZLANG_JOBS:-}
if [[ -z "${jobs}" ]]; then
    jobs=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || printf '4')
fi
current_step='startup'

trap 'status=$?; if [[ ${status} -ne 0 ]]; then printf "\nFAILED step: %s (exit %d)\n" "${current_step}" "${status}" >&2; fi' EXIT

step() {
    current_step=$1
    printf '\n== %s ==\n' "${current_step}"
}

require_tools() {
    local tool
    for tool in "$@"; do
        if ! command -v "${tool}" >/dev/null 2>&1; then
            printf 'error: required tool not found: %s\n' "${tool}" >&2
            exit 1
        fi
    done
}

configure_and_build() {
    local build_dir=$1
    shift
    cmake -Wdev --warn-uninitialized -S "${root}" -B "${build_dir}" -G Ninja "$@"
    cmake --build "${build_dir}" --parallel "${jobs}"
}

run_ctest() {
    local build_dir=$1
    ctest --test-dir "${build_dir}" --output-on-failure --parallel "${jobs}"
}

cd "${root}"

step 'toolchain'
require_tools cmake ninja clang gcc clang-format clang-tidy cppcheck valgrind gcovr
if [[ -z "${ZLANG_DOXYGEN:-}" ]] && ! command -v doxygen >/dev/null 2>&1; then
    require_tools curl sha256sum tar
fi
if [[ "${ZLANG_VERIFY_KEEP_BUILD:-0}" != '1' ]]; then
    rm -rf "${verify_root}"
fi
mkdir -p "${verify_root}"

step 'clang-format'
clang-format --dry-run --Werror \
    include/zlang/*.h include/zlang/*/*.h include/zlang/*/*/*.h \
    compiler/include/zlang/compiler/*.h compiler/src/*.c \
    tests/*/*.c tests/*/*.h fuzz/*.c fuzz/*.h tools/zc/*.c

step 'cppcheck'
cppcheck --enable=warning,style,performance,portability --error-exitcode=1 \
    --inline-suppr --suppress=missingIncludeSystem \
    --suppress=normalCheckLevelMaxBranches --std=c99 \
    -Iinclude -Icompiler/include include compiler tests fuzz tools

step 'clang-tidy'
configure_and_build "${verify_root}/tidy" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_C_CLANG_TIDY=clang-tidy \
    -DCMAKE_C_STANDARD=99 \
    -DCMAKE_C_STANDARD_REQUIRED=ON \
    -DCMAKE_C_EXTENSIONS=OFF

step 'clang --analyze'
generated_include="${verify_root}/generated/include"
"${root}/scripts/configure_generated_files.sh" "${generated_include}"
clang --analyze -std=c99 -DZLANG_SOURCE_DIR="\"${root}\"" \
    -Iinclude -Icompiler/include -I"${generated_include}" \
    -Wall -Wextra -Werror -Xanalyzer -analyzer-output=text \
    tests/version/version_tests.c \
    tests/testing/testing_tests.c \
    tests/consumer/main.c \
    fuzz/version_fuzz.c \
    tests/spec/spec_tests.c

step 'normal build and tests'
configure_and_build "${verify_root}/normal" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_C_STANDARD=99 \
    -DCMAKE_C_STANDARD_REQUIRED=ON \
    -DCMAKE_C_EXTENSIONS=OFF
run_ctest "${verify_root}/normal"

for standard in 11 17 23; do
    step "gcc C${standard} release build and tests"
    configure_and_build "${verify_root}/c${standard}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=gcc \
        -DCMAKE_C_STANDARD="${standard}" \
        -DCMAKE_C_STANDARD_REQUIRED=ON \
        -DCMAKE_C_EXTENSIONS=OFF
    run_ctest "${verify_root}/c${standard}"
done

for sanitizer in address undefined integer; do
    step "sanitizer ${sanitizer}"
    configure_and_build "${verify_root}/sanitizer-${sanitizer}" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_C_STANDARD=99 \
        -DCMAKE_C_STANDARD_REQUIRED=ON \
        -DCMAKE_C_EXTENSIONS=OFF \
        -DZLANG_SANITIZER="${sanitizer}"
    run_ctest "${verify_root}/sanitizer-${sanitizer}"
done

step 'valgrind memcheck'
configure_and_build "${verify_root}/valgrind" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_C_STANDARD=99 \
    -DCMAKE_C_STANDARD_REQUIRED=ON \
    -DCMAKE_C_EXTENSIONS=OFF
shopt -s nullglob
valgrind_binaries=("${verify_root}"/valgrind/tests/*/zlang_*_tests)
shopt -u nullglob
if [[ ${#valgrind_binaries[@]} -eq 0 ]]; then
    printf 'error: no valgrind test binaries found\n' >&2
    exit 1
fi
for binary in "${valgrind_binaries[@]}"; do
    valgrind --error-exitcode=99 --leak-check=full --show-leak-kinds=all \
        --errors-for-leak-kinds=all "${binary}"
done

step 'coverage'
cmake -Wdev --warn-uninitialized -S "${root}" -B "${verify_root}/coverage" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_C_STANDARD=99 \
    -DCMAKE_C_STANDARD_REQUIRED=ON \
    -DCMAKE_C_EXTENSIONS=OFF \
    -DZLANG_COVERAGE=ON
cmake --build "${verify_root}/coverage" --target zlang_coverage --parallel "${jobs}"

step 'header isolation'
"${root}/scripts/check_headers.sh" clang gcc

step 'install and package consumer'
install_dir="${verify_root}/install"
configure_and_build "${verify_root}/package" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_C_STANDARD=99 \
    -DCMAKE_C_STANDARD_REQUIRED=ON \
    -DCMAKE_C_EXTENSIONS=OFF \
    -DZLANG_BUILD_TESTS=OFF
cmake --install "${verify_root}/package" --prefix "${install_dir}"
cmake -Wdev --warn-uninitialized -S "${root}/tests/consumer" -B "${verify_root}/consumer" -G Ninja \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_C_STANDARD=99 \
    -DCMAKE_C_STANDARD_REQUIRED=ON \
    -DCMAKE_C_EXTENSIONS=OFF \
    -DCMAKE_PREFIX_PATH="${install_dir}"
cmake --build "${verify_root}/consumer" --parallel "${jobs}"
"${verify_root}/consumer/zlang_consumer"

step 'libFuzzer smoke'
configure_and_build "${verify_root}/fuzz" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_C_STANDARD=99 \
    -DCMAKE_C_STANDARD_REQUIRED=ON \
    -DCMAKE_C_EXTENSIONS=OFF \
    -DZLANG_BUILD_TESTS=OFF \
    -DZLANG_BUILD_FUZZERS=ON
shopt -s nullglob
fuzz_targets=("${verify_root}"/fuzz/fuzz/zlang_*_fuzz)
shopt -u nullglob
if [[ ${#fuzz_targets[@]} -eq 0 ]]; then
    printf 'error: no fuzz targets found\n' >&2
    exit 1
fi
for target in "${fuzz_targets[@]}"; do
    "${target}" -max_total_time=5 -rss_limit_mb=2048 -timeout=5
done

step 'Doxygen API reference'
"${root}/scripts/build_docs.sh"

step 'complete'
printf 'All verification checks passed.\n'
