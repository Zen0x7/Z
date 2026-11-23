# Z / libzen — agent and contributor guide

## Start here

Z is a header-only C99 library. The CMake target is `zen::zen`; there is no compiled library to
install and no ABI to manage. There is **no README** by project decision, so this file is the entry
point for contributors and agents. Read it before changing anything.

Basic workflow (Ninja, Clang):

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j7
```

Run one sanitizer build:

```sh
cmake -S . -B build-asan -G Ninja -DZEN_SANITIZER=address
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

Generate coverage (GCC + gcovr only; cannot be combined with `ZEN_SANITIZER`):

```sh
cmake -S . -B build-coverage -G Ninja -DCMAKE_C_COMPILER=gcc -DZEN_COVERAGE=ON
cmake --build build-coverage --target zen_coverage
```

The coverage target writes text, Cobertura XML, and HTML reports under `<build-dir>/coverage/`.

## Repository map

| Path | Purpose |
| --- | --- |
| `CMakeLists.txt` | Project definition, `zen::zen` interface target, coverage target |
| `cmake/ZenSanitizers.cmake` | `ZEN_SANITIZER` options, compiler/platform gates, CTest runtime env |
| `cmake/ZenWarnings.cmake` | One strict warning set (`-Werror` / `/WX`) shared by all builds |
| `cmake/ZenCoverage.cmake` | GCC + gcovr instrumentation gate |
| `cmake/ZenTesting.cmake` | `zen_add_test_module()`: builds one test executable and registers its cases |
| `include/zen/zen.h` | Umbrella header; includes every public module and defines nothing |
| `include/zen/<module>/<module>.h` | Public API of one module |
| `include/zen/detail/inline.h` | Private shared helper for `static inline` across compilers |
| `tests/<module>/` | One test module: CMake file, X-macro case list, test source |
| `.github/workflows/ci.yml` | Build/test/coverage/sanitizer matrix |
| `.clang-format`, `.clang-tidy` | Mechanically enforced style and naming |

## Project facts

- The project is **Z**; the library target is **libzen** (`zen::zen`), version **0.0.0**.
- Public headers are grouped by module: `include/zen/version/version.h` and
  `include/zen/testing/testing.h`. `include/zen/zen.h` is the umbrella header. The version API is
  `LIBZEN_VERSION_MAJOR`, `LIBZEN_VERSION_MINOR`, `LIBZEN_VERSION_PATCH` and
  `zen_version_t zen_get_version(void)`.
- The library is header-only. Keep it that way unless the user explicitly asks otherwise.
- The project is currently **unlicensed**. Do not add SPDX headers, license files, or license
  banners to any file.

## Key decisions and why they exist

- **Header-only, C99 baseline** (`target_compile_features(zen INTERFACE c_std_99)`): widest
  compiler and platform reach. CI also compiles C99, C11, C17, and C23 with GCC and Clang, plus
  MSVC, clang-cl, and MinGW.
- **No mutable global variables**: state travels in context structs passed by pointer. This keeps
  tests independent and makes process-level parallelism safe.
- **One CTest test per test case**: `ctest -jX` then means exactly X cases running concurrently.
  Linker-section auto-registration was rejected because it needs platform-specific linker scripts,
  has fragile initialization order, and reintroduces globals.
- **MSAN is intentionally excluded**: it requires rebuilding the C/C++ standard library with
  instrumentation, which violates the "no toolchain rebuild" requirement. Do not add combined
  sanitizer modes such as `address+undefined`; each sanitizer already has its own matrix entry.
- **No README, no license**: explicit project decisions. Do not reintroduce either.

## Language and naming

- Everything in the repository is written in **English**: identifiers, comments, commit messages,
  test names, and documentation. Never use Spanish identifiers such as `_contexto`.
- Types (structs, unions, enums, typedefs, function-pointer typedefs) end in `_t` and use
  `lower_case`: `zen_version_t`, `zen_test_case_t`, `zen_test_fn_t`.
- Struct and enum members end in `_`: `major_`, `minor_`, `patch_`, `name_`, `run_`,
  `failure_count_`.
- Local variables start with `_`: `_index`, `_case_context`.
- Constants (local `const`, `static const`, global `const`, enum constants) end in `_k`. Local
  constants also carry the `_` prefix (`_expected_k`); file-scope constants do not, because a
  leading underscore is reserved at file scope in C (`cases_k`).
- Macros are `UPPER_SNAKE_CASE`: `LIBZEN_VERSION_MAJOR`, `ZEN_CHECK_UINT`.
- Functions, parameters, namespaces, and aliases are `lower_case` with no prefix or suffix:
  `zen_get_version`, `zen_test_run_cases`, `context`, `case_count`.
- Parameters do **not** carry the local `_` prefix. `zen_test_context_t *const context` is correct;
  `_context` as a parameter name is not.
- Names must be expressive. Spell words out: `memory_copy`, not `memcpy`; `case_count`, not `n`;
  `index` or `item_index`, never `i`, `j`, or `k` as loop counters.
- These rules are mirrored mechanically in `.clang-tidy` via `readability-identifier-naming`, and
  formatting is enforced by `.clang-format`. One tooling exception is documented in `.clang-tidy`:
  `readability-avoid-const-params-in-decls` is disabled because declarations intentionally repeat
  the top-level `const` used in definitions. A change that needs another exception must be discussed
  with the user first.

## Structure and ownership

- **No mutable global variables.** State travels explicitly through context structs passed by
  pointer: `zen_test_context_t *const context`. The old `zen_test_failures` global is gone and must
  not come back.
- Prefer `const` on every parameter that is not mutated. Pass structs by pointer (`const` when read
  only); pass scalars by value only where a pointer would add noise. Double `const`
  (`zen_test_context_t *const context`) is expected when the pointer itself is not rebound.
- Use `const` for local values that are never reassigned; name them with the `_k` suffix.
- There is exactly **one way** to do a given thing in this codebase. Do not introduce a second
  naming scheme, a second test registration mechanism, a second header layout, or a second warning
  flag set. If a new pattern is needed, replace the old one.
- Header guards are `ZEN_<PATH>_H` with the full path after `include/`: `ZEN_VERSION_VERSION_H`,
  `ZEN_TESTING_TESTING_H`, `ZEN_ZEN_H`. `#endif` carries the matching comment. A module header lives
  at `include/zen/<module>/<module>.h`; shared private helpers live in `include/zen/detail/`. The
  umbrella `include/zen/zen.h` only includes module headers and defines nothing.

## Tests and parallelism

- The testing framework is the public module header `include/zen/testing/testing.h`
  (`zen_test_context_t`, `zen_test_case_t`, `ZEN_CHECK`, `ZEN_CHECK_UINT`, `zen_test_run_cases`,
  `zen_test_main`). It is header-only, uses `ZEN_STATIC_INLINE` from `include/zen/detail/inline.h`,
  and is reached through the existing `zen::zen` include path. Do not add a second test-support
  layer in `tests/`, and do not create a separate `zen::testing` target without explicit approval.
- **One test module = one directory = one executable.** Each module owns
  `tests/<module>/CMakeLists.txt`, `tests/<module>/test_cases.h`, and
  `tests/<module>/<module>_tests.c`. No file outside the module is edited to add cases to it.
- Test cases are declared exactly once, as `ZEN_TEST_CASE(name)` lines in the module's own
  `tests/<module>/test_cases.h`. C uses that file via X-macro expansion for prototypes and the
  case table; `zen_add_test_module()` in `cmake/ZenTesting.cmake` parses it with `file(STRINGS ...)`
  to register tests. Case names are unique within the module only.
- **One CTest test per test case**, named `zen.<module>.<case>` and labelled `module:<module>`.
  Never register a single CTest test that executes the whole suite, and never add in-process
  threads to compensate for that. Filter a module with `ctest -L module:<module>`.
- Test cases must not share mutable state. Each `zen_test_run_cases` invocation creates a local
  `zen_test_context_t` for the selected case; output is written by a single process.
- Sanitizer runtime options are attached by `zen_add_test_module()` to every test of the module
  through `ZEN_SANITIZER_ENV`, not to one hard-coded test name.

To add a test module:

1. Create `tests/<module>/CMakeLists.txt` containing only `zen_add_test_module(<module>)`.
2. Create `tests/<module>/test_cases.h` with the `ZEN_TEST_CASE(name)` lines.
3. Create `tests/<module>/<module>_tests.c` with the functions, case table, and `main`.
4. Add one `add_subdirectory(<module>)` line to `tests/CMakeLists.txt`.

To add a public module: create `include/zen/<module>/<module>.h` with the matching guard, include
it from `include/zen/zen.h`, and add tests in the corresponding `tests/<module>/` directory.

## Sanitizers and CI

- Supported sanitizers: `address`, `undefined`, `thread`, `leak`, `integer`, `cfi`, `hwaddress`,
  `shadow-call-stack` (the last two are clang/aarch64 experiments in CI).
- Zero-warning policy: `-Werror` on GCC/Clang, `/WX` on MSVC. Coverage uses GCC + gcovr and
  publishes Cobertura XML for Codecov.
- Platform exceptions are handled in `cmake/ZenSanitizers.cmake`: Apple's ASAN runtime rejects
  `detect_leaks=1`, MSVC ASAN needs `/Zi` plus `/DEBUG` to avoid C5072 under `/WX`, and link-only
  flags such as `-fuse-ld=lld` belong in `ZEN_SANITIZER_LINK_FLAGS` (never in compile flags).
- Target platforms in the matrix: amd64 and arm64 on Linux (glibc and musl), macOS, Windows
  (MSVC, clang-cl, MinGW), plus QEMU architectures and 32-bit x86.

## Before changing code

1. Check this file and the existing implementation for an established pattern; follow it.
2. Ask the user before inventing public API, changing the stated contract, or adding a dependency.
3. Run `cmake --build`, `ctest --output-on-failure`, and, when touching style-sensitive code, the
   `analysis` job commands: `clang-format --dry-run --Werror`, `cppcheck`, and a clang-tidy build.
