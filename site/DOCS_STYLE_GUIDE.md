# Documentation style guide

This is the contract for writing Zlang documentation. Agents must follow it before writing prose.

## Scope

Document what is implemented and stable. Do not document work in progress as API.

The following are out of scope until they are implemented and stable:

- inheritance;
- polymorphism;
- traits;
- proposed standard library domains;
- functions present only in a catalog but not registered.

When a planned feature must be mentioned, mark it as proposed and do not give it a reference page.

## Source of truth

Use these sources, in this order:

1. `AGENTS.md` for naming and repository rules.
2. `SPEC.md` for language semantics.
3. `compiler/include/zlang/compiler/stdlib.h` and `compiler/src/stdlib.c` for registered functions.
4. `include/zlang/standard/**` for runtime headers and C symbols.
5. Golden fixtures under `examples/**` for working programs and generated C.
6. `site/STDLIB_DOCS_SPEC.md` for standard library structure.
7. Existing canonical pages in `site/content/docs/**` for voice and layout.

Never infer an API from a roadmap or from a filename.

## Voice

- One topic per paragraph.
- Begin with the concrete action, then the technical name.
- Use active voice.
- Use definite, specific language.
- Omit needless words.
- Do not write marketing copy.
- Do not use “simply”, “just”, “obviously” or “as you can see”.
- Do not stack adjectives.
- Explain the nearest concept first.
- Put details in the page that owns them, not in every page.

Good:

```text
Imports activate standard functions by name. The compiler emits only the runtime headers those imports need.
```

Bad:

```text
Zlang provides a powerful, expressive and deterministic import mechanism for robust modular development.
```

## Page model

Every page has frontmatter:

```yaml
---
title: Page title
description: One sentence that says what the page answers.
navigation:
  title: Short sidebar label
---
```

Standard library pages add domain, kind, status and registry-backed fields. See `STDLIB_DOCS_SPEC.md`.

## Headings

- Use one H1 per page.
- Use H2 for sections that answer a distinct question.
- Use H3 sparingly, only inside a long H2.
- Headings are nouns or short questions, not marketing claims.

Good section titles:

```text
Import
Signature
Parameters
Bounds and ownership
Generated C
Example
```

## Code examples

- Use the current language surface: explicit `main`, `write` from `@std/io`, no `print`.
- Every example must be small enough to read at a glance.
- An example must compile with the documented version.
- Do not show invented C output. Use golden C or a labeled excerpt.
- If registry and generated C disagree, say so explicitly.
- Use `zlang` for Zlang blocks, `c` for C, `console` for commands and output.

Example shape:

```zlang
import { write } from "@std/io";

function main(): integer {
    write("Hello from Zlang");
    write("\n");
    return 0;
}
```

## Navigation

- Left sidebar label: **Sections**.
- The sidebar root follows the current level.
- Groups with an index page are links.
- Groups without an index page may be collapsible.
- Breadcrumbs go above the article and are clickable.
- Right-side ToC is **On this page**.

## Standard library template

Use the canonical pilot under:

```text
content/docs/standard-library/namespaces/io/
```

Copy its structure, not its prose.

## Agent brief

Before editing, an agent must:

1. Read this guide.
2. Read the relevant `SPEC.md` clauses.
3. Read the registry and generated header for the API.
4. Read one canonical page of the same kind.
5. Write only files inside its assigned directory.
6. Avoid global config, navigation components and unrelated docs.

After editing, the agent reports:

- files changed;
- source files used;
- any mismatch found between spec, registry and generated C;
- anything left deliberately undocumented.

## Review gate

A page is accepted only when:

- every claim has a source;
- no WIP feature is documented as available;
- signatures match the registry;
- examples use current syntax;
- navigation path is reachable;
- description is present;
- prose follows this guide.
