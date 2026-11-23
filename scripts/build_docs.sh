#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
version=1.18.0
archive_sha256=14fa81bdc34171edb5f1f02b1d60e74802f0439b77fa44e592565d517d72df90
archive_urls=(
    "https://www.doxygen.nl/files/doxygen-${version}.linux.bin.tar.gz"
    "https://github.com/doxygen/doxygen/releases/download/Release_${version}/doxygen-${version}.linux.bin.tar.gz"
    "https://sourceforge.net/projects/doxygen/files/rel-${version}/doxygen-${version}.linux.bin.tar.gz/download"
)
cache_dir="${root}/build/.doxygen"
archive="${cache_dir}/doxygen.tar.gz"
doxygen_bin=${ZLANG_DOXYGEN:-}

if [[ -z "${doxygen_bin}" ]]; then
    if command -v doxygen >/dev/null 2>&1 &&
       [[ "$(doxygen --version)" == "${version}" ]]; then
        doxygen_bin=$(command -v doxygen)
    else
        mkdir -p "${cache_dir}"
        if [[ ! -f "${archive}" ]] ||
           ! printf '%s  %s\n' "${archive_sha256}" "${archive}" | LC_ALL=C sha256sum -c --status; then
            downloaded=false
            for archive_url in "${archive_urls[@]}"; do
                if curl -fsSL --retry 3 --connect-timeout 10 --max-time 180 \
                       "${archive_url}" -o "${archive}" &&
                   printf '%s  %s\n' "${archive_sha256}" "${archive}" |
                       LC_ALL=C sha256sum -c --status; then
                    downloaded=true
                    break
                fi
            done
            if [[ "${downloaded}" != true ]]; then
                printf 'error: could not download a matching Doxygen %s archive\n' "${version}" >&2
                exit 1
            fi
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
