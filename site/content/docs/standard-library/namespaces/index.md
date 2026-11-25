---
title: Overview
description: Domain namespaces available in the standard library.
kind: overview
status: implemented
navigation:
  title: Overview
---

# Namespaces

A namespace is a standard library domain. Each one groups functions and types under a canonical specifier such as `@std/io`.

## Available namespaces

| Namespace | Specifier | Description |
| --- | --- | --- |
| [Inputs and Outputs](/docs/standard-library/namespaces/io) | `@std/io` | Bounded input and output for Zlang programs. |
| [Randomness](/docs/standard-library/namespaces/random) | `@std/random` | Opaque pseudo-random generation for numeric types. |
| [Math](/docs/standard-library/namespaces/math) | `@std/math` | Bounded scalar math overloads. |
| [Strings](/docs/standard-library/namespaces/strings) | `@std/strings` | Bounded string comparison and search. |
| [Program options](/docs/standard-library/namespaces/program_options) | `@std/program_options` | Explicit access to host arguments and flags. |

More namespaces will appear here as their function references are written. A catalog entry alone does not mean the domain is implemented.

## Reading a namespace

Every implemented namespace has:

- an overview with the domain model;
- a `Functions` catalog with one page per function;
- type notes inside the overview when the namespace exposes no dedicated struct.

The sidebar follows the same levels.
