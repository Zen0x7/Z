# Standard library design

**Status:** implemented in Zlang 0.25. The catalog below is the allowed namespace map; the
implemented standard functions are the deterministic `random` generators and the
`@std/program_options` domain.
User modules are resolved through the same import syntax with relative paths and `z.yaml` aliases.

The following domains are **planned only** and are not registered or importable yet:
`@std/algorithms`, `@std/encoding`, `@std/memory`, `@std/streams`, `@std/filesystem`,
`@std/system` and `@std/networking`. They are listed as design targets because the domain-based
import grammar is intended to grow without adding aliases such as `net` or `fs`.

## Import grammar

A program opts into standard functions by name:

```zlang
import { generate_int32 } from "@std/random";
import { generate_int32, generate_bytes } from "@std/random";
import { tcp_listener, send_bytes } from "networking";
```

The grammar is:

```text
import "{" identifier ("," identifier)* "}" from string ";"
```

Rules:

- imports are file-scope declarations and must appear before executable statements;
- multiple names from the same domain may appear in one statement;
- multiple import statements for the same domain are allowed;
- no default imports;
- no wildcard imports;
- no aliases in the first version;
- duplicate imported names are rejected;
- an imported name cannot be redeclared by a user function or variable;
- an unimported standard function is not in scope;
- `import` is a source construct, not a keyword attached to a function name.

The import must read as an intention:

```zlang
import { generate_int32 } from "@std/random";
```

This reads as “from randomness, generate an int32”. The following are rejected by naming review:

```zlang
import { random_int32 } from "algorithms";  // domain is not an algorithm namespace
import { generate_int32 } from "rng";       // abbreviation
import { listener } from "inet";            // neither expressive nor full-word
```

## Naming rules

### Domains

A domain is a full, lowercase, expressive noun for a capability. It answers “what kind of capability
is this?”.

- Prefer full words: `networking`, not `inet` or `net`.
- Prefer capability, not implementation: `filesystem`, not `posix`.
- Prefer singular, concrete namespaces: `random`, `text`, `math`.
- Avoid catch-all domains such as `util`, `common`, `core` or `misc`.
- Avoid result types in domain names: `random`, not `random_int32`.

### Functions

A function name answers “what do I want to do?” inside its domain.

- Actions use a verb first: `generate_int32`, `parse_int32`, `encode_hex`, `sort_int32`.
- Resource constructors use protocol or resource plus role: `tcp_listener`, `file_reader`.
- Use a result suffix only when the domain has multiple result shapes: `generate_int32`,
  `generate_bytes`, `parse_uint32`.
- Do not repeat the domain in the function name: `random.generate_int32`, not
  `random.random_int32`.
- Do not use generic names such as `get`, `make`, `handle`, `process`, `manager` or `helper`.
- Do not use abbreviations: `generate_uint32`, not `gen_u32`.

Examples:

```zlang
import { generate_int32, generate_bytes } from "@std/random";
import { tcp_listener, tcp_connect, send_bytes } from "networking";
import { parse_int32, starts_with, to_upper } from "text";
```

## Generated C provenance

Generated C should tell the reader where each name comes from. Prefixes are ownership clues, not
decoration. The 0.20 naming map is:

| Prefix | Owner | Meaning | Example |
| --- | --- | --- | --- |
| `app_fn_` | user source | C translation of a user-defined function | `app_fn_add` |
| `app_local_` | user source | C translation of a user local variable | `app_local_total` |
| `app_param_` | user source | C translation of a user parameter | `app_param_left` |
| `app_struct_`, `app_enum_`, `app_array_` | user source | C type generated from a user declaration | `app_struct_point_t` |
| `zlang_gen_` | compiler | Support derived from user source; not user-editable | `zlang_gen_literal_0` |
| `zlang_std_` | standard library | Registered standard function | `zlang_std_random_generate_int32` |
| `zlang_` | runtime | Public runtime API used by generated C | `zlang_print_i32` |
| `ZLANG_` | language | Compile-time constants and macros | `ZLANG_TRAP_BOUNDS` |

`app_` is the canonical prefix for “this comes from the program I wrote”. It is preferred over
`user_` because the generated file is the compiled application module, and over all-caps role
prefixes because all-caps names read as macros or types. No prefix starts with `_` at file scope, so
generated C avoids C's reserved leading-underscore namespace.

Compiler-generated storage for a source string is source-derived but not user-authored. Its name
uses `zlang_gen_`, not `app_`:

```c
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = {
    zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH
};

zlang_write(zlang_gen_literal_0);
```

The literal's content belongs to the user's program; the C object is compiler support. A bug in the
value comes from the source string. A bug in the C object's declaration or lifetime belongs to the
compiler. This distinction is the point: a reader should know which code to inspect.

Runtime calls keep `zlang_*` because they are documented API. Standard functions keep
`zlang_std_*` because they belong to the language library, not to the user's application. User
functions, locals, parameters and declared types use `app_*` because they map back to source the
user owns.

## Domain catalog

The catalog is the allowed set of top-level domains. New domains require a design decision and a
catalog entry. Function lists below define the allowed catalog; only the functions named above are
implemented.

### Pure and bounded domains

| Domain | Meaning | Proposed functions | Evaluation |
| --- | --- | --- | --- |
| `algorithms` | Operations over fixed arrays and structs | `sort_int32`, `reverse_int32`, `find_int32`, `contains_int32`, `min_int32`, `max_int32`, `sum_int32` | pure, bounded, foldable when inputs are constant |
| `math` | Scalar numeric operations beyond checked builtins | `abs`, `clamp`, `min`, `max`, `sqrt` | pure, overloads resolved by operand types |
| `io` | Process input and output | `write`, `read`, `read_line` | bounded, overloads resolved by operand type |
| `strings` | Bounded string operations | `compare`, `contains`, `starts_with` | pure, non-owning views |
| `encoding` | Byte-to-text and text-to-byte transforms | `encode_hex`, `decode_hex`, `encode_base64`, `decode_base64`, `utf8_length` | pure, bounded, explicit capacity |
| `math` | Scalar numeric operations beyond checked builtins | `abs_int32`, `clamp_int32`, `gcd_int32`, `pow_uint32`, `sqrt_uint32` | pure, foldable when operands are constant |
| `memory` | Direct bounded byte operations | `copy_bytes`, `fill_bytes`, `compare_bytes`, `zero_bytes` | pure, bounded, caller-owned buffers |

### Effectful domains

| Domain | Meaning | Proposed functions | Evaluation |
| --- | --- | --- | --- |
| `random` | Deterministic pseudo-random generation | `generate_int32`, `generate_uint32`, `generate_int64`, `generate_uint64`, `generate_float32`, `generate_float64` | opaque, deterministic, explicit runtime context |
| `time` | Clocks and delays | `now_unix_seconds`, `now_monotonic_millis`, `sleep_millis` | opaque, stateful or platform-dependent |
| `streams` | Input and output streams | `read_line`, `write_string`, `read_bytes`, `write_bytes` | effectful, explicit buffers |
| `filesystem` | Files and directories | `open_file`, `read_file`, `write_file`, `file_exists`, `close_file` | resource-based, explicit close |
| `networking` | Sockets and protocols | `tcp_listener`, `tcp_connect`, `send_bytes`, `receive_bytes`, `close_socket` | resource-based, explicit close |
| `system` | Process and environment access | `exit_process`, `args_count`, `arg_at`, `environment_get` | effectful, platform-dependent |

`generate_float32` and `generate_float64` use the same deterministic PRNG as the int32 generators.
They produce values in `[from, to)` and trap on invalid or non-finite bounds. `cryptography` remains
deferred until there is a stable security and entropy policy. `read` keeps the logical newline in the destination; `read_line` removes it. CRLF is normalized to LF so results are identical on every platform. `@std/strings` is the canonical string domain; `@std/text` is not part of the catalog. Avoid adding
a domain before its capability has a clear boundary.

## Classification dimensions

Every standard function must be classified along all axes below. The registry is invalid if any
axis is missing.

### Lifecycle

- `DRAFT` — usable now, signature may change.
- `UNSTABLE` — signature committed, implementation algorithm may change.
- `STABLE` — signature and observable behavior are committed.

### Safety

- `SAFE` — no hidden global state, no unsafe resource behavior, bounded by declared capacities.
- `UNSAFE` — opaque, entropy-dependent, resource-based, platform-dependent or externally
  effectful.

### Evaluation

- `PURE` — depends only on arguments; eligible for transpile-time evaluation.
- `OPAQUE` — result must not be folded or cached.
- `STATEFUL` — depends on explicit runtime state.
- `EFFECTFUL` — reads or writes external state.

### Determinism

- `DETERMINISTIC` — same inputs and state produce the same result.
- `ENTROPY` — result depends on external entropy.

### Resource model

- `VALUE` — returns only values.
- `BUFFER` — writes into a caller-owned bounded buffer.
- `HANDLE` — returns an explicit resource handle that must be closed.

### Error model

- `TRAP` — invalid input invokes a fixed runtime trap.
- `STATUS` — returns an explicit status value.
- `TOTAL` — valid for every argument allowed by the type system.

### Portability

- `PORTABLE` — works the same on every supported target.
- `PLATFORM` — behavior is target-dependent and documented as such.

### Optimizer contract

- `FOLDABLE` — `--tto` may evaluate it when all inputs are constant.
- `BARRIER` — `--tto` may fold arguments but never the call result.
- `NO_CSE` — repeated calls must not share a cached result.

`random.generate_int32` is `DRAFT`, `UNSAFE`, `OPAQUE`, `ENTROPY`, `VALUE`, `TRAP`, `PORTABLE` and
`BARRIER` until the PRNG decision is made.

## Registration

Each domain owns a registry file. Functions are registered declaratively:

```cmake
zlang_std_function(
    DOMAIN random
    NAME generate_int32
    SIGNATURE "function generate_int32(clone int32 from, clone int32 to): int32"
    C_SYMBOL zlang_std_random_generate_int32
    LIFECYCLE DRAFT
    SAFETY UNSAFE
    EVALUATION OPAQUE
    DETERMINISM ENTROPY
    RESOURCE VALUE
    ERROR TRAP
    PORTABILITY PORTABLE
    OPTIMIZER BARRIER
    DEPENDS "")
```

A function that uses another standard function names the dependency by full registry id:

```cmake
zlang_std_function(
    DOMAIN algorithms
    NAME shuffle_int32
    SIGNATURE "function shuffle_int32(clone array values): array"
    C_SYMBOL zlang_std_algorithms_shuffle_int32
    LIFECYCLE DRAFT
    SAFETY UNSAFE
    EVALUATION OPAQUE
    DETERMINISM ENTROPY
    RESOURCE VALUE
    ERROR TRAP
    PORTABILITY PORTABLE
    OPTIMIZER BARRIER
    DEPENDS "random.generate_int32")
```

Importing `shuffle_int32` activates both `algorithms.shuffle_int32` and `random.generate_int32`.
Dependency activation comes from metadata, not from source import order.

## Generated C activation

Generated C defines one macro per activated function before including the runtime:

```c
#define ZLANG_STD_ALGORITHMS_SHUFFLE_INT32
#define ZLANG_STD_RANDOM_GENERATE_INT32
#include <zlang/runtime/runtime.h>
```

`runtime.h` includes activated headers in two phases. The declaration phase exposes all prototypes;
the implementation phase emits all `static inline` bodies. This allows standard functions to call
each other regardless of include order.

Standard headers never own mutable globals or hidden mutable `static` state. Stateful functions use
an explicit runtime context. If the emitter must thread that context through user functions, it adds
a hidden C parameter to definitions, prototypes and call sites together. Zlang source signatures do
not expose it.

## Testing standard functions

Every registered standard function gets one test module. The directory is a micro CMake project
that can be configured standalone and is also added from `tests/stdlib/`:

```text
tests/stdlib/random/generate_int32/
tests/stdlib/random/generators/
  CMakeLists.txt
  README.md
  test_cases.h
  generate_int32_tests.c
  programs/basic/main.z
  programs/basic/main.c
  programs/basic/expected.txt
```

Rules:

- one directory, one executable, one X-macro case list, following the existing test module model;
- CTest case names are `zlang.stdlib.<domain>.<function>.<case>`;
- labels are `module:stdlib`, `domain:<domain>` and `function:<function>`;
- each function has at least one direct C case using the header-only test framework;
- each function has at least one Zlang integration program using `import`;
- integration programs use plain `.z`, committed golden `.c` and `expected.txt`;
- the generated C is compiled with the strict warning set and then executed;
- configure-time validation rejects a registered function without a matching test directory.

The direct C cases cover boundaries, invalid arguments, traps and explicit state. The Zlang cases
cover import activation, dependency closure, emitter output and runtime linkage. This separates
runtime correctness from language integration.

Portability rules:

- tests depend only on C99 and Zlang headers;
- no third-party test framework;
- no locale, timezone or host entropy in expected stdout;
- `DRAFT`/`UNSAFE` entropy functions assert ranges or structural invariants, not exact values;
- deterministic functions assert exact values;
- `PLATFORM` functions are guarded by compile-time capability checks and test the contract, not
  host-specific output;
- the same suite runs with GCC, Clang, MSVC, clang-cl, MinGW, sanitizers and Valgrind.

Stateful functions use an explicit injected state in their C cases. If deterministic runtime state
is selected, tests use a fixed seed. If OS entropy is selected, tests never depend on a specific
drawn value.

## Validation

Configure-time checks:

- domain exists in the catalog;
- function name follows naming rules;
- `(domain, name)` is unique;
- C symbol is unique;
- header exists and has declaration and implementation sections;
- Zlang signature matches the generated declaration table;
- every classification axis is present;
- dependencies exist as full registry ids;
- dependency closure terminates.

Semantic checks:

- imported domain and name exist;
- duplicate imports are rejected or merged according to the final rule;
- imported name does not shadow a user declaration;
- call arguments match the registered signature;
- unsafe functions are allowed but classified correctly.

## Acceptance criteria for 0.20

- `import { generate_int32, generate_bytes } from "@std/random";` compiles into the expected activation
  macros.
- `import { listener } from "inet";` fails because the domain is not in the catalog.
- `import { listener } from "networking";` fails naming review because the name is not expressive.
- `import { tcp_listener } from "networking";` succeeds.
- `shuffle_int32` activates `random.generate_int32` transitively.
- `generate_int32` is never constant-folded under `--tto`;
- all random generators share one `zlang_runtime_context_t`;
- generated C uses `app_*`, `zlang_gen_*`, `zlang_std_*`, `zlang_*` and `ZLANG_*` consistently;
- every standard function has a micro CMake test project with direct C and Zlang integration cases.
- Pure functions such as `math.clamp_int32` are foldable under `--tto`.
