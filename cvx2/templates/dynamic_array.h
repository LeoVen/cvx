/**
 * @file dynamic_array.h
 * @brief cvx template: an array that gets reallocated as needed.
 *
 * ## Required config fields
 *
 * - **value_type**: element type stored in the array (maps to CVX_VAL)
 * - **struct_name**: name of the generated struct (maps to CVX_SNAME)
 * - **prefix**: prefix of all generated functions (maps to CVX_PFX)
 *
 */

#include "cvx2/fallback.h"

// clang-format off
#ifndef CVX_VAL
#error "dynamic_array.h requires CVX_VAL to be defined (the element type)"
#endif
#ifndef CVX_SNAME
#error "dynamic_array.h requires CVX_SNAME to be defined (the struct name)"
#endif
#ifndef CVX_PFX
#error "dynamic_array.h requires CVX_PFX to be defined (the function prefix)"
#endif
// clang-format on

#include <stdbool.h>
#include <stddef.h>

#include "cvx2/core.h"
#include "cvx2/flags.h"

#define FUNC(X) CVX_(CVX_PFX, X)
#define VTAB_V CVX_(CVX_SNAME, _vtabv)

struct VTAB_V
{
    CVX_VTAB_COPY(copy, CVX_VAL);
    CVX_VTAB_COMP(comp, CVX_VAL);
    CVX_VTAB_DROP(drop, CVX_VAL);
};

struct CVX_SNAME
{
    size_t capacity;
    size_t count;
    struct VTAB_V *vtabv;
    CVX_VAL *buffer;
};

// Initializers and destructors
enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_V *vtabv, size_t capacity);
enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self);
enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone);
// Getters (no failure mode beyond "empty" struct state, so these keep direct returns)
size_t FUNC(_count)(struct CVX_SNAME *self);
size_t FUNC(_capacity)(struct CVX_SNAME *self);
bool FUNC(_empty)(struct CVX_SNAME *self);
bool FUNC(_full)(struct CVX_SNAME *self);
// Operations
/**
 * @brief Gets the element at position 0
 *
 * **Error Handling**
 * - `CVX_FLAG_EMPTY` - if the array is empty
 */
enum cvx_flags FUNC(_front)(struct CVX_SNAME *self, CVX_VAL *out);
enum cvx_flags FUNC(_back)(struct CVX_SNAME *self, CVX_VAL *out);
enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, size_t index, CVX_VAL *out);
/**
 * @brief Inserts an element at position 0.
 * @note If the buffer is full, it gets reallocated. The new buffer's size
 * depends on `CVX_BUFFER_GROWTH_RATE` and `CVX_BUFFER_MIN_SIZE`.
 *
 * **Error Handling**
 * - `CVX_FLAG_ALLOC` - if reallocation of the buffer fails
 */
enum cvx_flags FUNC(_push_front)(struct CVX_SNAME *self, CVX_VAL item);
/**
 * @brief Inserts `item` at position `index`.
 * @param index must be 0 <= index <= count
 *
 * **Error Handling**
 * - `CVX_FLAG_RANGE` - if index > count
 * - `CVX_FLAG_ALLOC` - if reallocation of the buffer fails
 */
enum cvx_flags FUNC(_push_at)(struct CVX_SNAME *self, CVX_VAL item, size_t index);
/**
 * @brief Inserts an element at the last position.
 *
 * **Error Handling**
 * - `CVX_FLAG_ALLOC` - if reallocation of the buffer fails
 */
enum cvx_flags FUNC(_push_back)(struct CVX_SNAME *self, CVX_VAL item);
/**
 * @brief Removes and returns (through `out`) the item at position 0.
 * @note `_pop*` functions do not cause the buffer to shrink.
 *
 * **Error Handling**
 * - `CVX_FLAG_EMPTY` - if there are no items in the dynamic array.
 */
enum cvx_flags FUNC(_pop_front)(struct CVX_SNAME *self, CVX_VAL *out);
/**
 * @brief
 */
enum cvx_flags FUNC(_pop_at)(struct CVX_SNAME *self, size_t index, CVX_VAL *out);
/**
 * @brief
 */
enum cvx_flags FUNC(_pop_back)(struct CVX_SNAME *self, CVX_VAL *out);
/**
 * @brief
 */
enum cvx_flags FUNC(_replace_front)(struct CVX_SNAME *self, CVX_VAL new_value, CVX_VAL *old_out);
/**
 * @brief
 */
enum cvx_flags FUNC(_replace_back)(struct CVX_SNAME *self, CVX_VAL new_value, CVX_VAL *old_out);
/**
 * @brief
 */
enum cvx_flags FUNC(_swap)(struct CVX_SNAME *self, size_t idx1, size_t idx2);
// Extras
enum cvx_flags FUNC(_compare)(struct CVX_SNAME *left, struct CVX_SNAME *right, int *out);
enum cvx_flags FUNC(_sort)(struct CVX_SNAME *self);
