#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
tmp_dir=$(mktemp -d)
trap 'rm -rf "$tmp_dir"' EXIT
generated_include="$tmp_dir/generated/include"
cmake -DZLANG_ROOT="$root" -DZLANG_OUTPUT="$generated_include" \
    -P "$root/scripts/configure_version_header.cmake"

compilers=("$@")
if ((${#compilers[@]} == 0)); then
    compilers=(clang gcc)
fi

while IFS= read -r header; do
    relative=${header#"$root/include/"}
    printf '#include "%s"\ntypedef int zlang_header_probe_t;\n' "$relative" > "$tmp_dir/probe.c"
    for compiler in "${compilers[@]}"; do
        "$compiler" -std=c99 -I"$generated_include" -I"$root/include" \
            -Wall -Wextra -Wpedantic -Werror -fsyntax-only "$tmp_dir/probe.c"
    done
    printf 'ok %s\n' "$relative"
done < <({ find "$root/include" -type f -name '*.h';
           find "$generated_include" -type f -name '*.h'; } | sort)
