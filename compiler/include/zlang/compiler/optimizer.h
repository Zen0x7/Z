/** @file
 * @brief Transpile-time optimizer API.
 */

#ifndef ZLANG_COMPILER_OPTIMIZER_H
#define ZLANG_COMPILER_OPTIMIZER_H

#include "zlang/compiler/ast.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ZLANG_TTO_WORKLIST_CAPACITY 256U
#define ZLANG_TTO_MAX_ROUNDS 64U

typedef struct zlang_optimizer_context_t {
    size_t worklist_[ZLANG_TTO_WORKLIST_CAPACITY];
    size_t worklist_count_;
    size_t worklist_front_;
    size_t generation_;
    uint32_t round_count_;
    bool queued_[ZLANG_TTO_WORKLIST_CAPACITY];
} zlang_optimizer_context_t;

void zlang_optimizer_context_init(zlang_optimizer_context_t *context);

/* Rewrites the mutable program in place until a fixed point is reached or the
   caller-owned worklist budget is exhausted. Returns false only on invalid AST
   indices; budget exhaustion leaves a valid partially optimized program. */
bool zlang_optimize_program(zlang_program_t *program, zlang_optimizer_context_t *context);

#endif /* ZLANG_COMPILER_OPTIMIZER_H */
