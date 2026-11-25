---
title: Standard library
description: The canonical reference for domains, imports, functions, types and generated C activation.
kind: overview
status: implemented
navigation:
  title: Overview
---

# Standard library

The standard library is a set of domains. You import functions by name, and the compiler activates only the runtime headers those imports need.

This section is the language-facing reference. It documents what you write in Zlang, what the compiler guarantees, and which C symbols the generated code calls.

Start with [conventions](/docs/standard-library/conventions) if you are new to imports. Use the domain catalogs when you need a specific function.

## Implemented domains

| Domain | Specifier | Status |
| --- | --- | --- |
| Inputs and Outputs | `@std/io` | implemented |
| Randomness | `@std/random` | implemented |
| Math | `@std/math` | implemented |
| Strings | `@std/strings` | implemented |
| Program options | `@std/program_options` | implemented |

Other domains such as `text`, `encoding`, `algorithms`, `memory`, `time`, `streams`, `filesystem`, `networking` and `system` belong to the catalog. Do not treat them as available until a function page says so.

## How to read this section

Each domain has:

- an overview;
- a catalog;
- one page per implemented function;
- one page per exposed type, when the domain has one.

Function pages follow the same template: import, signature, parameters, result, bounds, errors, state, generated C, example and related pages.

## Reference

- [Conventions](/docs/standard-library/conventions)
- [Classification](/docs/standard-library/classification)
- [Generated C](/docs/standard-library/generated-c)
- [Namespaces](/docs/standard-library/namespaces)
