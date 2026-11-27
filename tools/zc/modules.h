/** @file
 * @brief Module graph resolution and single-translation-unit assembly.
 */

#ifndef ZLANG_TOOLS_ZC_MODULES_H
#define ZLANG_TOOLS_ZC_MODULES_H

#include "zlang/compiler/diagnostics.h"
#include "zlang/compiler/lexer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ZLANG_MODULE_MAX 16U
#define ZLANG_MODULE_PATH_CAPACITY 256U
#define ZLANG_MODULE_ID_CAPACITY 128U
#define ZLANG_MODULE_SOURCE_CAPACITY 32768U
#define ZLANG_MODULE_TOKEN_CAPACITY 1024U
#define ZLANG_MODULE_COMMENT_CAPACITY 128U
#define ZLANG_MODULE_NAME_CAPACITY 128U
#define ZLANG_MODULE_DECLARATION_CAPACITY 64U
#define ZLANG_ALIAS_MAX 16U
#define ZLANG_NATIVE_MAX 16U
#define ZLANG_NATIVE_PATH_MAX 16U

typedef struct zlang_native_t {
    char alias_[ZLANG_MODULE_NAME_CAPACITY];
    char header_[ZLANG_MODULE_PATH_CAPACITY];
    char include_dirs_[ZLANG_NATIVE_PATH_MAX][ZLANG_MODULE_PATH_CAPACITY];
    size_t include_dir_count_;
    char sources_[ZLANG_NATIVE_PATH_MAX][ZLANG_MODULE_PATH_CAPACITY];
    size_t source_count_;
    char libraries_[ZLANG_NATIVE_PATH_MAX][ZLANG_MODULE_NAME_CAPACITY];
    size_t library_count_;
    char link_flags_[ZLANG_NATIVE_PATH_MAX][ZLANG_MODULE_PATH_CAPACITY];
    size_t link_flag_count_;
    bool used_;
} zlang_native_t;

typedef struct zlang_module_t {
    char path_[ZLANG_MODULE_PATH_CAPACITY];
    char id_[ZLANG_MODULE_ID_CAPACITY];
    char source_[ZLANG_MODULE_SOURCE_CAPACITY];
    size_t source_length_;
    char scratch_[ZLANG_MODULE_SOURCE_CAPACITY];
    zlang_token_t tokens_[ZLANG_MODULE_TOKEN_CAPACITY];
    size_t token_count_;
    char import_names_[ZLANG_MODULE_DECLARATION_CAPACITY][ZLANG_MODULE_NAME_CAPACITY];
    char import_domains_[ZLANG_MODULE_DECLARATION_CAPACITY][ZLANG_MODULE_NAME_CAPACITY];
    size_t import_count_;
    char declaration_names_[ZLANG_MODULE_DECLARATION_CAPACITY][ZLANG_MODULE_NAME_CAPACITY];
    char canonical_names_[ZLANG_MODULE_DECLARATION_CAPACITY][ZLANG_MODULE_NAME_CAPACITY];
    bool declaration_is_enum_[ZLANG_MODULE_DECLARATION_CAPACITY];
    char decl_name_buffer_[ZLANG_MODULE_NAME_CAPACITY];
    size_t declaration_count_;
    char assembled_[ZLANG_MODULE_SOURCE_CAPACITY];
    bool loaded_;
    bool visiting_;
} zlang_module_t;

typedef struct zlang_modules_t {
    char root_[ZLANG_MODULE_PATH_CAPACITY];
    char alias_names_[ZLANG_ALIAS_MAX][ZLANG_MODULE_NAME_CAPACITY];
    char alias_paths_[ZLANG_ALIAS_MAX][ZLANG_MODULE_PATH_CAPACITY];
    size_t alias_count_;
    zlang_module_t modules_[ZLANG_MODULE_MAX];
    size_t module_count_;
    char std_import_names_[ZLANG_MODULE_DECLARATION_CAPACITY][ZLANG_MODULE_NAME_CAPACITY];
    char std_domains_[ZLANG_MODULE_DECLARATION_CAPACITY][ZLANG_MODULE_NAME_CAPACITY];
    size_t std_import_count_;
    zlang_native_t natives_[ZLANG_NATIVE_MAX];
    size_t native_count_;
} zlang_modules_t;

void zlang_modules_init(zlang_modules_t *modules);

bool zlang_modules_assemble(zlang_modules_t *modules, const char *source_path,
                            zlang_diagnostics_t *diagnostics, char *output, size_t capacity);

size_t zlang_modules_native_count(const zlang_modules_t *modules);
const zlang_native_t *zlang_modules_native_at(const zlang_modules_t *modules, size_t index);
bool zlang_modules_native_include_dir(const zlang_modules_t *modules, const zlang_native_t *native,
                                      size_t index, char *output, size_t capacity);
bool zlang_modules_native_source(const zlang_modules_t *modules, const zlang_native_t *native,
                                 size_t index, char *output, size_t capacity);
bool zlang_modules_native_library(const zlang_native_t *native, size_t index, char *output,
                                  size_t capacity);
bool zlang_modules_native_link_flag(const zlang_native_t *native, size_t index, char *output,
                                    size_t capacity);

#endif
