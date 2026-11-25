#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated array helpers */
static inline uint32_t zlang_gen_array_offset_2(uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1) {
    if (index_0 >= extent_0 || index_1 >= extent_1) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }
    return extent_1 * index_0 + index_1;
}

static inline uint32_t zlang_gen_array_offset_3(uint32_t extent_0, uint32_t extent_1, uint32_t extent_2, uint32_t index_0, uint32_t index_1, uint32_t index_2) {
    if (index_0 >= extent_0 || index_1 >= extent_1 || index_2 >= extent_2) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }
    return extent_1 * extent_2 * index_0 + extent_2 * index_1 + index_2;
}

static inline uint32_t zlang_gen_array_offset_4(uint32_t extent_0, uint32_t extent_1, uint32_t extent_2, uint32_t extent_3, uint32_t index_0, uint32_t index_1, uint32_t index_2, uint32_t index_3) {
    if (index_0 >= extent_0 || index_1 >= extent_1 || index_2 >= extent_2 || index_3 >= extent_3) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }
    return extent_1 * extent_2 * extent_3 * index_0 + extent_2 * extent_3 * index_1 + extent_3 * index_2 + index_3;
}

static inline int32_t zlang_gen_array_get_2_i32(const int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1) {
    return data_[zlang_gen_array_offset_2(extent_0, extent_1, index_0, index_1)];
}

static inline int32_t zlang_gen_array_get_3_i32(const int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t extent_2, uint32_t index_0, uint32_t index_1, uint32_t index_2) {
    return data_[zlang_gen_array_offset_3(extent_0, extent_1, extent_2, index_0, index_1, index_2)];
}

static inline int32_t zlang_gen_array_get_4_i32(const int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t extent_2, uint32_t extent_3, uint32_t index_0, uint32_t index_1, uint32_t index_2, uint32_t index_3) {
    return data_[zlang_gen_array_offset_4(extent_0, extent_1, extent_2, extent_3, index_0, index_1, index_2, index_3)];
}

/* generated capacities */
#define ZLANG_SQUARE_CAPACITY 4U
#define ZLANG_SQUARE_LENGTH ZLANG_SQUARE_EXTENT_0
#define ZLANG_SQUARE_EXTENT_0 2U
#define ZLANG_SQUARE_EXTENT_1 2U
#define ZLANG_CUBE_CAPACITY 8U
#define ZLANG_CUBE_LENGTH ZLANG_CUBE_EXTENT_0
#define ZLANG_CUBE_EXTENT_0 2U
#define ZLANG_CUBE_EXTENT_1 2U
#define ZLANG_CUBE_EXTENT_2 2U
#define ZLANG_HYPER_CAPACITY 16U
#define ZLANG_HYPER_LENGTH ZLANG_HYPER_EXTENT_0
#define ZLANG_HYPER_EXTENT_0 2U
#define ZLANG_HYPER_EXTENT_1 2U
#define ZLANG_HYPER_EXTENT_2 2U
#define ZLANG_HYPER_EXTENT_3 2U


/* generated sequence types */
typedef struct app_array_square_t {
    int32_t data_[ZLANG_SQUARE_CAPACITY];
} app_array_square_t;

typedef struct app_array_cube_t {
    int32_t data_[ZLANG_CUBE_CAPACITY];
} app_array_cube_t;

typedef struct app_array_hyper_t {
    int32_t data_[ZLANG_HYPER_CAPACITY];
} app_array_hyper_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

/* generated function prototypes */

int32_t app_fn_main(void);

int32_t app_fn_main(void) {
    app_array_square_t app_local_square = { { 1, 2, 3, 4 } };
    app_array_cube_t app_local_cube = { { 1, 2, 3, 4, 5, 6, 7, 8 } };
    app_array_hyper_t app_local_hyper = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } };
    int32_t app_local_total = 0;

    for (uint32_t app_local_row = ZLANG_ZERO; app_local_row < 2; ++app_local_row) {
        for (uint32_t app_local_col = ZLANG_ZERO; app_local_col < 2; ++app_local_col) {
            app_local_total = zlang_add_i32(app_local_total, zlang_gen_array_get_2_i32(app_local_square.data_, 2, 2, app_local_row, app_local_col));
        }
    }

    for (uint32_t app_local_i = ZLANG_ZERO; app_local_i < 2; ++app_local_i) {
        for (uint32_t app_local_j = ZLANG_ZERO; app_local_j < 2; ++app_local_j) {
            for (uint32_t app_local_k = ZLANG_ZERO; app_local_k < 2; ++app_local_k) {
                app_local_total = zlang_add_i32(app_local_total, zlang_gen_array_get_3_i32(app_local_cube.data_, 2, 2, 2, app_local_i, app_local_j, app_local_k));
            }
        }
    }

    for (uint32_t app_local_a = ZLANG_ZERO; app_local_a < 2; ++app_local_a) {
        for (uint32_t app_local_b = ZLANG_ZERO; app_local_b < 2; ++app_local_b) {
            for (uint32_t app_local_c = ZLANG_ZERO; app_local_c < 2; ++app_local_c) {
                for (uint32_t app_local_d = ZLANG_ZERO; app_local_d < 2; ++app_local_d) {
                    app_local_total = zlang_add_i32(app_local_total, zlang_gen_array_get_4_i32(app_local_hyper.data_, 2, 2, 2, 2, app_local_a, app_local_b, app_local_c, app_local_d));
                }
            }
        }
    }

    zlang_std_io_write_i32(app_local_total);
    zlang_std_io_write_string(zlang_gen_literal_0);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
