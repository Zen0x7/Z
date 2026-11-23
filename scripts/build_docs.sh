#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
version=1.18.0
archive_sha256=14fa81bdc34171edb5f1f02b1d60e74802f0439b77fa44e592565d517d72df90
archive_url="https://github.com/doxygen/doxygen/releases/download/Release_${version}/doxygen-${version}.linux.bin.tar.gz"
cache_dir="${root}/build/.doxygen"
archive="${cache_dir}/doxygen.tar.gz"
doxygen_bin=${ZEN_DOXYGEN:-}

if [[ -z "${doxygen_bin}" ]]; then
    if command -v doxygen >/dev/null 2>&1 &&
       [[ "$(doxygen --version)" == "${version}" ]]; then
        doxygen_bin=$(command -v doxygen)
    else
        mkdir -p "${cache_dir}"
        if [[ ! -f "${archive}" ]] ||
           ! printf '%s  %s\n' "${archive_sha256}" "${archive}" | LC_ALL=C sha256sum -c --status; then
            curl -fsSL --retry 3 "${archive_url}" -o "${archive}"
        fi
        printf '%s  %s\n' "${archive_sha256}" "${archive}" | LC_ALL=C sha256sum -c -
        tar -xzf "${archive}" -C "${cache_dir}" --strip-components=2 \
            "doxygen-${version}/bin/doxygen"
        chmod +x "${cache_dir}/doxygen"
        doxygen_bin="${cache_dir}/doxygen"
    fi
fi

mkdir -p "${root}/build/docs"
cd "${root}"
"${doxygen_bin}" Doxyfile
printf 'Documentation written to build/docs/html/index.html\n'
