#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
generated_include=${1:-"${root}/build/generated/include"}

read_setting() {
    local name=$1
    sed -nE "s/^set\\(${name}[[:space:]]+([^)]*)\\).*/\\1/p" \
        "${root}/cmake/ZlangVersion.cmake" | head -n 1 | tr -d '"'
}

project_version=$(read_setting ZLANG_PROJECT_VERSION)
language_version=$(read_setting ZLANG_LANGUAGE_VERSION)
if [[ -z "${project_version}" || -z "${language_version}" ]]; then
    printf 'error: could not read versions from cmake/ZlangVersion.cmake\n' >&2
    exit 1
fi

IFS='.' read -r version_major version_minor version_patch <<< "${project_version}"
if [[ -z "${version_major}" || -z "${version_minor}" || -z "${version_patch}" ]]; then
    printf 'error: invalid project version: %s\n' "${project_version}" >&2
    exit 1
fi

mkdir -p "${generated_include}/zlang/version" "${root}/build/docs"

sed -e "s|@ZLANG_VERSION_MAJOR@|${version_major}|g" \
    -e "s|@ZLANG_VERSION_MINOR@|${version_minor}|g" \
    -e "s|@ZLANG_VERSION_PATCH@|${version_patch}|g" \
    "${root}/include/zlang/version/version.h.in" \
    > "${generated_include}/zlang/version/version.h"

sed -e "s|@ZLANG_VERSION_MAJOR@|${version_major}|g" \
    -e "s|@ZLANG_VERSION_MINOR@|${version_minor}|g" \
    -e "s|@ZLANG_VERSION_PATCH@|${version_patch}|g" \
    "${root}/include/zlang/version/version_config.h.in" \
    > "${generated_include}/zlang/version/version_config.h"

sed -e "s|@ZLANG_PROJECT_VERSION@|${project_version}|g" \
    -e "s|@ZLANG_LANGUAGE_VERSION@|${language_version}|g" \
    -e "s|@ZLANG_GENERATED_INCLUDE_DIR@|${generated_include}|g" \
    "${root}/Doxyfile.in" > "${root}/build/Doxyfile"
