/** @file
 * @brief Fixed-point transpile-time optimizer for the flat AST.
 */

#include "zlang/compiler/optimizer.h"

#include <limits.h>
#include <string.h>

typedef struct zlang_constant_t {
    bool known_;
    bool unsigned_;
    bool boolean_;
    int32_t signed_value_;
    uint32_t unsigned_value_;
} zlang_constant_t;

void zlang_optimizer_context_init(zlang_optimizer_context_t *context) {
    memset(context, 0, sizeof(*context));
}

static void zlang_optimizer_queue_all(const zlang_program_t *program,
                                      zlang_optimizer_context_t *context) {
    context->worklist_count_ = 0U;
    context->worklist_front_ = 0U;
    memset(context->queued_, 0, sizeof(context->queued_));
    for (size_t _index = 0U; _index < program->expression_count_; ++_index) {
        if (_index >= ZLANG_TTO_WORKLIST_CAPACITY) {
            break;
        }
        context->worklist_[context->worklist_count_++] = _index;
        context->queued_[_index] = true;
    }
}

static const zlang_ast_statement_t *zlang_tto_array_decl(const zlang_program_t *program,
                                                         const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_array_declaration_ &&
            _statement_k->name_ != NULL && strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static bool zlang_tto_is_written(const zlang_program_t *program, const char *name) {
    for (size_t _index = 0U; _index < program->expression_count_; ++_index) {
        const zlang_ast_expression_t *const _expression_k = &program->expressions_[_index];
        if ((_expression_k->kind_ == zlang_expression_index_assignment_ ||
             _expression_k->kind_ == zlang_expression_direct_index_assignment_) &&
            _expression_k->name_ != NULL && name != NULL &&
            strcmp(_expression_k->name_, name) == 0) {
            return true;
        }
    }
    return false;
}

static bool zlang_tto_is_assignment_target(const zlang_program_t *program, size_t index) {
    for (size_t _owner = 0U; _owner < program->expression_count_; ++_owner) {
        const zlang_ast_expression_t *const _owner_k = &program->expressions_[_owner];
        if (_owner_k->kind_ == zlang_expression_index_assignment_ && _owner_k->left_ == index) {
            return true;
        }
    }
    return false;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_tto_eval(const zlang_program_t *program, size_t index, zlang_constant_t *result,
                           uint32_t depth) {
    if (result == NULL || depth > 16U || index >= program->expression_count_) {
        return false;
    }
    memset(result, 0, sizeof(*result));
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    switch (_expression_k->kind_) {
    case zlang_expression_integer_literal_:
        result->known_ = true;
        result->unsigned_ = _expression_k->is_unsigned_;
        result->signed_value_ = _expression_k->integer_value_;
        result->unsigned_value_ = (uint32_t)_expression_k->integer_value_;
        return true;
    case zlang_expression_boolean_literal_:
        result->known_ = true;
        result->boolean_ = true;
        result->signed_value_ = _expression_k->bool_value_ ? 1 : 0;
        result->unsigned_value_ = _expression_k->bool_value_ ? 1U : 0U;
        return true;
    case zlang_expression_identifier_: {
        for (size_t _statement = 0U; _statement < program->statement_count_; ++_statement) {
            const zlang_ast_statement_t *const _statement_k = &program->statements_[_statement];
            if (_statement_k->kind_ != zlang_statement_declaration_ || !_statement_k->is_const_ ||
                _statement_k->name_ == NULL || _expression_k->name_ == NULL ||
                strcmp(_statement_k->name_, _expression_k->name_) != 0) {
                continue;
            }
            zlang_constant_t _value;
            if (zlang_tto_eval(program, _statement_k->expression_, &_value, depth + 1U) &&
                _value.known_ && !_value.boolean_) {
                *result = _value;
                return true;
            }
            return false;
        }
        return false;
    }
    case zlang_expression_length_: {
        const zlang_ast_statement_t *const _array_k =
            zlang_tto_array_decl(program, _expression_k->name_);
        if (_array_k == NULL) {
            return false;
        }
        result->known_ = true;
        result->unsigned_ = true;
        result->unsigned_value_ =
            _array_k->rank_ > 0U ? _array_k->extents_[0] : _array_k->capacity_;
        return true;
    }
    case zlang_expression_negative_: {
        zlang_constant_t _operand;
        if (!zlang_tto_eval(program, _expression_k->left_, &_operand, depth + 1U) ||
            !_operand.known_ || _operand.unsigned_ || _operand.boolean_) {
            return false;
        }
        if (_operand.signed_value_ == INT32_MIN) {
            return false;
        }
        result->known_ = true;
        result->signed_value_ = -_operand.signed_value_;
        return true;
    }
    case zlang_expression_add_:
    case zlang_expression_sub_:
    case zlang_expression_mul_:
    case zlang_expression_div_:
    case zlang_expression_mod_: {
        zlang_constant_t _left;
        zlang_constant_t _right;
        if (!zlang_tto_eval(program, _expression_k->left_, &_left, depth + 1U) ||
            !zlang_tto_eval(program, _expression_k->right_, &_right, depth + 1U) || !_left.known_ ||
            !_right.known_ || _left.boolean_ || _right.boolean_ ||
            _left.unsigned_ != _right.unsigned_) {
            return false;
        }
        result->known_ = true;
        result->unsigned_ = _left.unsigned_;
        if (_left.unsigned_) {
            uint64_t _value_k = 0ULL;
            switch (_expression_k->kind_) {
            case zlang_expression_add_:
                _value_k = (uint64_t)_left.unsigned_value_ + _right.unsigned_value_;
                break;
            case zlang_expression_sub_:
                if (_right.unsigned_value_ > _left.unsigned_value_) {
                    return false;
                }
                _value_k = (uint64_t)_left.unsigned_value_ - _right.unsigned_value_;
                break;
            case zlang_expression_mul_:
                _value_k = (uint64_t)_left.unsigned_value_ * _right.unsigned_value_;
                break;
            case zlang_expression_div_:
                if (_right.unsigned_value_ == 0U) {
                    return false;
                }
                _value_k = _left.unsigned_value_ / _right.unsigned_value_;
                break;
            case zlang_expression_mod_:
                if (_right.unsigned_value_ == 0U) {
                    return false;
                }
                _value_k = _left.unsigned_value_ % _right.unsigned_value_;
                break;
            default:
                return false;
            }
            if (_value_k > UINT32_MAX) {
                return false;
            }
            result->unsigned_value_ = (uint32_t)_value_k;
            return true;
        }
        int64_t _value_k = 0LL;
        switch (_expression_k->kind_) {
        case zlang_expression_add_:
            _value_k = (int64_t)_left.signed_value_ + _right.signed_value_;
            break;
        case zlang_expression_sub_:
            _value_k = (int64_t)_left.signed_value_ - _right.signed_value_;
            break;
        case zlang_expression_mul_:
            _value_k = (int64_t)_left.signed_value_ * _right.signed_value_;
            break;
        case zlang_expression_div_:
            if (_right.signed_value_ == 0 || _right.signed_value_ == -1) {
                return false;
            }
            _value_k = _left.signed_value_ / _right.signed_value_;
            break;
        case zlang_expression_mod_:
            if (_right.signed_value_ == 0 || _right.signed_value_ == -1) {
                return false;
            }
            _value_k = _left.signed_value_ % _right.signed_value_;
            break;
        default:
            return false;
        }
        if (_value_k < INT32_MIN || _value_k > INT32_MAX) {
            return false;
        }
        result->signed_value_ = (int32_t)_value_k;
        return true;
    }
    case zlang_expression_equal_:
    case zlang_expression_not_equal_:
    case zlang_expression_less_:
    case zlang_expression_greater_:
    case zlang_expression_less_equal_:
    case zlang_expression_greater_equal_: {
        zlang_constant_t _left;
        zlang_constant_t _right;
        if (!zlang_tto_eval(program, _expression_k->left_, &_left, depth + 1U) ||
            !zlang_tto_eval(program, _expression_k->right_, &_right, depth + 1U) || !_left.known_ ||
            !_right.known_ || _left.boolean_ != _right.boolean_ ||
            _left.unsigned_ != _right.unsigned_) {
            return false;
        }
        bool _value_k = false;
        if (_left.boolean_) {
            const int32_t _left_value_k = _left.signed_value_;
            const int32_t _right_value_k = _right.signed_value_;
            switch (_expression_k->kind_) {
            case zlang_expression_equal_:
                _value_k = _left_value_k == _right_value_k;
                break;
            case zlang_expression_not_equal_:
                _value_k = _left_value_k != _right_value_k;
                break;
            default:
                return false;
            }
        } else if (_left.unsigned_) {
            const uint32_t _left_value_k = _left.unsigned_value_;
            const uint32_t _right_value_k = _right.unsigned_value_;
            switch (_expression_k->kind_) {
            case zlang_expression_equal_:
                _value_k = _left_value_k == _right_value_k;
                break;
            case zlang_expression_not_equal_:
                _value_k = _left_value_k != _right_value_k;
                break;
            case zlang_expression_less_:
                _value_k = _left_value_k < _right_value_k;
                break;
            case zlang_expression_greater_:
                _value_k = _left_value_k > _right_value_k;
                break;
            case zlang_expression_less_equal_:
                _value_k = _left_value_k <= _right_value_k;
                break;
            case zlang_expression_greater_equal_:
                _value_k = _left_value_k >= _right_value_k;
                break;
            default:
                return false;
            }
        } else {
            const int32_t _left_value_k = _left.signed_value_;
            const int32_t _right_value_k = _right.signed_value_;
            switch (_expression_k->kind_) {
            case zlang_expression_equal_:
                _value_k = _left_value_k == _right_value_k;
                break;
            case zlang_expression_not_equal_:
                _value_k = _left_value_k != _right_value_k;
                break;
            case zlang_expression_less_:
                _value_k = _left_value_k < _right_value_k;
                break;
            case zlang_expression_greater_:
                _value_k = _left_value_k > _right_value_k;
                break;
            case zlang_expression_less_equal_:
                _value_k = _left_value_k <= _right_value_k;
                break;
            case zlang_expression_greater_equal_:
                _value_k = _left_value_k >= _right_value_k;
                break;
            default:
                return false;
            }
        }
        result->known_ = true;
        result->boolean_ = true;
        result->signed_value_ = _value_k ? 1 : 0;
        result->unsigned_value_ = _value_k ? 1U : 0U;
        return true;
    }
    case zlang_expression_not_: {
        zlang_constant_t _operand;
        if (!zlang_tto_eval(program, _expression_k->left_, &_operand, depth + 1U) ||
            !_operand.known_ || !_operand.boolean_) {
            return false;
        }
        result->known_ = true;
        result->boolean_ = true;
        result->signed_value_ = _operand.signed_value_ == 0 ? 1 : 0;
        result->unsigned_value_ = _operand.unsigned_value_ == 0U ? 1U : 0U;
        return true;
    }
    case zlang_expression_and_:
    case zlang_expression_or_: {
        zlang_constant_t _left;
        zlang_constant_t _right;
        if (!zlang_tto_eval(program, _expression_k->left_, &_left, depth + 1U) ||
            !zlang_tto_eval(program, _expression_k->right_, &_right, depth + 1U) || !_left.known_ ||
            !_right.known_ || !_left.boolean_ || !_right.boolean_) {
            return false;
        }
        const bool _value_k = _expression_k->kind_ == zlang_expression_and_
                                  ? (_left.signed_value_ != 0 && _right.signed_value_ != 0)
                                  : (_left.signed_value_ != 0 || _right.signed_value_ != 0);
        result->known_ = true;
        result->boolean_ = true;
        result->signed_value_ = _value_k ? 1 : 0;
        result->unsigned_value_ = _value_k ? 1U : 0U;
        return true;
    }
    case zlang_expression_index_: {
        if (_expression_k->argument_count_ != 1U ||
            zlang_tto_is_assignment_target(program, index)) {
            return false;
        }
        const zlang_ast_statement_t *const _array_k =
            zlang_tto_array_decl(program, _expression_k->name_);
        if (_array_k == NULL || _array_k->initializer_count_ == 0U ||
            zlang_tto_is_written(program, _expression_k->name_)) {
            return false;
        }
        zlang_constant_t _index;
        if (!zlang_tto_eval(program, _expression_k->right_, &_index, depth + 1U) ||
            !_index.known_ || _index.boolean_ || _index.signed_value_ < 0) {
            return false;
        }
        const uint32_t _extent_k =
            _array_k->rank_ > 0U ? _array_k->extents_[0] : _array_k->capacity_;
        if ((uint32_t)_index.signed_value_ >= _extent_k) {
            return false;
        }
        const size_t _item_k = _array_k->initializer_start_ + (size_t)_index.signed_value_;
        if ((size_t)_index.signed_value_ < _array_k->initializer_count_ &&
            zlang_tto_eval(program, _item_k, result, depth + 1U)) {
            return result->known_;
        }
        if (_array_k->initializer_count_ == 0U) {
            return false;
        }
        result->known_ = true;
        result->unsigned_ = strcmp(_array_k->type_name_, "unsigned") == 0;
        return true;
    }
    default:
        return false;
    }
}

static bool zlang_tto_store_literal(zlang_ast_expression_t *expression,
                                    const zlang_constant_t *value) {
    if (expression == NULL || value == NULL || !value->known_) {
        return false;
    }
    const char *const _name_k = expression->name_;
    const uint32_t _line_k = expression->line_;
    const uint32_t _column_k = expression->column_;
    memset(expression, 0, sizeof(*expression));
    expression->left_ = ZLANG_NO_EXPRESSION;
    expression->right_ = ZLANG_NO_EXPRESSION;
    expression->name_ = _name_k;
    expression->line_ = _line_k;
    expression->column_ = _column_k;
    expression->kind_ =
        value->boolean_ ? zlang_expression_boolean_literal_ : zlang_expression_integer_literal_;
    expression->bool_value_ = value->boolean_ && value->signed_value_ != 0;
    expression->is_unsigned_ = value->unsigned_;
    expression->integer_value_ = value->signed_value_;
    return true;
}

typedef struct zlang_loop_range_t {
    const char *variable_name_;
    int64_t min_;
    int64_t max_;
    bool inclusive_;
} zlang_loop_range_t;

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_tto_contains_expression(const zlang_program_t *program, size_t root,
                                          size_t index) {
    if (root == ZLANG_NO_EXPRESSION || root >= program->expression_count_) {
        return false;
    }
    if (root == index) {
        return true;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[root];
    if (zlang_tto_contains_expression(program, _expression_k->left_, index) ||
        zlang_tto_contains_expression(program, _expression_k->right_, index)) {
        return true;
    }
    for (size_t _argument = 0U;
         _argument < _expression_k->argument_count_ && _argument < ZLANG_CALL_ARGUMENT_CAPACITY;
         ++_argument) {
        if (zlang_tto_contains_expression(program, _expression_k->argument_indices_[_argument],
                                          index)) {
            return true;
        }
    }
    return false;
}

static bool zlang_tto_statement_owns_expression(const zlang_program_t *program, size_t statement,
                                                size_t expression) {
    if (statement >= program->statement_count_) {
        return false;
    }
    const zlang_ast_statement_t *const _statement_k = &program->statements_[statement];
    if (zlang_tto_contains_expression(program, _statement_k->expression_, expression) ||
        zlang_tto_contains_expression(program, _statement_k->update_expression_, expression)) {
        return true;
    }
    const size_t _end_k = _statement_k->initializer_start_ + _statement_k->initializer_count_;
    for (size_t _item = _statement_k->initializer_start_;
         _item < _end_k && _item < program->expression_count_; ++_item) {
        if (_item == expression) {
            return true;
        }
    }
    return false;
}

static bool zlang_tto_expression_in_body(const zlang_program_t *program, size_t expression,
                                         const zlang_ast_statement_t *loop) {
    const size_t _end_k = loop->body_start_ + loop->body_count_;
    for (size_t _statement = loop->body_start_;
         _statement < _end_k && _statement < program->statement_count_; ++_statement) {
        if (zlang_tto_statement_owns_expression(program, _statement, expression)) {
            return true;
        }
    }
    return false;
}

static bool zlang_tto_eval_signed(const zlang_program_t *program, size_t index, int64_t *value) {
    zlang_constant_t _constant;
    if (!zlang_tto_eval(program, index, &_constant, 0U) || !_constant.known_ ||
        _constant.boolean_ || _constant.unsigned_) {
        return false;
    }
    *value = (int64_t)_constant.signed_value_;
    return true;
}

static bool zlang_tto_same_identifier(const zlang_program_t *program, size_t index,
                                      const char *name) {
    if (index >= program->expression_count_ || name == NULL) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    return _expression_k->kind_ == zlang_expression_identifier_ && _expression_k->name_ != NULL &&
           strcmp(_expression_k->name_, name) == 0;
}

static bool zlang_tto_loop_range(const zlang_program_t *program, const zlang_ast_statement_t *loop,
                                 zlang_loop_range_t *range) {
    if (loop->kind_ != zlang_statement_for_ ||
        loop->initializer_statement_ >= program->statement_count_) {
        return false;
    }
    const zlang_ast_statement_t *const _initializer_k =
        &program->statements_[loop->initializer_statement_];
    if (_initializer_k->kind_ != zlang_statement_declaration_ || _initializer_k->is_const_ ||
        _initializer_k->type_name_ == NULL || strcmp(_initializer_k->type_name_, "integer") != 0 ||
        _initializer_k->name_ == NULL ||
        _initializer_k->expression_ >= program->expression_count_) {
        return false;
    }
    int64_t _min_k = 0;
    if (!zlang_tto_eval_signed(program, _initializer_k->expression_, &_min_k)) {
        return false;
    }
    const zlang_ast_expression_t *const _condition_k =
        loop->expression_ < program->expression_count_ ? &program->expressions_[loop->expression_]
                                                       : NULL;
    if (_condition_k == NULL ||
        !zlang_tto_same_identifier(program, _condition_k->left_, _initializer_k->name_)) {
        return false;
    }
    int64_t _bound_k = 0;
    if (!zlang_tto_eval_signed(program, _condition_k->right_, &_bound_k)) {
        return false;
    }
    const bool _inclusive_k = _condition_k->kind_ == zlang_expression_less_equal_;
    if (_condition_k->kind_ != zlang_expression_less_ && !_inclusive_k) {
        return false;
    }
    const zlang_ast_expression_t *const _update_k =
        loop->update_expression_ < program->expression_count_
            ? &program->expressions_[loop->update_expression_]
            : NULL;
    if (_update_k == NULL || _update_k->kind_ != zlang_expression_compound_assignment_ ||
        _update_k->operator_ != zlang_expression_add_ ||
        !zlang_tto_same_identifier(program, _update_k->left_, _initializer_k->name_)) {
        return false;
    }
    int64_t _step_k = 0;
    if (!zlang_tto_eval_signed(program, _update_k->right_, &_step_k) || _step_k != 1) {
        return false;
    }
    range->variable_name_ = _initializer_k->name_;
    range->min_ = _min_k;
    range->max_ = _inclusive_k ? _bound_k : _bound_k - 1;
    range->inclusive_ = _inclusive_k;
    return true;
}

static bool zlang_tto_variable_mutated(const zlang_program_t *program,
                                       const zlang_ast_statement_t *loop, const char *name) {
    const size_t _end_k = loop->body_start_ + loop->body_count_;
    for (size_t _statement = loop->body_start_;
         _statement < _end_k && _statement < program->statement_count_; ++_statement) {
        for (size_t _expression = 0U; _expression < program->expression_count_; ++_expression) {
            if (!zlang_tto_statement_owns_expression(program, _statement, _expression)) {
                continue;
            }
            const zlang_ast_expression_t *const _expression_k = &program->expressions_[_expression];
            const bool _assignment_k =
                _expression_k->kind_ == zlang_expression_assignment_ ||
                _expression_k->kind_ == zlang_expression_compound_assignment_;
            if (_assignment_k && zlang_tto_same_identifier(program, _expression_k->left_, name)) {
                return true;
            }
        }
    }
    return false;
}

static bool zlang_tto_index_offset(const zlang_program_t *program, size_t expression,
                                   const zlang_loop_range_t *range, int64_t *min, int64_t *max) {
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[expression];
    if (zlang_tto_same_identifier(program, expression, range->variable_name_)) {
        *min = range->min_;
        *max = range->max_;
        return true;
    }
    int64_t _offset_k = 0;
    if (_expression_k->kind_ == zlang_expression_sub_ &&
        zlang_tto_same_identifier(program, _expression_k->left_, range->variable_name_) &&
        zlang_tto_eval_signed(program, _expression_k->right_, &_offset_k)) {
        *min = range->min_ - _offset_k;
        *max = range->max_ - _offset_k;
        return true;
    }
    if (_expression_k->kind_ == zlang_expression_add_ &&
        zlang_tto_same_identifier(program, _expression_k->left_, range->variable_name_) &&
        zlang_tto_eval_signed(program, _expression_k->right_, &_offset_k)) {
        *min = range->min_ + _offset_k;
        *max = range->max_ + _offset_k;
        return true;
    }
    return false;
}

static bool zlang_tto_mark_proven_ranges(zlang_program_t *program) {
    for (size_t _statement = 0U; _statement < program->statement_count_; ++_statement) {
        zlang_loop_range_t _range;
        if (!zlang_tto_loop_range(program, &program->statements_[_statement], &_range) ||
            zlang_tto_variable_mutated(program, &program->statements_[_statement],
                                       _range.variable_name_)) {
            continue;
        }
        const zlang_ast_statement_t *const _loop_k = &program->statements_[_statement];
        for (size_t _expression = 0U; _expression < program->expression_count_; ++_expression) {
            zlang_ast_expression_t *const _expression_k = &program->expressions_[_expression];
            zlang_ast_expression_t *_target_k = NULL;
            if (_expression_k->kind_ == zlang_expression_index_ ||
                _expression_k->kind_ == zlang_expression_index_assignment_) {
                if (_expression_k->kind_ == zlang_expression_index_) {
                    _target_k = _expression_k;
                } else if (_expression_k->left_ < program->expression_count_) {
                    _target_k = &program->expressions_[_expression_k->left_];
                }
            }
            if (_target_k == NULL || _target_k->kind_ != zlang_expression_index_ ||
                _target_k->argument_count_ != 1U ||
                !zlang_tto_expression_in_body(program, _expression, _loop_k)) {
                continue;
            }
            const zlang_ast_statement_t *const _array_k =
                zlang_tto_array_decl(program, _target_k->name_);
            if (_array_k == NULL || _array_k->rank_ != 1U || _array_k->type_name_ == NULL ||
                strcmp(_array_k->type_name_, "string") == 0) {
                continue;
            }
            int64_t _min_k = 0;
            int64_t _max_k = 0;
            if (!zlang_tto_index_offset(program, _target_k->argument_indices_[0], &_range, &_min_k,
                                        &_max_k) ||
                _min_k < 0 || _max_k < _min_k || _max_k >= (int64_t)_array_k->extents_[0]) {
                continue;
            }
            _target_k->is_proven_ = true;
        }
    }
    return true;
}

static bool zlang_tto_fold_index_assignment(zlang_program_t *program, size_t index) {
    zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ != zlang_expression_index_assignment_ ||
        _expression_k->left_ >= program->expression_count_) {
        return false;
    }
    const zlang_ast_expression_t *const _target_k = &program->expressions_[_expression_k->left_];
    if (_target_k->kind_ != zlang_expression_index_ || _target_k->argument_count_ != 1U) {
        return false;
    }
    zlang_constant_t _index;
    if (!zlang_tto_eval(program, _target_k->argument_indices_[0], &_index, 0U) || !_index.known_ ||
        _index.boolean_ || _index.signed_value_ < 0) {
        return false;
    }
    const zlang_ast_statement_t *const _array_k = zlang_tto_array_decl(program, _target_k->name_);
    if (_array_k == NULL || (_array_k->rank_ != 0U && _array_k->rank_ != 1U)) {
        return false;
    }
    const uint32_t _extent_k = _array_k->rank_ > 0U ? _array_k->extents_[0] : _array_k->capacity_;
    if ((uint32_t)_index.signed_value_ >= _extent_k) {
        return false;
    }
    const size_t _value_k = _expression_k->right_;
    const char *const _name_k = _target_k->name_;
    const uint32_t _line_k = _expression_k->line_;
    const uint32_t _column_k = _expression_k->column_;
    memset(_expression_k, 0, sizeof(*_expression_k));
    _expression_k->left_ = ZLANG_NO_EXPRESSION;
    _expression_k->right_ = _value_k;
    _expression_k->kind_ = zlang_expression_direct_index_assignment_;
    _expression_k->name_ = _name_k;
    _expression_k->argument_count_ = 1U;
    _expression_k->argument_indices_[0] = (size_t)_index.signed_value_;
    _expression_k->line_ = _line_k;
    _expression_k->column_ = _column_k;
    return true;
}

bool zlang_optimize_program(zlang_program_t *program, zlang_optimizer_context_t *context) {
    if (program == NULL || context == NULL) {
        return false;
    }
    if (!zlang_tto_mark_proven_ranges(program)) {
        return false;
    }
    zlang_optimizer_queue_all(program, context);
    while (context->worklist_front_ < context->worklist_count_ &&
           context->round_count_ < ZLANG_TTO_MAX_ROUNDS) {
        const size_t _index_k = context->worklist_[context->worklist_front_++];
        if (_index_k >= program->expression_count_) {
            return false;
        }
        bool _changed_k = zlang_tto_fold_index_assignment(program, _index_k);
        if (!_changed_k) {
            const zlang_expression_kind_t _kind_k = program->expressions_[_index_k].kind_;
            const bool _literal_k = _kind_k == zlang_expression_integer_literal_ ||
                                    _kind_k == zlang_expression_boolean_literal_;
            zlang_constant_t _value;
            if (!_literal_k && zlang_tto_eval(program, _index_k, &_value, 0U) &&
                zlang_tto_store_literal(&program->expressions_[_index_k], &_value)) {
                _changed_k = true;
            }
        }
        if (_changed_k) {
            ++context->generation_;
            if (!zlang_tto_mark_proven_ranges(program)) {
                return false;
            }
            zlang_optimizer_queue_all(program, context);
        }
    }
    ++context->round_count_;
    return true;
}
