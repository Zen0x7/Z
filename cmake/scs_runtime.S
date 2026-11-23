// AArch64 shadow-call-stack link-check runtime.
//
// Clang's ShadowCallStack instrumentation uses x18 as the shadow stack pointer,
// but compiler-rt does not provide a startup runtime. This object exists so the
// instrumented test executables can be linked in CI; the shadow-call-stack job
// verifies compilation and linking only, not execution.
//
// Execution is not supported with system glibc: glibc is not built with
// -ffixed-x18 and clobbers x18 inside ordinary libc calls, so an instrumented
// caller loses its shadow stack pointer. A real runtime would require a libc
// and dynamic loader built with x18 reserved, not just wrappers around a few
// functions.

    .section .bss
    .balign 16
    .globl zen_scs_shadow_stack
    .type zen_scs_shadow_stack, @object
zen_scs_shadow_stack:
    .zero 65536
    .size zen_scs_shadow_stack, .-zen_scs_shadow_stack

    .section .text
    .globl zen_scs_runtime_init
    .type zen_scs_runtime_init, @function
zen_scs_runtime_init:
    adrp x0, zen_scs_shadow_stack
    add x0, x0, :lo12:zen_scs_shadow_stack
    mov x18, x0
    ret
    .size zen_scs_runtime_init, .-zen_scs_runtime_init

    // Uninstrumented entry point for the link check. x18 is written directly
    // instead of through x0 so argc and argv stay intact. The C entry point is
    // renamed to zen_main_impl by ZenTesting.cmake in this mode.
    .globl main
    .type main, @function
main:
    adrp x18, zen_scs_shadow_stack
    add x18, x18, :lo12:zen_scs_shadow_stack
    b zen_main_impl
    .size main, .-main

    .section .init_array, "aw"
    .balign 8
    .quad zen_scs_runtime_init
