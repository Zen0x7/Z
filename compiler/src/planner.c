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

static const zlang_ast_statement_t *zlang_find_record(const zlang_program_t *program,
                                                      const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_record_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static const zlang_ast_statement_t *zlang_find_enum(const zlang_program_t *program,
                                                    const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_enum_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_element_size(const zlang_program_t *program, const char *type_name,
                               uint32_t depth,
                               uint32_t *size) { // NOLINT(misc-no-recursion)
    if (type_name == NULL || size == NULL || depth > 8U) {
        return false;
    }
    if (strcmp(type_name, "integer") == 0 || strcmp(type_name, "unsigned") == 0) {
        *size = 4U;
        return true;
    }
    if (strcmp(type_name, "boolean") == 0) {
        *size = 1U;
        return true;
    }
    if (strcmp(type_name, "string") == 0) {
        *size = (uint32_t)(sizeof(void *) + sizeof(uint32_t));
        return true;
    }
    if (zlang_find_enum(program, type_name) != NULL) {
        *size = 4U;
        return true;
    }
    const zlang_ast_statement_t *const _record_k = zlang_find_record(program, type_name);
    if (_record_k == NULL) {
        return false;
    }
    uint32_t _total_k = 0U;
    const size_t _end_k = _record_k->body_start_ + _record_k->body_count_;
    for (size_t _index = _record_k->body_start_;
         _index < _end_k && _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _field_k = &program->statements_[_index];
        uint32_t _field_size_k = 0U;
        if (!zlang_element_size(program, _field_k->type_name_, depth + 1U, &_field_size_k) ||
            _total_k > UINT32_MAX - _field_size_k) {
            return false;
        }
        _total_k += _field_size_k;
    }
    *size = _total_k;
    return true;
}

static bool zlang_is_declaration(const zlang_ast_statement_t *statement) {
    return statement->kind_ == zlang_statement_declaration_ ||
           statement->kind_ == zlang_statement_array_declaration_;
}

static bool zlang_declaration_size(const zlang_program_t *program,
                                   const zlang_ast_statement_t *statement, uint32_t *size) {
    uint32_t _element_size_k = 0U;
    if (!zlang_element_size(program, statement->type_name_, 0U, &_element_size_k) ||
        _element_size_k == 0U) {
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

    if (statement->capacity_ == 0U) {
        return false;
    }
    if (statement->has_element_capacity_) {
        const uint32_t _view_size_k = (uint32_t)(sizeof(void *) + sizeof(uint32_t));
        if (statement->element_capacity_ == UINT32_MAX ||
            statement->element_capacity_ > UINT32_MAX - _view_size_k - 1U) {
            return false;
        }
        const uint32_t _element_k = statement->element_capacity_ + 1U + _view_size_k;
        if (statement->capacity_ > UINT32_MAX / _element_k) {
            return false;
        }
        *size = _element_k * statement->capacity_;
        return true;
    }
    if (statement->capacity_ > UINT32_MAX / _element_size_k) {
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
        if (!zlang_declaration_size(program, _statement_k, &_entry_k->size_)) {
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
