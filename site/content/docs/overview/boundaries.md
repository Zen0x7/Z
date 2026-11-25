---
title: Boundaries
description: What Zlang includes and what it leaves out.
navigation:
  title: Boundaries
---

# Boundaries

Zlang has a narrow scope.

## Included

- fixed-size arrays and bounded strings;
- checked arithmetic;
- structs and enums;
- functions with `clone` and `reference` parameters;
- native loops;
- explicit `main`;
- domain-based standard library imports;
- C99 output.

## Not included

- virtual machine or bytecode;
- garbage collector;
- hidden heap allocation;
- runtime name lookup;
- dynamic array growth;
- classes, inheritance or objects;
- traits or polymorphism.

These are non-goals, not missing features waiting to be documented.

## Work in progress

Language work that is not stable is not part of this reference. A feature appears here only when the compiler implements it and golden examples cover it.
