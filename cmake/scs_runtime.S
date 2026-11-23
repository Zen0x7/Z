// Minimal AArch64 shadow-call-stack runtime.
//
// Clang's ShadowCallStack instrumentation uses x18 as the shadow stack pointer,
// but compiler-rt does not provide a startup runtime. Without this constructor
// x18 is zero on entry to the first instrumented function and every test
// segfaults. The constructor is plain assembly, so it is not instrumented
// itself, and it runs from .init_array before main() and before any other
// instrumented code.

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

    // glibc is not built with -ffixed-x18 and may use x18 between the
    // .init_array constructor and main(). Reset it in an uninstrumented
    // assembly wrapper that tail-calls the real C entry point. The macro
    // `main=zen_main_impl` is defined by ZenTesting.cmake in this mode.
    .globl main
    .type main, @function
main:
    adrp x18, zen_scs_shadow_stack
    add x18, x18, :lo12:zen_scs_shadow_stack
    b zen_main_impl
    .size main, .-main

    // glibc is not built with -ffixed-x18 and clobbers it inside printf,
    // fprintf and strcmp. Save x18 around each of those calls so instrumented
    // code sees the shadow stack pointer again on return. Link with
    // -Wl,--wrap=<name> for each shim.
    .macro ZEN_SCS_LIBC_SHIM name, real
    .globl \name
    .type \name, @function
\name:
    sub sp, sp, #32
    str x30, [sp, #24]
    str x18, [sp, #16]
    bl \real
    ldr x18, [sp, #16]
    ldr x30, [sp, #24]
    add sp, sp, #32
    ret
    .size \name, .-\name
    .endm

    ZEN_SCS_LIBC_SHIM __wrap_printf, __real_printf
    ZEN_SCS_LIBC_SHIM __wrap_fprintf, __real_fprintf
    ZEN_SCS_LIBC_SHIM __wrap_strcmp, __real_strcmp

    .section .init_array, "aw"
    .balign 8
    .quad zen_scs_runtime_init
