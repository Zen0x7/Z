# inheritance_traits

Shows single inheritance with virtual dispatch and a trait with static behavior reuse. `Shape`
declares `area()` and the `Named` trait provides `label()`. `Square : Shape` inherits the base
subobject and overrides `area()`. `describe(reference Shape)` calls through the vtable, so the same
call dispatches to `Shape.area()` for `base` and `Square.area()` for `square`. Trait calls are direct
calls compiled into the using struct.

The program prints `2 16 2 3`.
