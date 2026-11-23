#ifndef ZLANG_COMPILER_PLANNER_H
#define ZLANG_COMPILER_PLANNER_H

#include "zlang/compiler/ast.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* The memory planner maps declarations to storage classes. It performs no
   allocation: the caller owns the entry array. */

#define ZLANG_DEFAULT_STACK_LIMIT 4096U

typedef enum zlang_storage_t { zlang_storage_automatic_, zlang_storage_static_ } zlang_storage_t;

typedef struct zlang_memory_plan_entry_t {
    const zlang_ast_statement_t *statement_;
    uint32_t size_;
    zlang_storage_t storage_;
} zlang_memory_plan_entry_t;

typedef struct zlang_memory_plan_t {
    zlang_memory_plan_entry_t *entries_;
    size_t capacity_;
    size_t count_;
    uint32_t stack_limit_;
} zlang_memory_plan_t;

void zlang_memory_plan_init(zlang_memory_plan_t *plan, zlang_memory_plan_entry_t *entries,
                            size_t capacity, uint32_t stack_limit);

bool zlang_plan_memory(const zlang_program_t *program, zlang_memory_plan_t *plan);

zlang_storage_t zlang_plan_storage_for(const zlang_memory_plan_t *plan,
                                       const zlang_ast_statement_t *statement);

#endif /* ZLANG_COMPILER_PLANNER_H */
