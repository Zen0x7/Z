/** @file
 * @brief Static standard library registry.
 */

#include "zlang/compiler/stdlib.h"

#include <string.h>

static const zlang_std_function_t functions_k[] = {{"random",
                                                    "generate_int32",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_random_generate_int32",
                                                    "ZLANG_STD_RANDOM_GENERATE_INT32",
                                                    2U,
                                                    {"int32", "int32"},
                                                    true,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"random",
                                                    "generate_uint32",
                                                    "result_type<uint32,status_code>",
                                                    "zlang_std_random_generate_uint32",
                                                    "ZLANG_STD_RANDOM_GENERATE_UINT32",
                                                    2U,
                                                    {"uint32", "uint32"},
                                                    true,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "uint32",
                                                    "status_code"},
                                                   {"random",
                                                    "generate_int64",
                                                    "result_type<int64,status_code>",
                                                    "zlang_std_random_generate_int64",
                                                    "ZLANG_STD_RANDOM_GENERATE_INT64",
                                                    2U,
                                                    {"int64", "int64"},
                                                    true,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int64",
                                                    "status_code"},
                                                   {"random",
                                                    "generate_uint64",
                                                    "result_type<uint64,status_code>",
                                                    "zlang_std_random_generate_uint64",
                                                    "ZLANG_STD_RANDOM_GENERATE_UINT64",
                                                    2U,
                                                    {"uint64", "uint64"},
                                                    true,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "uint64",
                                                    "status_code"},
                                                   {"random",
                                                    "generate_float32",
                                                    "result_type<float,status_code>",
                                                    "zlang_std_random_generate_float32",
                                                    "ZLANG_STD_RANDOM_GENERATE_FLOAT32",
                                                    2U,
                                                    {"float", "float"},
                                                    true,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "float",
                                                    "status_code"},
                                                   {"random",
                                                    "generate_float64",
                                                    "result_type<double,status_code>",
                                                    "zlang_std_random_generate_float64",
                                                    "ZLANG_STD_RANDOM_GENERATE_FLOAT64",
                                                    2U,
                                                    {"double", "double"},
                                                    true,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "double",
                                                    "status_code"},
                                                   {"io",
                                                    "read",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_read",
                                                    "ZLANG_STD_IO_READ",
                                                    1U,
                                                    {"string"},
                                                    false,
                                                    true,
                                                    {true},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "read_line",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_read_line",
                                                    "ZLANG_STD_IO_READ_LINE",
                                                    1U,
                                                    {"string"},
                                                    false,
                                                    true,
                                                    {true},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_string",
                                                    "ZLANG_STD_IO_WRITE_STRING",
                                                    1U,
                                                    {"string"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {true, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_bool",
                                                    "ZLANG_STD_IO_WRITE_BOOL",
                                                    1U,
                                                    {"boolean"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_i32",
                                                    "ZLANG_STD_IO_WRITE_I32",
                                                    1U,
                                                    {"int32"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_u32",
                                                    "ZLANG_STD_IO_WRITE_U32",
                                                    1U,
                                                    {"uint32"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_i64",
                                                    "ZLANG_STD_IO_WRITE_I64",
                                                    1U,
                                                    {"int64"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_u64",
                                                    "ZLANG_STD_IO_WRITE_U64",
                                                    1U,
                                                    {"uint64"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_f32",
                                                    "ZLANG_STD_IO_WRITE_F32",
                                                    1U,
                                                    {"float"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_f64",
                                                    "ZLANG_STD_IO_WRITE_F64",
                                                    1U,
                                                    {"double"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "read",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_read_stream",
                                                    "ZLANG_STD_IO_READ",
                                                    4U,
                                                    {"stream", "string", "uint32", "uint32"},
                                                    false,
                                                    true,
                                                    {false, true, false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "read_line",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_read_line_stream",
                                                    "ZLANG_STD_IO_READ_LINE",
                                                    4U,
                                                    {"stream", "string", "uint32", "uint32"},
                                                    false,
                                                    true,
                                                    {false, true, false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_string_stream",
                                                    "ZLANG_STD_IO_WRITE_STRING",
                                                    2U,
                                                    {"stream", "string"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_bool_stream",
                                                    "ZLANG_STD_IO_WRITE_BOOL",
                                                    2U,
                                                    {"stream", "boolean"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_i32_stream",
                                                    "ZLANG_STD_IO_WRITE_I32",
                                                    2U,
                                                    {"stream", "int32"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_u32_stream",
                                                    "ZLANG_STD_IO_WRITE_U32",
                                                    2U,
                                                    {"stream", "uint32"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_i64_stream",
                                                    "ZLANG_STD_IO_WRITE_I64",
                                                    2U,
                                                    {"stream", "int64"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_u64_stream",
                                                    "ZLANG_STD_IO_WRITE_U64",
                                                    2U,
                                                    {"stream", "uint64"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_f32_stream",
                                                    "ZLANG_STD_IO_WRITE_F32",
                                                    2U,
                                                    {"stream", "float"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_f64_stream",
                                                    "ZLANG_STD_IO_WRITE_F64",
                                                    2U,
                                                    {"stream", "double"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"io",
                                                    "write",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_io_write_string_window",
                                                    "ZLANG_STD_IO_WRITE_STRING",
                                                    4U,
                                                    {"stream", "string", "uint32", "uint32"},
                                                    false,
                                                    true,
                                                    {false, false, false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"math",
                                                    "abs",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_math_abs_i32",
                                                    "ZLANG_STD_MATH_ABS_I32",
                                                    1U,
                                                    {"int32"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"math",
                                                    "clamp",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_math_clamp_i32",
                                                    "ZLANG_STD_MATH_CLAMP_I32",
                                                    3U,
                                                    {"int32", "int32", "int32"},
                                                    false,
                                                    true,
                                                    {false, false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"math",
                                                    "min",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_math_min_i32",
                                                    "ZLANG_STD_MATH_MIN_I32",
                                                    2U,
                                                    {"int32", "int32"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"math",
                                                    "max",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_math_max_i32",
                                                    "ZLANG_STD_MATH_MAX_I32",
                                                    2U,
                                                    {"int32", "int32"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"math",
                                                    "abs",
                                                    "result_type<uint32,status_code>",
                                                    "zlang_std_math_abs_u32",
                                                    "ZLANG_STD_MATH_ABS_U32",
                                                    1U,
                                                    {"uint32"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "uint32",
                                                    "status_code"},
                                                   {"math",
                                                    "clamp",
                                                    "result_type<uint32,status_code>",
                                                    "zlang_std_math_clamp_u32",
                                                    "ZLANG_STD_MATH_CLAMP_U32",
                                                    3U,
                                                    {"uint32", "uint32", "uint32"},
                                                    false,
                                                    true,
                                                    {false, false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "uint32",
                                                    "status_code"},
                                                   {"math",
                                                    "min",
                                                    "result_type<uint32,status_code>",
                                                    "zlang_std_math_min_u32",
                                                    "ZLANG_STD_MATH_MIN_U32",
                                                    2U,
                                                    {"uint32", "uint32"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "uint32",
                                                    "status_code"},
                                                   {"math",
                                                    "max",
                                                    "result_type<uint32,status_code>",
                                                    "zlang_std_math_max_u32",
                                                    "ZLANG_STD_MATH_MAX_U32",
                                                    2U,
                                                    {"uint32", "uint32"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "uint32",
                                                    "status_code"},
                                                   {"math",
                                                    "abs",
                                                    "result_type<int64,status_code>",
                                                    "zlang_std_math_abs_i64",
                                                    "ZLANG_STD_MATH_ABS_I64",
                                                    1U,
                                                    {"int64"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int64",
                                                    "status_code"},
                                                   {"math",
                                                    "clamp",
                                                    "result_type<int64,status_code>",
                                                    "zlang_std_math_clamp_i64",
                                                    "ZLANG_STD_MATH_CLAMP_I64",
                                                    3U,
                                                    {"int64", "int64", "int64"},
                                                    false,
                                                    true,
                                                    {false, false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int64",
                                                    "status_code"},
                                                   {"math",
                                                    "min",
                                                    "result_type<int64,status_code>",
                                                    "zlang_std_math_min_i64",
                                                    "ZLANG_STD_MATH_MIN_I64",
                                                    2U,
                                                    {"int64", "int64"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int64",
                                                    "status_code"},
                                                   {"math",
                                                    "max",
                                                    "result_type<int64,status_code>",
                                                    "zlang_std_math_max_i64",
                                                    "ZLANG_STD_MATH_MAX_I64",
                                                    2U,
                                                    {"int64", "int64"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int64",
                                                    "status_code"},
                                                   {"math",
                                                    "abs",
                                                    "result_type<uint64,status_code>",
                                                    "zlang_std_math_abs_u64",
                                                    "ZLANG_STD_MATH_ABS_U64",
                                                    1U,
                                                    {"uint64"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "uint64",
                                                    "status_code"},
                                                   {"math",
                                                    "clamp",
                                                    "result_type<uint64,status_code>",
                                                    "zlang_std_math_clamp_u64",
                                                    "ZLANG_STD_MATH_CLAMP_U64",
                                                    3U,
                                                    {"uint64", "uint64", "uint64"},
                                                    false,
                                                    true,
                                                    {false, false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "uint64",
                                                    "status_code"},
                                                   {"math",
                                                    "min",
                                                    "result_type<uint64,status_code>",
                                                    "zlang_std_math_min_u64",
                                                    "ZLANG_STD_MATH_MIN_U64",
                                                    2U,
                                                    {"uint64", "uint64"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "uint64",
                                                    "status_code"},
                                                   {"math",
                                                    "max",
                                                    "result_type<uint64,status_code>",
                                                    "zlang_std_math_max_u64",
                                                    "ZLANG_STD_MATH_MAX_U64",
                                                    2U,
                                                    {"uint64", "uint64"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "uint64",
                                                    "status_code"},
                                                   {"math",
                                                    "abs",
                                                    "result_type<float,status_code>",
                                                    "zlang_std_math_abs_f32",
                                                    "ZLANG_STD_MATH_ABS_F32",
                                                    1U,
                                                    {"float"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "float",
                                                    "status_code"},
                                                   {"math",
                                                    "clamp",
                                                    "result_type<float,status_code>",
                                                    "zlang_std_math_clamp_f32",
                                                    "ZLANG_STD_MATH_CLAMP_F32",
                                                    3U,
                                                    {"float", "float", "float"},
                                                    false,
                                                    true,
                                                    {false, false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "float",
                                                    "status_code"},
                                                   {"math",
                                                    "min",
                                                    "result_type<float,status_code>",
                                                    "zlang_std_math_min_f32",
                                                    "ZLANG_STD_MATH_MIN_F32",
                                                    2U,
                                                    {"float", "float"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "float",
                                                    "status_code"},
                                                   {"math",
                                                    "max",
                                                    "result_type<float,status_code>",
                                                    "zlang_std_math_max_f32",
                                                    "ZLANG_STD_MATH_MAX_F32",
                                                    2U,
                                                    {"float", "float"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "float",
                                                    "status_code"},
                                                   {"math",
                                                    "abs",
                                                    "result_type<double,status_code>",
                                                    "zlang_std_math_abs_f64",
                                                    "ZLANG_STD_MATH_ABS_F64",
                                                    1U,
                                                    {"double"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "double",
                                                    "status_code"},
                                                   {"math",
                                                    "clamp",
                                                    "result_type<double,status_code>",
                                                    "zlang_std_math_clamp_f64",
                                                    "ZLANG_STD_MATH_CLAMP_F64",
                                                    3U,
                                                    {"double", "double", "double"},
                                                    false,
                                                    true,
                                                    {false, false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "double",
                                                    "status_code"},
                                                   {"math",
                                                    "min",
                                                    "result_type<double,status_code>",
                                                    "zlang_std_math_min_f64",
                                                    "ZLANG_STD_MATH_MIN_F64",
                                                    2U,
                                                    {"double", "double"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "double",
                                                    "status_code"},
                                                   {"math",
                                                    "max",
                                                    "result_type<double,status_code>",
                                                    "zlang_std_math_max_f64",
                                                    "ZLANG_STD_MATH_MAX_F64",
                                                    2U,
                                                    {"double", "double"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "double",
                                                    "status_code"},
                                                   {"math",
                                                    "sqrt",
                                                    "result_type<float,status_code>",
                                                    "zlang_std_math_sqrt_f32",
                                                    "ZLANG_STD_MATH_SQRT_F32",
                                                    1U,
                                                    {"float"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "float",
                                                    "status_code"},
                                                   {"math",
                                                    "sqrt",
                                                    "result_type<double,status_code>",
                                                    "zlang_std_math_sqrt_f64",
                                                    "ZLANG_STD_MATH_SQRT_F64",
                                                    1U,
                                                    {"double"},
                                                    false,
                                                    true,
                                                    {false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "double",
                                                    "status_code"},
                                                   {"strings",
                                                    "compare",
                                                    "result_type<int32,status_code>",
                                                    "zlang_std_strings_compare",
                                                    "ZLANG_STD_STRINGS_COMPARE",
                                                    2U,
                                                    {"string", "string"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "int32",
                                                    "status_code"},
                                                   {"strings",
                                                    "contains",
                                                    "result_type<boolean,status_code>",
                                                    "zlang_std_strings_contains",
                                                    "ZLANG_STD_STRINGS_CONTAINS",
                                                    2U,
                                                    {"string", "string"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "boolean",
                                                    "status_code"},
                                                   {"strings",
                                                    "starts_with",
                                                    "result_type<boolean,status_code>",
                                                    "zlang_std_strings_starts_with",
                                                    "ZLANG_STD_STRINGS_STARTS_WITH",
                                                    2U,
                                                    {"string", "string"},
                                                    false,
                                                    true,
                                                    {false, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "boolean",
                                                    "status_code"},
                                                   {"program_options",
                                                    "argument_count",
                                                    "result_type<uint32,status_code>",
                                                    "zlang_std_program_options_argument_count",
                                                    "ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT",
                                                    1U,
                                                    {"arguments"},
                                                    false,
                                                    true,
                                                    {true},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "uint32",
                                                    "status_code"},
                                                   {"program_options",
                                                    "argument_at",
                                                    "result_type<string,status_code>",
                                                    "zlang_std_program_options_argument_at",
                                                    "ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_AT",
                                                    2U,
                                                    {"arguments", "uint32"},
                                                    false,
                                                    true,
                                                    {true, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "string",
                                                    "status_code"},
                                                   {"program_options",
                                                    "has_option",
                                                    "result_type<boolean,status_code>",
                                                    "zlang_std_program_options_has_option",
                                                    "ZLANG_STD_PROGRAM_OPTIONS_HAS_OPTION",
                                                    2U,
                                                    {"arguments", "string"},
                                                    false,
                                                    true,
                                                    {true, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "boolean",
                                                    "status_code"},
                                                   {"program_options",
                                                    "option_value",
                                                    "result_type<string,status_code>",
                                                    "zlang_std_program_options_option_value",
                                                    "ZLANG_STD_PROGRAM_OPTIONS_OPTION_VALUE",
                                                    2U,
                                                    {"arguments", "string"},
                                                    false,
                                                    true,
                                                    {true, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "string",
                                                    "status_code"},
                                                   {"program_options",
                                                    "has_flag",
                                                    "result_type<boolean,status_code>",
                                                    "zlang_std_program_options_has_flag",
                                                    "ZLANG_STD_PROGRAM_OPTIONS_HAS_FLAG",
                                                    2U,
                                                    {"arguments", "string"},
                                                    false,
                                                    true,
                                                    {true, false},
                                                    {false, false, false, false},
                                                    zlang_std_return_result_,
                                                    "boolean",
                                                    "status_code"}};

size_t zlang_std_function_count(void) { return sizeof(functions_k) / sizeof(functions_k[0]); }

const zlang_std_function_t *zlang_std_function_at(size_t index) {
    return index < zlang_std_function_count() ? &functions_k[index] : NULL;
}

const zlang_std_function_t *zlang_std_find(const char *domain, const char *name) {
    if (domain == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < sizeof(functions_k) / sizeof(functions_k[0]); ++_index) {
        const zlang_std_function_t *const _entry_k = &functions_k[_index];
        if (strcmp(_entry_k->domain_, domain) == 0 && strcmp(_entry_k->name_, name) == 0) {
            return _entry_k;
        }
    }
    return NULL;
}

const zlang_std_function_t *zlang_std_find_by_name(const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < sizeof(functions_k) / sizeof(functions_k[0]); ++_index) {
        if (strcmp(functions_k[_index].name_, name) == 0) {
            return &functions_k[_index];
        }
    }
    return NULL;
}

const zlang_std_function_t *zlang_std_resolve(const char *domain, const char *name,
                                              const char *const *argument_types, size_t count) {
    if (domain == NULL || name == NULL || argument_types == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < sizeof(functions_k) / sizeof(functions_k[0]); ++_index) {
        const zlang_std_function_t *const _entry_k = &functions_k[_index];
        if (strcmp(_entry_k->domain_, domain) != 0 || strcmp(_entry_k->name_, name) != 0 ||
            _entry_k->parameter_count_ != count) {
            continue;
        }
        bool _matches_k = true;
        for (size_t _argument = 0U; _argument < count; ++_argument) {
            if (argument_types[_argument] == NULL ||
                strcmp(_entry_k->parameter_types_[_argument], argument_types[_argument]) != 0) {
                _matches_k = false;
                break;
            }
        }
        if (_matches_k) {
            return _entry_k;
        }
    }
    return NULL;
}

const zlang_std_function_t *
zlang_std_resolve_by_name(const char *name, const char *const *argument_types, size_t count) {
    if (name == NULL || argument_types == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < sizeof(functions_k) / sizeof(functions_k[0]); ++_index) {
        const zlang_std_function_t *const _entry_k = &functions_k[_index];
        if (strcmp(_entry_k->name_, name) != 0) {
            continue;
        }
        bool _matches_k = _entry_k->parameter_count_ == count;
        for (size_t _argument = 0U; _matches_k && _argument < count; ++_argument) {
            if (argument_types[_argument] == NULL ||
                strcmp(_entry_k->parameter_types_[_argument], argument_types[_argument]) != 0) {
                _matches_k = false;
            }
        }
        if (_matches_k) {
            return _entry_k;
        }
    }
    return NULL;
}

static const zlang_std_result_t results_k[] = {
    {"result_type<void,status_code>", "void", "status_code", "zlang_std_result_void_status_t",
     "zlang_std_result_void"},
    {"result_type<int32,status_code>", "int32", "status_code", "zlang_std_result_i32_status_t",
     "zlang_std_result_i32"},
    {"result_type<uint32,status_code>", "uint32", "status_code", "zlang_std_result_u32_status_t",
     "zlang_std_result_u32"},
    {"result_type<int64,status_code>", "int64", "status_code", "zlang_std_result_i64_status_t",
     "zlang_std_result_i64"},
    {"result_type<uint64,status_code>", "uint64", "status_code", "zlang_std_result_u64_status_t",
     "zlang_std_result_u64"},
    {"result_type<boolean,status_code>", "boolean", "status_code", "zlang_std_result_bool_status_t",
     "zlang_std_result_bool"},
    {"result_type<float,status_code>", "float", "status_code", "zlang_std_result_f32_status_t",
     "zlang_std_result_f32"},
    {"result_type<double,status_code>", "double", "status_code", "zlang_std_result_f64_status_t",
     "zlang_std_result_f64"},
    {"result_type<string,status_code>", "string", "status_code", "zlang_std_result_string_status_t",
     "zlang_std_result_string"}};

size_t zlang_std_result_count(void) { return sizeof(results_k) / sizeof(results_k[0]); }

const zlang_std_result_t *zlang_std_result_at(size_t index) {
    return index < zlang_std_result_count() ? &results_k[index] : NULL;
}

const zlang_std_result_t *zlang_std_find_result(const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < sizeof(results_k) / sizeof(results_k[0]); ++_index) {
        if (strcmp(results_k[_index].name_, name) == 0) {
            return &results_k[_index];
        }
    }
    return NULL;
}

const zlang_std_result_t *zlang_std_find_result_by_value(const char *value_type) {
    if (value_type == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < sizeof(results_k) / sizeof(results_k[0]); ++_index) {
        if (strcmp(results_k[_index].value_type_, value_type) == 0) {
            return &results_k[_index];
        }
    }
    return NULL;
}

const char *zlang_std_native_c_type(const char *name) {
    const zlang_std_result_t *const _result_k = zlang_std_find_result(name);
    if (_result_k != NULL) {
        return _result_k->c_type_;
    }
    if (name != NULL && strcmp(name, "status_code") == 0) {
        return "zlang_std_status_t";
    }
    if (name != NULL && strcmp(name, "stream") == 0) {
        return "zlang_stream_t";
    }
    return NULL;
}

const char *zlang_std_native_result_field_type(const zlang_std_result_t *result,
                                               const char *field_name) {
    if (result == NULL || field_name == NULL) {
        return NULL;
    }
    if (strcmp(field_name, "code") == 0) {
        return "status_code";
    }
    if (strcmp(field_name, "what") == 0) {
        return "string";
    }
    if (strcmp(field_name, "value") == 0 && strcmp(result->value_type_, "void") != 0) {
        return result->value_type_;
    }
    return NULL;
}

bool zlang_std_is_native_type(const char *name) { return zlang_std_native_c_type(name) != NULL; }

bool zlang_std_is_native_generic_base(const char *name) {
    return name != NULL && strcmp(name, "result_type") == 0;
}

static const char *const status_constants_k[] = {"STATUS_SUCCESS", "STATUS_END_OF_INPUT",
                                                 "STATUS_IO_ERROR", "STATUS_INVALID_ARGUMENT",
                                                 "STATUS_CAPACITY"};

static const char *const status_constant_c_names_k[] = {
    "zlang_std_status_success_", "zlang_std_status_end_of_input_", "zlang_std_status_io_error_",
    "zlang_std_status_invalid_argument_", "zlang_std_status_capacity_"};

const char *zlang_std_native_result_tag(const zlang_std_result_t *result) {
    if (result == NULL) {
        return NULL;
    }
    if (strcmp(result->value_type_, "void") == 0) {
        return "void";
    }
    if (strcmp(result->value_type_, "int32") == 0) {
        return "i32";
    }
    if (strcmp(result->value_type_, "uint32") == 0) {
        return "u32";
    }
    if (strcmp(result->value_type_, "int64") == 0) {
        return "i64";
    }
    if (strcmp(result->value_type_, "uint64") == 0) {
        return "u64";
    }
    if (strcmp(result->value_type_, "boolean") == 0) {
        return "bool";
    }
    if (strcmp(result->value_type_, "float") == 0) {
        return "f32";
    }
    if (strcmp(result->value_type_, "double") == 0) {
        return "f64";
    }
    if (strcmp(result->value_type_, "string") == 0) {
        return "string";
    }
    return NULL;
}

const char *zlang_std_status_constant_c_name(const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < sizeof(status_constants_k) / sizeof(status_constants_k[0]);
         ++_index) {
        if (strcmp(status_constants_k[_index], name) == 0) {
            return status_constant_c_names_k[_index];
        }
    }
    return NULL;
}

bool zlang_std_is_status_constant(const char *name) {
    return zlang_std_status_constant_c_name(name) != NULL;
}

static const char *const stream_constants_k[] = {"STDIN", "STDOUT", "STDERR"};

static const char *const stream_constant_c_names_k[] = {
    "zlang_std_io_stream_stdin()", "zlang_std_io_stream_stdout()", "zlang_std_io_stream_stderr()"};

const char *zlang_std_stream_constant_c_name(const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < sizeof(stream_constants_k) / sizeof(stream_constants_k[0]);
         ++_index) {
        if (strcmp(stream_constants_k[_index], name) == 0) {
            return stream_constant_c_names_k[_index];
        }
    }
    return NULL;
}

bool zlang_std_is_stream_constant(const char *name) {
    return zlang_std_stream_constant_c_name(name) != NULL;
}

bool zlang_std_is_native_result_method(const zlang_std_result_t *result, const char *method_name) {
    if (result == NULL || method_name == NULL) {
        return false;
    }
    return strcmp(method_name, "is_ok") == 0 || strcmp(method_name, "get") == 0 ||
           strcmp(method_name, "message") == 0 || strcmp(method_name, "time") == 0 ||
           strcmp(method_name, "dump") == 0;
}

const char *zlang_std_native_result_method_type(const zlang_std_result_t *result,
                                                const char *method_name) {
    if (!zlang_std_is_native_result_method(result, method_name)) {
        return NULL;
    }
    if (strcmp(method_name, "is_ok") == 0) {
        return "boolean";
    }
    if (strcmp(method_name, "message") == 0 || strcmp(method_name, "dump") == 0) {
        return "string";
    }
    if (strcmp(method_name, "time") == 0) {
        return "uint64";
    }
    if (strcmp(result->value_type_, "void") == 0) {
        return NULL;
    }
    return result->value_type_;
}

bool zlang_std_is_known_generic_argument(const char *name) {
    return name != NULL && (strcmp(name, "void") == 0 || strcmp(name, "boolean") == 0 ||
                            strcmp(name, "int32") == 0 || strcmp(name, "uint32") == 0 ||
                            strcmp(name, "int64") == 0 || strcmp(name, "uint64") == 0 ||
                            strcmp(name, "float") == 0 || strcmp(name, "double") == 0 ||
                            strcmp(name, "string") == 0 || zlang_std_is_native_type(name));
}
