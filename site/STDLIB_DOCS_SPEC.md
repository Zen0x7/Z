# Standard library documentation specification

## Goal

The standard library section must work as a real API reference, not as a single introduction page.

It needs to serve two readers:

1. A newcomer who asks: what exists, how do I import it, and where do I start?
2. A working programmer who asks: exact signature, bounds, ownership, traps, generated C symbol, and stability.

The documentation is human-authored prose around machine-verifiable metadata. Doxygen remains the generated source-level reference. This section is the canonical language-facing reference.

## Information architecture

Route root:

```text
/docs/standard-library
```

Recommended tree:

```text
content/docs/standard-library/
  index.md
  conventions.md
  classification.md
  generated-c.md

  io/
    .navigation.yml
    index.md
    catalog.md
    functions/
      write.md
      read.md
    types/
      bounded_string.md

  random/
    .navigation.yml
    index.md
    catalog.md
    functions/
      generate_int32.md
      generate_bytes.md

  text/
    .navigation.yml
    index.md
    catalog.md
    functions/
      parse_int32.md
      starts_with.md

  algorithms/
  math/
  memory/
  time/
  streams/
  filesystem/
  networking/
  system/
```

Rules:

- One domain equals one folder.
- One function equals one page.
- One record, enum, handle or other exposed type equals one page under `types/`.
- Proposed functions are not documented as implemented. They may appear in a catalog table with status `proposed`.
- Domain slugs are lowercase and canonical: `io`, `random`, `text`, not `inet`, `rng` or `str`.
- File names are lowercase snake_case: `generate_int32.md`, `bounded_string.md`.

## Sidebar

The sidebar label must be **Sections**, not **Guides**.

Navigation is generated from Nuxt Content with `queryCollectionNavigation('content')`. The sidebar component is recursive. It must not know about domain names.

Expected top-level sections:

```text
Documentation
  Overview
  Quick start
  Language
  Compiler
  Standard library
    Overview
    Conventions
    Classification
    Generated C
    Domains
      Inputs and Outputs
        Overview
        Catalog
        Functions
          write
          read
        Types
          bounded string
      Randomness
        ...
```

Human-readable domain labels come from `.navigation.yml`:

```yaml
# content/docs/standard-library/io/.navigation.yml
title: Inputs and Outputs
navigation:
  title: Inputs and Outputs
  icon: io
```

The path still uses `@std/io` or `io`; the label can be human-friendly.

## Common frontmatter

Every page uses frontmatter. No page relies on implicit title inference.

```yaml
---
title: write
description: Write a bounded string or scalar to the standard output stream.
domain: io
kind: function
status: stable
stability: STABLE
safety: SAFE
evaluation: EFFECTFUL
determinism: DETERMINISTIC
resource: VALUE
error: TOTAL
portability: PORTABLE
optimizer: BARRIER
since: 0.25.0
canonical_symbol: zlang_std_io_write
activation_macro: ZLANG_STD_IO_WRITE_STRING
---
```

Allowed `kind` values:

```text
domain-overview
catalog
function
type
classification
generated-c
conventions
```

Status values:

```text
implemented
draft
proposed
```

Only `implemented` functions get a full function page. `proposed` functions belong only in catalog tables with a clear status marker.

## Templates

### 1. Domain overview

Route: `/docs/standard-library/<domain>`

Sections:

1. What the domain is for.
2. Canonical specifier: `@std/<domain>`.
3. Import example.
4. What is implemented now.
5. What is proposed but not available.
6. Safety and state model.
7. Links to catalog and function pages.

Voice example:

```text
Inputs and Outputs gives Zlang programs a bounded way to read and write data.
Nothing is buffered implicitly. Every read needs caller-owned storage.
```

### 2. Domain catalog

Route: `/docs/standard-library/<domain>/catalog`

A table with one row per function or type.

Columns:

```text
Name | Status | Signature | Purpose | Resource | Error | Notes
```

Example:

```markdown
| `write` | implemented | `function write(clone string value): void` | Write a bounded string | `VALUE` | `TRAP` | Activation macro ... |
```

Rules:

- Signature is shown verbatim from the registry.
- Status is visible in every row.
- Proposed entries never link to function pages.

### 3. Function page

Route: `/docs/standard-library/<domain>/functions/<function>`

Required sections, in this order:

1. **Summary** — one sentence, active voice.
2. **Import** — the exact import line.
3. **Signature** — canonical signature in a `zlang` block.
4. **Parameters** — name, type, mode, meaning, capacity requirements.
5. **Result** — return type, writes into caller storage, or `void`.
6. **Bounds and ownership** — what must be declared before the call.
7. **Errors and traps** — exact trap name or status value.
8. **State and concurrency** — explicitly say if it is stateless or uses an injected context.
9. **Generated C** — activation macro and C symbol.
10. **Example** — small complete program.
11. **See also** — sibling functions and domain catalog.

A function page must answer all of these:

- What do I import?
- What do I pass?
- What do I own?
- What can fail?
- What does the generated C call?
- Is it safe to call twice?
- Is the result deterministic?

### 4. Type page

Route: `/docs/standard-library/<domain>/types/<type>`

Required sections:

1. Summary.
2. Declaration or runtime shape.
3. Capacity rules.
4. Field table, if any.
5. Lifetime and ownership.
6. Copy/clone behavior.
7. Generated C representation.
8. Example.

Use `types/`, not `classes/`, unless the language introduces an actual `class` construct.

### 5. Classification page

Route: `/docs/standard-library/classification`

Explain every axis in plain language before listing values:

| Axis | Meaning |
| --- | --- |
| Lifecycle | How committed the signature and behavior are |
| Safety | Whether the function can hide global state or unsafe resource behavior |
| Evaluation | Whether the result may be folded at compile time |
| Determinism | Whether the same inputs always produce the same result |
| Resource | Value, caller-owned buffer, or explicit handle |
| Error | Fixed trap, explicit status, or total function |
| Portability | Same behavior everywhere or target-dependent |
| Optimizer | `FOLDABLE`, `BARRIER`, or `NO_CSE` |

### 6. Generated C page

Route: `/docs/standard-library/generated-c`

Explain:

- one activation macro per imported function;
- transitive activation from registry dependencies;
- two-phase runtime include;
- provenance prefixes;
- no mutable globals in standard headers;
- hidden context parameters when stateful functions require them.

This page is the bridge to Doxygen, not a replacement for it.

## Component requirements

### Sidebar

`DocsNav.vue` becomes recursive:

- accepts a navigation tree node;
- renders nested `<ul>` groups;
- marks active path and active ancestors;
- uses square nodes, no rounded pills;
- labels the root block **Sections**.

### Page header

Every page shows:

- eyebrow with domain label;
- title;
- description;
- status badges from frontmatter.

### Function badge row

Badges are generated from frontmatter:

```text
STABLE · SAFE · EFFECTFUL · DETERMINISTIC · VALUE · TRAP · PORTABLE · BARRIER
```

Never invent a value not present in the registry.

### Catalog tables

Tables use canonical signatures and stable slugs. They are generated where possible from the registry, with prose kept in markdown.

## Source of truth

The registry is authoritative for:

- domain;
- function name;
- signature;
- C symbol;
- activation macro;
- lifecycle;
- safety;
- evaluation;
- determinism;
- resource;
- error;
- portability;
- optimizer;
- dependencies.

Markdown is authoritative for:

- explanation;
- examples;
- caveats;
- import guidance;
- links between concepts.

If a registry value changes, the generated frontmatter or generated table must change with it. A human should not manually rewrite eight classification fields across ten pages.

## Writing rules

- One topic per paragraph.
- Active voice.
- Put concrete action before terminology.
- Explain the nearest concept first.
- Show an example before explaining constraints.
- Every example must compile with the documented version.
- Do not use “simply”, “just”, “obviously”, or “as you can see”.
- Do not call a proposed feature implemented.
- Do not use `class` for Zlang records or handles.
- Use `@std/<domain>` for canonical specifiers.
- Use `write`, never `print`, for current examples.

## Rollout order

1. Rebuild `DocsNav.vue` as recursive.
2. Replace the current `standard-library.md` with a hub page.
3. Add `conventions.md`, `classification.md`, `generated-c.md`.
4. Document `@std/io` first: overview, catalog, `write`, `read`, bounded string type.
5. Add frontmatter validation.
6. Generate catalog tables from the registry.
7. Repeat the same template for each implemented domain.
8. Add proposed domains only as catalog stubs.

## Acceptance criteria

The section is done when:

- the sidebar says **Sections**;
- nested pages are reachable from the sidebar;
- every implemented function has one page;
- every function page has all eleven required sections;
- no proposed function is presented as available;
- signatures and classifications match the registry;
- generated C symbols and macros match the golden C;
- examples use current `@std/io` and explicit `main`;
- a new domain can be added by copying one template and filling registry-backed fields.
