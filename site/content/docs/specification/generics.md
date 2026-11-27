---
title: Generics
description: Compile-time type parameters, applications and monomorphization.
navigation:
  title: Generics
---

# Generics

A generic declaration lists type parameters after the name. An application supplies concrete type arguments at the use site.

```zlang
struct Pair<T, U> {
    T first;
    U second;
}

function identity<T>(clone T value): T {
    return value;
}
```

```zlang
Pair<int32, boolean> pair;
int32 value = identity<int32>(pair.first);
```

## Interface constraints

A type parameter may require named interfaces inside the generic parameter list:

```zlang
function describe<T implements { Named, Other }>(clone T value): string {
    return value.label();
}
```

The compiler checks each interface when the generic application is specialized. Inside the generic
body, fields and methods declared by the required interfaces are available on values of that type.
Constraints are compile-time checks; they emit no vtable and no runtime lookup.

## Limits

A declaration may list up to four type parameters. An application may pass up to four concrete type arguments. Nested type arguments are not part of the language. Duplicate type parameters, argument count mismatches and invalid applications are diagnostics. Capacity exhaustion is reported instead of failing silently.

Generic structs cannot inherit, use traits or declare constructors or static methods. They may declare flat instance methods. Generic functions must be top-level and have flat bodies. Generic traits, generic inheritance and variadic type packs are outside the contract.

## Monomorphization

The compiler monomorphizes each used application into a concrete C99 type or function with substituted type names. There is no runtime generic value, no runtime type lookup and no reflection.

For the example above, the emitted C contains one concrete struct and one concrete function:

```c
typedef struct app_struct_pair_int32_boolean_t {
    int32_t first_;
    bool second_;
} app_struct_pair_int32_boolean_t;

int32_t app_fn_identity_int32(int32_t app_param_value) {
    return app_param_value;
}
```

The generated identifiers derive from the concrete application. Two applications with different arguments produce two independent specializations.

## Native results

`result_type<T, status_code>` uses the same application syntax. Unlike user generics, its applications form a closed family mapped to fixed C layouts. See [Types and values](/docs/specification/types).

`auto` may replace a native generic argument when the initializer fixes it:

```zlang
result_type<auto, status_code> result = query();
```

The compiler resolves the argument from the initializer before validation and emits the concrete native result type.

## Generic methods

A generic struct can declare flat instance methods. The compiler copies the method into each specialization with the same substituted type names used for fields and signatures. Names such as `app_fn_box_int32_get` are emitted as ordinary C functions; no vtable or runtime dispatch is involved.

Native `result_type<T, status_code>` values expose `is_ok()`, `get()` and `message()`. These methods lower to direct C field reads and `zlang_std_status_is_ok`; they are not user-defined generic methods.
