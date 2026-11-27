# constraints

Shows a generic function constrained by a named interface. `show<T implements { Labeled }>` can call
`label()` because the interface supplies the method contract. `Counter` satisfies the constraint
through its `implements` declaration.

The program prints `7`.
