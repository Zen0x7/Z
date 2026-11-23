#ifndef ZLANG_COMPILER_DIAGNOSTICS_H
#define ZLANG_COMPILER_DIAGNOSTICS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Codes match the categories published in SPEC.md. */
#define ZLANG_DIAGNOSTIC_UNEXPECTED_TOKEN 1001U
#define ZLANG_DIAGNOSTIC_UNTERMINATED_STRING 1002U
#define ZLANG_DIAGNOSTIC_INVALID_CHARACTER 1003U
#define ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT 1004U
#define ZLANG_DIAGNOSTIC_LITERAL_OUT_OF_RANGE 2001U
#define ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION 2002U
#define ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER 2003U
#define ZLANG_DIAGNOSTIC_TYPE_MISMATCH 2004U
#define ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE 3001U

typedef struct zlang_diagnostic_t {
    uint16_t code_;
    uint32_t line_;
    uint32_t column_;
    const char *message_;
} zlang_diagnostic_t;

typedef struct zlang_diagnostics_t {
    zlang_diagnostic_t *items_;
    size_t capacity_;
    size_t count_;
} zlang_diagnostics_t;

void zlang_diagnostics_init(zlang_diagnostics_t *diagnostics, zlang_diagnostic_t *items,
                            size_t capacity);

bool zlang_diagnostics_add(zlang_diagnostics_t *diagnostics, uint16_t code, uint32_t line,
                           uint32_t column, const char *message);

bool zlang_diagnostics_contains(const zlang_diagnostics_t *diagnostics, uint16_t code);

#endif /* ZLANG_COMPILER_DIAGNOSTICS_H */
