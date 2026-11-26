---
title: Language Specification
description: The normative contract for Zlang source, static meaning and emitted C.
navigation:
  title: Overview
---

# Language Specification

Current Version: v0.38

This specification defines Zlang. It fixes source syntax, static meaning and the C that `zc` emits. It does not define compiler internals, command-line options or CI. `ROADMAP.md` records how the language reached this state.

The rule behind the language:

> A program is a set of compile-time decisions that produces ordinary C. Nothing is decided at runtime that could have been decided at compile time.

## Scope

Zlang is source-to-source. It compiles to C99. Generated C calls the `zlang_*` runtime, and a native toolchain produces the executable.

Zlang has no virtual machine, no intermediate bytecode, no runtime type metadata, no runtime names and no garbage collector. Names, types, capacities, parameter modes and storage locations exist only in the compiler.

## Conformance

An implementation conforms when it:

1. accepts every program described by the [grammar](/docs/specification/grammar);
2. rejects every unbounded array at the declaration site with a resource diagnostic;
3. emits C with no call to `malloc`, `calloc`, `realloc`, `free`, `strdup` or any equivalent allocation service;
4. emits native C control flow, with no callback indirection except the internal vtables required by inheritance;
5. makes the output of `--emit-c` byte-for-byte identical to the C that is compiled by default;
6. preserves the observable output order of `write` calls.

A conforming program satisfies every static rule in this specification.

## Normative references

- ISO/IEC 9899:1999 (C99), the target output language.
- `include/zlang/**`, the public C API that generated code may call.
- The [grammar](/docs/specification/grammar) and [diagnostics](/docs/specification/diagnostics) clauses of this specification.

No external language standard is incorporated by reference. Zlang defines its own types, operations and grammar.

## Notational conventions

- `<type>`, `<name>` and `<expression>` are metavariables.
- “Must” states a requirement on a conforming implementation.
- “May” states an implementation-defined choice that must be documented.
- “Emits” states the exact C form. Whitespace may differ; tokens may not.
- Line and column numbers are one-based and refer to the original source file.

## Planned 0.39 work

Labeled jumps and `switch` are implemented in this working tree. The remaining planned syntax
forms are value-producing `match`, ternary `?:`,
bitwise and shift operators, `do while`, exclusive `uint32` range literals, and labeled loop jumps.
The trait model is also planned to change: interfaces become named field-and-method contracts,
traits require interfaces, and structs declare `implements` interfaces and `use` traits.

These forms are not part of 0.38 and are not accepted by `zc` until the 0.39 emitter lands.

## Language limits

Runtime reflection, runtime lookup by name, heap-backed values, dynamically grown storage, generic traits, generic inheritance and variadic type packs are outside this contract.
