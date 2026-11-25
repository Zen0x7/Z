# Comments

Adds `//` comments at file scope, on a record field, on a function and inside a body.

This example exercises:

- lexing comments as trivia attached to source ranges;
- emitter placement by source construct rather than emission order;
- preservation of comments in the generated C golden.

Run `zlang-compile main.zlang --disable-comments --emit-c` to see the output without them.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.comments
```
