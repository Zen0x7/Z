# Single source of truth for the package version and the documented language level.
set(ZLANG_PROJECT_VERSION 0.14.0)
set(ZLANG_LANGUAGE_VERSION 0.14)

string(REPLACE "." ";" _zlang_version_parts "${ZLANG_PROJECT_VERSION}")
list(GET _zlang_version_parts 0 ZLANG_VERSION_MAJOR)
list(GET _zlang_version_parts 1 ZLANG_VERSION_MINOR)
list(GET _zlang_version_parts 2 ZLANG_VERSION_PATCH)
