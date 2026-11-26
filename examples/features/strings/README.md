# Strings

Builds a folded string literal, concatenates it with another literal into a bounded string, then
reads and reassigns string values.

This example exercises:

- compile-time folding of literal concatenation;
- `string[N]` bounded concatenation;
- string indexing returning an uint32 byte;
- assignment of a string literal to a bounded string;
- printing strings and scalar bytes.

The golden C shows both a folded `_zlang_literal_0` and a `zlang_string_concat` call.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.strings
```
