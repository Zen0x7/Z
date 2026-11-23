#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
tmp_dir=$(mktemp -d)
trap 'rm -rf "$tmp_dir"' EXIT

compilers=("$@")
if ((${#compilers[@]} == 0)); then
    compilers=(clang gcc)
fi

while IFS= read -r header; do
    relative=${header#"$root/include/"}
    printf '#include "%s"\ntypedef int zen_header_probe_t;\n' "$relative" > "$tmp_dir/probe.c"
    for compiler in "${compilers[@]}"; do
        "$compiler" -std=c99 -I"$root/include" \
            -Wall -Wextra -Wpedantic -Werror -fsyntax-only "$tmp_dir/probe.c"
    done
    printf 'ok %s\n' "$relative"
done < <(find "$root/include" -type f -name '*.h' | sort)
