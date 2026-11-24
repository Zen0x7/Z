/** @file
 * @brief Compile-time storage placement for Zlang declarations.
 */

#include "zlang/compiler/planner.h"

#include <string.h>

void zlang_memory_plan_init(zlang_memory_plan_t *plan, zlang_memory_plan_entry_t *entries,
                            size_t capacity, uint32_t stack_limit) {
    plan->entries_ = entries;
    plan->capacity_ = capacity;
    plan->count_ = 0U;
    plan->stack_limit_ = stack_limit;
}

static uint32_t zlang_element_size(const char *type_name) {
    if (strcmp(type_name, "integer") == 0) {
        return 4U;
    }
    if (strcmp(type_name, "unsigned") == 0) {
        return 4U;
    }
    if (strcmp(type_name, "boolean") == 0) {
        return 1U;
    }
    if (strcmp(type_name, "string") == 0) {
        return (uint32_t)(sizeof(void *) + sizeof(uint32_t));
    }
    return 0U;
}

static bool zlang_is_declaration(const zlang_ast_statement_t *statement) {
    return statement->kind_ == zlang_statement_declaration_ ||
           statement->kind_ == zlang_statement_sequence_declaration_;
}

static bool zlang_declaration_size(const zlang_ast_statement_t *statement, uint32_t *size) {
    const uint32_t _element_size_k = zlang_element_size(statement->type_name_);
    if (_element_size_k == 0U) {
        return false;
    }

    if (statement->kind_ == zlang_statement_declaration_) {
        if (strcmp(statement->type_name_, "string") == 0 && statement->has_explicit_capacity_) {
            if (statement->capacity_ > UINT32_MAX - _element_size_k - 1U) {
                return false;
            }
            *size = _element_size_k + statement->capacity_ + 1U;
            return true;
        }
        *size = _element_size_k;
        return true;
    }

    if (statement->capacity_ == 0U || statement->capacity_ > UINT32_MAX / _element_size_k) {
        return false;
    }
    *size = _element_size_k * statement->capacity_;
    return true;
}

bool zlang_plan_memory(const zlang_program_t *program, zlang_memory_plan_t *plan) {
    plan->count_ = 0U;

    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (!zlang_is_declaration(_statement_k)) {
            continue;
        }
        if (plan->count_ >= plan->capacity_) {
            return false;
        }

        zlang_memory_plan_entry_t *const _entry_k = &plan->entries_[plan->count_];
        if (!zlang_declaration_size(_statement_k, &_entry_k->size_)) {
            return false;
        }
        _entry_k->statement_ = _statement_k;
        _entry_k->storage_ =
            _entry_k->size_ > plan->stack_limit_ ? zlang_storage_static_ : zlang_storage_automatic_;
        ++plan->count_;
    }
    return true;
}

zlang_storage_t zlang_plan_storage_for(const zlang_memory_plan_t *plan,
                                       const zlang_ast_statement_t *statement) {
    for (size_t _index = 0U; _index < plan->count_; ++_index) {
        if (plan->entries_[_index].statement_ == statement) {
            return plan->entries_[_index].storage_;
        }
    }
    return zlang_storage_automatic_;
}
