# generics

Shows compile-time generics on a struct and a function. `Pair<T, U>` is instantiated as
`Pair<int32, boolean>`, and `identity<int32>` is specialized for `int32`. The compiler
monomorphizes both applications into ordinary C99 declarations; no runtime generic representation
or dynamic allocation is emitted.

The program prints `7 true`.
