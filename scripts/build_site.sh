#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

printf 'Building Doxygen reference...\n'
"${root}/scripts/build_docs.sh"

printf 'Copying Doxygen output into the Nuxt public directory...\n'
rm -rf "${root}/site/public/doxygen"
mkdir -p "${root}/site/public/doxygen"
cp -R "${root}/build/docs/html/." "${root}/site/public/doxygen/"

printf 'Generating Nuxt site...\n'
cd "${root}/site"
yarn generate

printf 'Site output written to site/.output/public\n'
