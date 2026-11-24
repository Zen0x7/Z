# Z / libzlang — agent and contributor guide

## Start here

Z is a header-only C99 library. The CMake target is `zlang::zlang`; there is no compiled library to
install and no ABI to manage. This file is the entry point for contributors and agents; read it
before changing anything. `README.md` is a short project overview with status badges.

Basic workflow (Ninja, Clang):

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j13
```

Run one sanitizer build:

```sh
cmake -S . -B build-asan -G Ninja -DZLANG_SANITIZER=address
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

Generate coverage (GCC + gcovr only; cannot be combined with `ZLANG_SANITIZER`):

```sh
cmake -S . -B build-coverage -G Ninja -DCMAKE_C_COMPILER=gcc -DZLANG_COVERAGE=ON
cmake --build build-coverage --target zlang_coverage
```

The coverage target writes text, Cobertura XML, and HTML reports under `<build-dir>/coverage/`.

Use the compiler driver. `--emit-c` writes `<stem>.c` next to the source; without it,
the driver also invokes `$ZLANG_CC` (or `cc`, `cl` on Windows) to produce the executable:

```sh
./build/tools/zlang-compile/zlang-compile example.zlang --emit-c
./build/tools/zlang-compile/zlang-compile example.zlang
```

Check that the installed package works for an external consumer:

```sh
cmake -S . -B build-package -G Ninja -DCMAKE_INSTALL_PREFIX="$PWD/install" -DZLANG_BUILD_TESTS=OFF
cmake --install build-package
cmake -S tests/consumer -B build-consumer -G Ninja -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build build-consumer && ./build-consumer/zlang_consumer
```

Build and run the libFuzzer smoke target (Clang only):

```sh
cmake -S . -B build-fuzz -G Ninja -DCMAKE_C_COMPILER=clang -DZLANG_BUILD_TESTS=OFF -DZLANG_BUILD_FUZZERS=ON
cmake --build build-fuzz
for target in build-fuzz/fuzz/zlang_*_fuzz; do ./"$target" -max_total_time=30; done
```

Compile every public header on its own:

```sh
./scripts/check_headers.sh clang gcc
```

Build the API reference (set `ZLANG_DOXYGEN=doxygen` to use a local install; otherwise the script
downloads the pinned, checksum-verified Doxygen 1.18.0 Linux binary):

```sh
./scripts/build_docs.sh
# output: build/docs/html/index.html
```

## Repository map

| Path | Purpose |
| --- | --- |
| `CMakeLists.txt` | Project definition, `zlang::zlang` interface target, compiler core, coverage target |
| `SPEC.md` | Normative language contract emitted C must satisfy |
| `ROADMAP.md` | Implementation phases and deferred design questions |
| `cmake/ZlangSanitizers.cmake` | `ZLANG_SANITIZER` options, compiler/platform gates, CTest runtime env |
| `cmake/ZlangWarnings.cmake` | One strict warning set (`-Werror` / `/WX`) shared by all builds |
| `cmake/ZlangCoverage.cmake` | GCC + gcovr instrumentation gate |
| `cmake/ZlangTesting.cmake` | `zlang_add_test_module()`: builds one test executable and registers its cases |
| `cmake/zlangConfig.cmake.in` | Template for the installed CMake package config |
| `include/zlang/zlang.h` | Umbrella header; includes every public module and defines nothing |
| `include/zlang/<module>/<module>.h` | Public API of one module |
| `include/zlang/runtime/runtime.h` | Public runtime primitives used by emitted C: bounded strings, print, traps |
| `include/zlang/detail/inline.h` | Private shared helper for `static inline` across compilers |
| `compiler/` | Compiler core: lexer, parser, inert AST, diagnostics, semantic analyzer, memory planner, and C emitter |
| `tools/zlang-compile/` | Compiler driver: `--emit-c` writes `<stem>.c`; default mode also links a binary |
| `tests/<module>/` | One test module: CMake file, X-macro case list, test source |
| `tests/spec/` | Conformance fixtures and emitter tests derived from `SPEC.md` |
| `tests/consumer/` | Standalone project that verifies `find_package(zlang)` after install |
| `fuzz/` | Clang libFuzzer harnesses for the public API, lexer, parser and emitter |
| `scripts/check_headers.sh` | Compiles each public header in isolation with GCC and Clang |
| `Doxyfile` | Doxygen configuration; writes HTML into `build/docs/html` |
| `docs/index.md` | API reference landing page |
| `scripts/build_docs.sh` | Reproducible Doxygen build used by CI and Vercel |
| `vercel.json` | Vercel static build command and output directory for the API reference |
| `.github/workflows/ci.yml` | Build/test/coverage/sanitizer matrix plus review-gate jobs |
| `.github/workflows/codeql.yml` | C/C++ CodeQL analysis with security events upload |
| `.github/CODEOWNERS`, `.github/dependabot.yml` | Review ownership and weekly action updates |
| `.pre-commit-config.yaml` | Local clang-format and cppcheck hooks |
| `codecov.yml` | Coverage gate: 100% project and patch target for `include/`; Codecov ignores tests, fuzz, cmake, compiler, and tools |
| `.clang-format`, `.clang-tidy` | Mechanically enforced style and naming |

## Project facts

- The project is **Z**; the library target is **libzlang** (`zlang::zlang`), version **0.0.0**.
- Public headers are grouped by module: `include/zlang/version/version.h`,
  `include/zlang/testing/testing.h`, and `include/zlang/runtime/runtime.h`.
  `include/zlang/zlang.h` is the umbrella header. The version API is
  `LIBZLANG_VERSION_MAJOR`, `LIBZLANG_VERSION_MINOR`, `LIBZLANG_VERSION_PATCH` and
  `zlang_version_t zlang_get_version(void)`.
- The library is header-only. Keep it that way unless the user explicitly asks otherwise.
- The project is currently **unlicensed**. Do not add SPDX headers, license files, or license
  banners to any file.

## Key decisions and why they exist

- **Header-only, C99 baseline** (`target_compile_features(zlang INTERFACE c_std_99)`): widest
  compiler and platform reach. CI also compiles C99, C11, C17, and C23 with GCC and Clang, plus
  MSVC, clang-cl, and MinGW.
- **No mutable global variables**: state travels in context structs passed by pointer. This keeps
  tests independent and makes process-level parallelism safe.
- **One CTest test per test case**: `ctest -jX` then means exactly X cases running concurrently.
  Linker-section auto-registration was rejected because it needs platform-specific linker scripts,
  has fragile initialization order, and reintroduces globals.
- **Compile-time memory plan**: declarations get automatic storage at or below
  `ZLANG_DEFAULT_STACK_LIMIT` (4096 bytes) and `static` storage above it. The emitter consumes the
  plan; it never allocates memory or guesses storage at runtime.
- **MSAN is intentionally excluded**: it requires rebuilding the C/C++ standard library with
  instrumentation, which violates the "no toolchain rebuild" requirement. Do not add combined
  sanitizer modes such as `address+undefined`; each sanitizer already has its own matrix entry.
- **README and license**: `README.md` is a short project overview with status badges and is not a
  substitute for this guide. The project remains unlicensed; do not add a license file.
- **Pinned actions**: every `uses:` in a workflow names a full commit SHA followed by a version
  comment. Dependabot proposes weekly bumps; never replace a SHA with a floating tag. All
  `github/codeql-action/*` steps are grouped in `.github/dependabot.yml` because `init`, `analyze`
  and other CodeQL steps must run at the same version.
- **Per-ref workflow cancellation**: every workflow uses
  `concurrency.group: ${{ github.workflow }}-${{ github.ref }}` with `cancel-in-progress: true`.
  `github.ref` differs for `master`, each branch and each pull request, so a branch never blocks
  main; only superseded runs on the same ref are cancelled. If a run on one ref is ever stuck in
  `queued`, cancel that run in the Actions UI; other refs are unaffected.
- **All-green gate**: the `ci-green` job must be listed as a required status check in the GitHub
  ruleset for `master`. CodeQL is a separate workflow, so its own check must also be required.
  Repository files cannot create that ruleset without an admin token.
- **Layered static analysis**: clang-tidy, cppcheck, GCC `-fanalyzer`, `clang --analyze`, and
  CodeQL all run in CI. A new defect class should get a mechanical check, not only a test.
- **Installed-package and header checks**: `tests/consumer/` catches broken exports; scripts/
  check_headers.sh catches hidden header dependencies. Both are cheap required jobs.
- **macOS runners**: use only currently documented labels. `macos-14` is retired, which produced
  `The configuration 'macos-14-arm64-us-default' is not supported`. The matrix uses `macos-15`
  (arm64/M1), `macos-15-intel` and `macos-26-intel` (both Intel). All jobs keep explicit
  `timeout-minutes` because arm64 macOS can still hit capacity queues.
- **API docs are Vercel-ready**: `vercel.json` selects "Other" as the framework, skips the install
  step, runs `scripts/build_docs.sh`, and serves `build/docs/html`. The script pins Doxygen 1.18.0
  with a SHA256 check, so Vercel does not need `apt-get` or a preinstalled Doxygen. It tries the
  official doxygen.nl mirror first, then the GitHub release, then SourceForge; every source must
  match the same SHA256. This keeps Vercel builds working when GitHub release assets return 404 from
  the build region. There is no
  extra GitHub Actions step: Vercel's Git integration builds every push and PR directly. Vercel must
  still be connected to the GitHub repo in the Vercel dashboard; choose the repository root and keep
  preview deployments enabled so each PR gets its own URL. No Vercel secrets or deploy hooks are
  required. The published production reference is <https://z.iantorres.cl>.

## Language and naming

- Everything in the repository is written in **English**: identifiers, comments, commit messages,
  test names, and documentation. Never use Spanish identifiers such as `_contexto`.
- Types (structs, unions, enums, typedefs, function-pointer typedefs) end in `_t` and use
  `lower_case`: `zlang_version_t`, `zlang_test_case_t`, `zlang_test_fn_t`.
- Struct and enum members end in `_`: `major_`, `minor_`, `patch_`, `name_`, `run_`,
  `failure_count_`.
- Local variables start with `_`: `_index`, `_case_context`.
- Constants (local `const`, `static const`, global `const`, enum constants) end in `_k`. Local
  constants also carry the `_` prefix (`_expected_k`); file-scope constants do not, because a
  leading underscore is reserved at file scope in C (`cases_k`).
- Macros are `UPPER_SNAKE_CASE`: `LIBZLANG_VERSION_MAJOR`, `ZLANG_CHECK_UINT`.
- Functions, parameters, namespaces, and aliases are `lower_case` with no prefix or suffix:
  `zlang_get_version`, `zlang_test_run_cases`, `context`, `case_count`.
- Parameters do **not** carry the local `_` prefix. `zlang_test_context_t *const context` is correct;
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
  pointer: `zlang_test_context_t *const context`. The old `zlang_test_failures` global is gone and must
  not come back.
- Prefer `const` on every parameter that is not mutated. Pass structs by pointer (`const` when read
  only); pass scalars by value only where a pointer would add noise. Double `const`
  (`zlang_test_context_t *const context`) is expected when the pointer itself is not rebound.
- Use `const` for local values that are never reassigned; name them with the `_k` suffix.
- There is exactly **one way** to do a given thing in this codebase. Do not introduce a second
  naming scheme, a second test registration mechanism, a second header layout, or a second warning
  flag set. If a new pattern is needed, replace the old one.
- Header guards are `ZLANG_<PATH>_H` with the full path after `include/`: `ZLANG_VERSION_VERSION_H`,
  `ZLANG_TESTING_TESTING_H`, `ZLANG_ZLANG_H`. `#endif` carries the matching comment. A module header lives
  at `include/zlang/<module>/<module>.h`; shared private helpers live in `include/zlang/detail/`. The
  umbrella `include/zlang/zlang.h` only includes module headers and defines nothing.

## Tests and parallelism

- The testing framework is the public module header `include/zlang/testing/testing.h`
  (`zlang_test_context_t`, `zlang_test_case_t`, `ZLANG_CHECK`, `ZLANG_CHECK_UINT`, `zlang_test_run_cases`,
  `zlang_test_main`). It is header-only, uses `ZLANG_STATIC_INLINE` from `include/zlang/detail/inline.h`,
  and is reached through the existing `zlang::zlang` include path. Do not add a second test-support
  layer in `tests/`, and do not create a separate `zlang::testing` target without explicit approval.
- **One test module = one directory = one executable.** Each module owns
  `tests/<module>/CMakeLists.txt`, `tests/<module>/test_cases.h`, and
  `tests/<module>/<module>_tests.c`. No file outside the module is edited to add cases to it.
- Test cases are declared exactly once, as `ZLANG_TEST_CASE(name)` lines in the module's own
  `tests/<module>/test_cases.h`. C uses that file via X-macro expansion for prototypes and the
  case table; `zlang_add_test_module()` in `cmake/ZlangTesting.cmake` parses it with `file(STRINGS ...)`
  to register tests. Case names are unique within the module only.
- **One CTest test per test case**, named `zlang.<module>.<case>` and labelled `module:<module>`.
  Never register a single CTest test that executes the whole suite, and never add in-process
  threads to compensate for that. Filter a module with `ctest -L module:<module>`.
- Test cases must not share mutable state. Each `zlang_test_run_cases` invocation creates a local
  `zlang_test_context_t` for the selected case; output is written by a single process.
- Sanitizer runtime options are attached by `zlang_add_test_module()` to every test of the module
  through `ZLANG_SANITIZER_ENV`, not to one hard-coded test name.

To add a test module:

1. Create `tests/<module>/CMakeLists.txt` containing only `zlang_add_test_module(<module>)`.
2. Create `tests/<module>/test_cases.h` with the `ZLANG_TEST_CASE(name)` lines.
3. Create `tests/<module>/<module>_tests.c` with the functions, case table, and `main`.
4. Add one `add_subdirectory(<module>)` line to `tests/CMakeLists.txt`.

To add a public module: create `include/zlang/<module>/<module>.h` with the matching guard, include
it from `include/zlang/zlang.h`, and add tests in the corresponding `tests/<module>/` directory.

## Sanitizers and CI

- Supported sanitizers: `address`, `undefined`, `thread`, `leak`, `integer`, `cfi`, `hwaddress`.
  `hwaddress` is an experimental clang/aarch64 job.
- Zero-warning policy: `-Werror` on GCC/Clang, `/WX` on MSVC. Coverage uses GCC + gcovr and
  publishes Cobertura XML for Codecov.
- Platform exceptions are handled in `cmake/ZlangSanitizers.cmake`: Apple's ASAN runtime rejects
  `detect_leaks=1`, MSVC ASAN needs `/Zi` plus `/DEBUG` to avoid C5072 under `/WX`, and link-only
  flags such as `-fuse-ld=lld` belong in `ZLANG_SANITIZER_LINK_FLAGS` (never in compile flags).
- Not part of the matrix: MSan, DFSan and ShadowCallStack require an instrumented libc. SCS also
  needs a dynamic loader built with x18 reserved; system glibc clobbers that register, and shimming
  libc calls one by one would not scale. DFSan additionally needs explicit tagging APIs and is
  x86-64/Linux-only. TypeSanitizer, RealtimeSanitizer and ConcurrencySanitizer need a custom
  Clang/compiler-rt build, and CSan is x86-64/Linux-only. LeakSanitizer is already
  covered by the standalone `leak` mode and by ASAN on supported hosts.
- Target platforms in the matrix: amd64 and arm64 on Linux (glibc and musl), macOS, Windows
  (MSVC, clang-cl, MinGW), plus QEMU architectures and 32-bit x86. Windows uses every documented
  hosted label: `windows-2022`, `windows-2025` and `windows-2025-vs2026` for x64, and
  `windows-11-arm` plus `windows-11-vs2026-arm` for arm64; MinGW runs on both 2022 and 2025.

## Before changing code

1. Check this file and the existing implementation for an established pattern; follow it.
2. Ask the user before inventing public API, changing the stated contract, or adding a dependency.
3. Run `cmake --build`, `ctest --output-on-failure`, and, when touching style-sensitive code, the
   `analysis` job commands: `clang-format --dry-run --Werror`, `cppcheck`, and a clang-tidy build.
4. If public headers changed, run `./scripts/check_headers.sh`; if install rules changed, run the
   package-consumer commands; if the public API changed, run the libFuzzer smoke target.
