---
title: Conformance
description: How the language contract is checked in emitted C.
navigation:
  title: Conformance
---

# Conformance

A conformance test may assert on emitted C text without running the program:

```text
parse("int32 var = 10;").transpile().contains("int32_t var = 10;")
```

```text
parse("array[] int32 values;").diagnostics().contains("ZLANG3001")
```

These assertions keep the specification tied to the emitter. If the translation changes, the contract changes with it.

## Requirement areas

| Area | Representative tests |
| --- | --- |
| declarations and literal emission | `emits_a_scalar_declaration`, `covers_every_annex_a_production` |
| branches, loops and loop jumps | `emits_if_and_else`, `emits_a_native_foreach_loop`, `emits_repeat_as_native_for`, `emits_native_for_loops`, `emits_break_and_continue` |
| arithmetic and conversions | `emits_arithmetic_operators`, `emits_compound_assignments`, `emits_explicit_numeric_conversions`, `emits_checked_unary_negation` |
| strings and arrays | `folds_literal_string_concatenation`, `emits_bounded_variable_string_concatenation`, `emits_an_inferred_sequence_initializer`, `emits_nested_array_initializer`, `emits_multidimensional_foreach` |
| bounds and capacity | `checked_index_reads_return_values`, `keeps_unproven_bounds_traps`, `folds_proven_loop_indices`, `rejects_an_unbounded_sequence_at_the_declaration` |
| functions and parameter modes | `emits_function_and_return`, `emits_function_prototypes`, `emits_parameter_qualifiers`, `passes_complex_function_arguments` |
| structs, enums and methods | `emits_structs_and_enums`, `emits_struct_constructor_and_methods`, `emits_static_struct_method` |
| inheritance and traits | `emits_struct_inheritance_and_vtable`, `emits_dynamic_dispatch_call`, `emits_trait_method_direct_call`, `rejects_downcast` |
| generics | `emits_generic_struct_specialization`, `emits_generic_function_specialization`, `rejects_generic_argument_count_mismatch`, `rejects_duplicate_generic_parameter` |
| native results | `emits_native_result_type`, `assigns_native_result_fields` |
| uniform stdlib result contract | `emits_stream_read_result`, `emits_stream_write_result`, `emits_stream_string_write_result`, `emits_native_result_methods` |
| imports and modules | `emits_a_write_call`, `zlang.cli.modules`, `zlang.cli.modules_cycle`, `zlang.cli.modules_escape` |
| standard library | `emits_scalar_write`, `emits_io_read_calls`, `emits_math_overloads`, `emits_strings_api`, `zlang.stdlib.random.generate_int32.basic` |
| diagnostics and source locations | `reports_syntax_diagnostics_with_source_locations`, `reports_type_and_resource_diagnostics_with_source_locations`, `zlang.cli.diagnostic_format` |
| generated C and golden examples | `zlang.cli.compiles_generated_c`, `zlang.cli.disable_comments.output`, `zlang.examples.*` |

The maintained matrix lives in `tests/spec/test_cases.h`. Every production in the [grammar](/docs/specification/grammar) and every stable diagnostic has a case in that file.
