# Z / libzlang — agent and contributor guide

## Start here

Z is a source-to-source language and a header-only C99 library. `zc` emits readable C99 that calls the
`zlang_*` runtime; a native C toolchain produces the executable. The CMake target is `zlang::zlang`.
There is no compiled library to install and no ABI to manage.

`README.md` is a short overview. The published documentation at <https://z.iantorres.cl> is the
source of truth for the language contract. `SPEC.md` and `ROADMAP.md` are working notes where
changes are accumulated before they are merged into the site specification. Read this file and the
relevant site page before changing behavior.

Basic workflow (Ninja, Clang):

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j13
```

One sanitizer build:

```sh
cmake -S . -B build-asan -G Ninja -DZLANG_SANITIZER=address
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

Coverage (GCC + gcovr only; never combined with `ZLANG_SANITIZER`):

```sh
cmake -S . -B build-coverage -G Ninja -DCMAKE_C_COMPILER=gcc -DZLANG_COVERAGE=ON
cmake --build build-coverage --target zlang_coverage
```

Reports land in `<build-dir>/coverage/`.

Compiler driver:

```sh
./build/tools/zc/zc example.z --emit-c
./build/tools/zc/zc example.z
```

`--emit-c` writes `<stem>.c`; default mode also invokes `$ZLANG_CC` (`cc`, `cl` on Windows).

Installed-package check:

```sh
cmake -S . -B build-package -G Ninja -DCMAKE_INSTALL_PREFIX="$PWD/install" -DZLANG_BUILD_TESTS=OFF
cmake --install build-package
cmake -S tests/consumer -B build-consumer -G Ninja -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build build-consumer && ./build-consumer/zlang_consumer
```

libFuzzer smoke target (Clang only):

```sh
cmake -S . -B build-fuzz -G Ninja -DCMAKE_C_COMPILER=clang -DZLANG_BUILD_TESTS=OFF -DZLANG_BUILD_FUZZERS=ON
cmake --build build-fuzz
for target in build-fuzz/fuzz/zlang_*_fuzz; do ./"$target" -max_total_time=30; done
```

Header isolation:

```sh
./scripts/check_headers.sh clang gcc
```

Doxygen API reference (`ZLANG_DOXYGEN=doxygen` uses a local install; otherwise the script downloads
the pinned, checksum-verified Doxygen 1.18.0 binary):

```sh
./scripts/build_docs.sh
# output: build/docs/html/index.html
```

Documentation site and browser playground:

```sh
cd site
nvm use 26
yarn install
yarn dev      # http://localhost:3000
yarn build    # static Nuxt build for Vercel
```

The browser toolchain is built from upstream LLVM, Emscripten and wasi-sdk:

```sh
./tools/wasm/build-toolchain.sh
./tools/wasm/build-zc-wasm.sh
```

Artifacts are served from `site/public/compiler/`. Third-party notices live in
`site/THIRD_PARTY_NOTICES.md`. Do not copy another project's driver source; build from upstream and
keep the notices current.

## Repository map

| Path | Purpose |
| --- | --- |
| `CMakeLists.txt` | Project definition, `zlang::zlang` interface target, compiler core, coverage target |
| `SPEC.md` | Working language notes; the published `/docs/specification` page is normative |
| `ROADMAP.md` | Compact implementation history and locked decisions |
| `cmake/` | Sanitizer, warning, coverage, testing and package-config modules |
| `include/zlang/zlang.h` | Umbrella header; includes every public module and defines nothing |
| `include/zlang/<module>/<module>.h` | Public API of one module |
| `include/zlang/runtime/runtime.h` | Runtime primitives used by emitted C: strings, print, traps, streams |
| `include/zlang/standard/` | Header-only standard library domains activated by generated macros |
| `include/zlang/detail/inline.h` | Private shared helper for `static inline` across compilers |
| `compiler/` | Compiler core: lexer, parser, AST, diagnostics, semantics, planner and C emitter |
| `tools/zc/` | Compiler driver with `z.yaml` module resolution |
| `tools/wasm/` | Reproducible builds for `zc.wasm`, `clang.wasm`, `lld.wasm` and the sysroot |
| `tests/<module>/` | One test module: CMake file, X-macro case list, test source |
| `tests/spec/` | Conformance fixtures and emitter tests derived from the site specification |
| `tests/consumer/` | Standalone project that verifies `find_package(zlang)` after install |
| `examples/` | Golden examples in `algorithms/` and `features/` with generated C and expected output |
| `fuzz/` | Clang libFuzzer harnesses for the public API, lexer, parser and emitter |
| `scripts/` | Header checks, Doxygen build and generated-file helpers |
| `Doxyfile.in` | Doxygen template used by `scripts/build_docs.sh` |
| `docs/` | Doxygen landing page and source-level reference material |
| `site/` | Nuxt documentation site, guides, specification pages and browser playground |
| `site/content/docs/` | Markdown source for `/docs/**` |
| `site/app/pages/playground.vue` | Browser playground page |
| `site/app/components/PlaygroundPanel.vue` | Shared playground editor/result panel |
| `site/app/lib/` | Browser toolchain runner and worker facade |
| `site/app/workers/` | Web Worker that runs compile and link off the UI thread |
| `site/public/compiler/` | Static wasm/sysroot artifacts served to the browser |
| `site/THIRD_PARTY_NOTICES.md` | Upstream notices for bundled toolchain artifacts |
| `.github/workflows/` | CI, CodeQL and review-gate workflows |
| `.pre-commit-config.yaml` | Local clang-format and cppcheck hooks |
| `codecov.yml` | Coverage gate for `include/` |
| `.clang-format`, `.clang-tidy` | Mechanically enforced style and naming |

## Project facts

- The project is **Z**; the library target is **libzlang** (`zlang::zlang`). Authoritative versions
  live in `cmake/ZlangVersion.cmake`.
- Public headers are grouped by module and reached through `include/zlang/zlang.h`.
- The version API exposes `LIBZLANG_VERSION_MAJOR`, `LIBZLANG_VERSION_MINOR`,
  `LIBZLANG_VERSION_PATCH` and `zlang_version_t zlang_get_version(void)`.
- The library is header-only. Keep it that way unless the user explicitly asks otherwise.
- The project remains **unlicensed**. Do not add SPDX headers, license files or license banners.
- The documentation site is published at <https://z.iantorres.cl>; the generated API reference is at
  <https://z-doxygen.iantorres.cl>.

## Key decisions

- **Header-only C99 baseline**: widest compiler and platform reach. CI covers C99/C11/C17/C23 with
  GCC and Clang, plus MSVC, clang-cl and MinGW.
- **No mutable global variables**: state travels in context structs passed by pointer. This keeps
  tests independent and process parallelism safe.
- **One CTest test per test case**: `ctest -jX` means exactly X cases. Linker-section registration
  was rejected because it needs platform-specific scripts and reintroduces globals.
- **Compile-time memory plan**: automatic storage at or below `ZLANG_DEFAULT_STACK_LIMIT` (4096
  bytes), `static` storage above it. The emitter consumes the plan; it never guesses storage.
- **Site spec over working notes**: `SPEC.md` and `ROADMAP.md` accumulate decisions. The normative
  contract is the published `/docs/specification` page. When they disagree, update the site first.
- **Browser toolchain is client-side**: `zc.wasm`, `clang.wasm`, `lld.wasm` and the sysroot are static
  assets; compilation runs in a Web Worker. No source or binary is sent to a server.
- **Upstream artifacts, not copied drivers**: build clang/lld/sysroot from pinned upstream sources
  and preserve `site/THIRD_PARTY_NOTICES.md`.
- **MSAN excluded**: it needs an instrumented standard library. Do not add combined sanitizer modes.
- **Layered static analysis**: clang-tidy, cppcheck, GCC `-fanalyzer`, `clang --analyze` and CodeQL
  all run in CI. A new defect class should get a mechanical check, not only a test.
- **Pinned actions**: every `uses:` names a full commit SHA followed by a version comment.
  Dependabot proposes weekly bumps; never replace a SHA with a floating tag.
- **Per-ref workflow cancellation**: workflows use
  `concurrency.group: ${{ github.workflow }}-${{ github.ref }}` with `cancel-in-progress: true`.
- **All-green gate**: `ci-green` and CodeQL are required checks in the `master` ruleset. Repository
  files cannot create that ruleset without an admin token.
- **macOS runners**: use currently documented labels only; `macos-14` is retired. Keep explicit
  `timeout-minutes` on arm64 jobs.
- **Vercel**: the documentation site is a standard Nuxt project with Root Directory `site`. There is
  no `vercel.json`; the API reference is generated separately by `scripts/build_docs.sh`.

## Language and naming

- Everything in the repository is written in **English**: identifiers, comments, commit messages,
  test names and documentation. Never use Spanish identifiers such as `_contexto`.
- Types end in `_t` and use `lower_case`: `zlang_version_t`, `zlang_test_case_t`.
- Struct and enum members end in `_`: `major_`, `failure_count_`.
- Local variables start with `_`: `_index`, `_case_context`.
- Constants end in `_k`; local constants also carry the `_` prefix (`_expected_k`), file-scope
  constants do not (`cases_k`).
- Macros are `UPPER_SNAKE_CASE`: `LIBZLANG_VERSION_MAJOR`, `ZLANG_CHECK_UINT`.
- Functions, parameters, namespaces and aliases are `lower_case` with no prefix or suffix.
- Parameters do **not** carry the local `_` prefix: `zlang_test_context_t *const context` is correct.
- Names must be expressive: `memory_copy`, `case_count`, `index`; never `memcpy`, `n`, `i`, `j`, `k`.
- `.clang-tidy` mirrors these rules. One documented exception disables
  `readability-avoid-const-params-in-decls`. A new exception needs user approval.

## Structure and ownership

- **No mutable global variables.** State travels through context structs passed by pointer.
- Prefer `const` on parameters that are not mutated. Double `const` on a non-rebound pointer is
  expected: `zlang_test_context_t *const context`.
- Use `const` for locals that are never reassigned; name them with `_k`.
- There is exactly one way to do a given thing. Replace old patterns instead of adding a second one.
- Header guards are `ZLANG_<PATH>_H` with the full path after `include/`; `#endif` carries the match.
  Module headers live at `include/zlang/<module>/<module>.h`; private helpers live in
  `include/zlang/detail/`. The umbrella header only includes modules and defines nothing.

## Tests and parallelism

- The testing framework is the header-only `include/zlang/testing/testing.h`. Do not add a second
  support layer or a separate `zlang::testing` target without explicit approval.
- One test module = one directory = one executable. Cases are declared once as
  `ZLANG_TEST_CASE(name)` in the module's `test_cases.h`; CMake parses that file via X-macro and
  `file(STRINGS ...)`.
- One CTest test per case, named `zlang.<module>.<case>`, labelled `module:<module>`. Filter with
  `ctest -L module:<module>`. Never register one test that runs the whole suite.
- Cases must not share mutable state. Sanitizer runtime options are attached to every test of the
  module through `ZLANG_SANITIZER_ENV`.

To add a test module:

1. Create `tests/<module>/CMakeLists.txt` containing only `zlang_add_test_module(<module>)`.
2. Create `tests/<module>/test_cases.h` with the `ZLANG_TEST_CASE(name)` lines.
3. Create `tests/<module>/<module>_tests.c` with the functions, case table and `main`.
4. Add one `add_subdirectory(<module>)` line to `tests/CMakeLists.txt`.

To add a public module: create `include/zlang/<module>/<module>.h`, include it from
`include/zlang/zlang.h`, and add tests in the matching `tests/<module>/` directory.

## Sanitizers

- Supported: `address`, `undefined`, `thread`, `leak`, `integer`, `cfi`, `hwaddress`.
- Zero-warning policy: `-Werror` on GCC/Clang, `/WX` on MSVC.
- Platform exceptions live in `cmake/ZlangSanitizers.cmake`: Apple ASAN rejects `detect_leaks=1`,
  MSVC ASAN needs `/Zi` plus `/DEBUG`, and link-only flags belong in `ZLANG_SANITIZER_LINK_FLAGS`.
- Out of scope: MSan, DFSan, ShadowCallStack, TypeSanitizer, RealtimeSanitizer,
  ConcurrencySanitizer and CSan for the reasons recorded in the old guide. LSan is covered by the
  standalone `leak` mode and ASAN where supported.
- Targets include amd64/arm64 Linux (glibc and musl), macOS, Windows (MSVC, clang-cl, MinGW), QEMU
  architectures and 32-bit x86.

## Before changing code

1. Check this file, the site specification and the existing implementation for an established
   pattern; follow it.
2. Ask the user before inventing public API, changing the stated contract or adding a dependency.
3. Run `cmake --build`, `ctest --output-on-failure`, and for style-sensitive changes run
   `clang-format --dry-run --Werror`, `cppcheck` and a clang-tidy build.
4. If public headers changed, run `./scripts/check_headers.sh`; if install rules changed, run the
   package-consumer commands; if the public API changed, run the libFuzzer smoke target.
5. If site content or the wasm toolchain changed, run `cd site && yarn build` and refresh
   `site/public/compiler/` with `tools/wasm/` scripts when runtime headers or artifacts change.
