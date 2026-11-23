#include "zlang/compiler/diagnostics.h"

void zlang_diagnostics_init(zlang_diagnostics_t *diagnostics, zlang_diagnostic_t *items,
                            size_t capacity) {
    diagnostics->items_ = items;
    diagnostics->capacity_ = capacity;
    diagnostics->count_ = 0U;
}

bool zlang_diagnostics_add(zlang_diagnostics_t *diagnostics, uint16_t code, uint32_t line,
                           uint32_t column, const char *message) {
    if (diagnostics->count_ >= diagnostics->capacity_) {
        return false;
    }

    zlang_diagnostic_t *const _item_k = &diagnostics->items_[diagnostics->count_];
    _item_k->code_ = code;
    _item_k->line_ = line;
    _item_k->column_ = column;
    _item_k->message_ = message;
    ++diagnostics->count_;
    return true;
}

bool zlang_diagnostics_contains(const zlang_diagnostics_t *diagnostics, uint16_t code) {
    for (size_t _index = 0U; _index < diagnostics->count_; ++_index) {
        if (diagnostics->items_[_index].code_ == code) {
            return true;
        }
    }
    return false;
}
