# Conditions

Exercises branching and logical operators with two scalar values.

This example exercises:

- `if`, `else if` and `else`;
- equality and relational comparisons;
- `&&`, `||` and `!`;
- grouping `!` around a comparison;
- printing boolean results as scalars.

The generated C keeps grouped comparisons parenthesized so `!(value > 10)` does not become
`(!value) > 10`.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.conditions
```
