# Scopes

Declares an inner variable in a user-authored block and reads both it and an outer variable.

This example exercises:

- a real source block, which emits one C block;
- access to an outer variable from an inner scope;
- no shadowing of the outer declaration;
- no synthetic C blocks for compiler temporaries.

The golden C contains exactly one standalone `{}` scope, matching the source block.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.scopes
```
