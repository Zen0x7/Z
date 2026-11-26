---
title: Lexical grammar
description: Source text, tokens, literals, keywords and comments.
navigation:
  title: Lexical grammar
---

# Lexical grammar

A source file is UTF-8. Identifiers and keywords use ASCII characters. String literals may contain any UTF-8 byte sequence except an unescaped double quote.

## Tokens

| Element | Rule |
| --- | --- |
| Identifier | `[A-Za-z_][A-Za-z0-9_]*` |
| Integer literal | decimal digits with an optional `u`, `i64` or `u64` suffix |
| Floating literal | decimal digits with `.` or an exponent, with an optional `f` suffix |
| String literal | `"..."` with escapes `\"`, `\\`, `\n` and `\t` |
| Comment | `//` to end of line, or `/* ... */` |
| Whitespace | space, tab, carriage return and line feed |

These words are reserved and cannot be used as identifiers:

```text
arguments array boolean break clone const constructor continue double else enum false float for
foreach from function if import in int32 int64 length reference repeat requires return static
string struct trait true uint32 uint64 uses while
```

The generated-code namespace is also reserved. An identifier that begins with `zlang`, `_zlang` or `ZLANG` is rejected with `ZLANG1004`.

## Numeric literals

An unsuffixed int32 literal has type `int32`. The `u` suffix gives `uint32`, `i64` gives `int64` and `u64` gives `uint64`. A floating literal has type `double`; the `f` suffix gives `float`. A literal outside the range of its type produces `ZLANG2001`.

## Comments

Comments are lexed but never become parser tokens. The emitter writes them back into the generated C next to the construct they document: a comment before a declaration is emitted before that declaration, a comment inside a struct is emitted before the matching field, and a comment inside an enum is emitted before the matching enumerator. Consecutive `//` lines stay together as one block, and block comments keep their internal line breaks. Comments that do not document a construct are emitted at the start of `main`.

Comments have no semantic effect. `--disable-comments` omits them from the generated C.
