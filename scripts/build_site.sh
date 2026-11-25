#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

printf 'Building Doxygen reference...\n'
"${root}/scripts/build_docs.sh"

printf 'Clearing Nuxt caches...\n'
rm -rf "${root}/site/.nuxt" \
       "${root}/site/.data" \
       "${root}/site/.output" \
       "${root}/site/node_modules/.cache"

printf 'Generating Nuxt site...\n'
cd "${root}/site"
yarn generate

printf 'Copying Doxygen output into the generated site...\n'
rm -rf "${root}/site/.output/public/doxygen"
mkdir -p "${root}/site/.output/public/doxygen"
cp -R "${root}/build/docs/html/." "${root}/site/.output/public/doxygen/"

printf 'Site output written to site/.output/public\n'
