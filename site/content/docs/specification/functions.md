---
title: Functions, methods and traits
description: Parameter modes, methods, single inheritance and static traits.
navigation:
  title: Functions and methods
---

# Functions, methods and traits

## Functions

```zlang
function <name>(<parameter-list>): <return-type> {
    <statements>
}
```

A parameter is `<const?> <mode> <type> <name>`, where `<mode>` is `clone` or `reference`. Every parameter must state a mode.

```zlang
function add(clone int32 left, clone int32 right): int32 {
    return left + right;
}
```

`clone` passes a value copy. Mutation affects only the local parameter. `reference` passes an alias to the caller's lvalue; mutation is visible to the caller. `const clone` is a read-only copy and `const reference` is a read-only alias. A mutable `reference` cannot bind a const argument, and reference arguments must be lvalues. References cannot be stored or returned.

A `reference` parameter of a small value type with no mutational purpose produces `ZLANG3004`. A mutable reference is warned about when the body never mutates it; `const reference` to a scalar, string or enum is always warned about. Mutated references and references to structs are accepted.

A function with no return type emits `void`. A function with no parameters emits `(void)` in C, and unused parameters receive `(void)` guards. User functions are declared before their definitions, so a function may call one defined later. Functions have no access to mutable globals, because Zlang has none.

## Methods

```zlang
struct Counter {
    int32 value;

    constructor(clone int32 initial) {
        value = initial;
    }

    function increment(): int32 {
        value += 1;
        return value;
    }

    static function zero(): Counter {
        return Counter(0);
    }
}
```

Inside a constructor or instance method, fields are referenced without an explicit receiver. The emitter supplies `self` in generated C. Instance calls use `value.method()` and static calls use `Struct.method()`. A method name cannot collide with a field name.

## Inheritance

```zlang
struct Square : Shape {
    int32 side;
}
```

The base is the first C subobject of the derived struct. A derived value can be used where a base value or `reference Base` is expected. The reverse conversion is rejected. Arrays keep their exact element types and do not participate in inheritance.

Instance methods are virtual by default. A polymorphic hierarchy emits one internal vtable type and one vtable instance per concrete struct. The root subobject carries an internal type tag and vtable pointer. A call through a base receiver uses the vtable slot for the method name. The emitter derives dispatch metadata from the AST, so it has no fixed vtable or type-tag capacity. No dynamic allocation is introduced.

## Traits

```zlang
interface Named {
    int32 value;
    function label(): int32;
}

trait NamedTrait requires Named {
    function label(): int32 {
        return self.value;
    }
}

struct Shape implements Named {
    use NamedTrait;

    int32 value;
}
```

An interface declares a named contract of fields and method signatures. A trait requires one or more interfaces and supplies method bodies. A struct declares `implements` interfaces and opts into behavior with `use`. The compiler checks that every implemented interface field and method exists directly or through a used trait, and that every required interface is implemented. Trait methods become concrete methods of the using struct and are called directly. They do not use the vtable and cannot override inherited methods. Trait method collisions and invalid overrides are rejected.

## Generic functions

A top-level function may declare type parameters and be applied to concrete type arguments. Generic functions must have flat bodies. See [Generics](/docs/specification/generics) for the full contract.
