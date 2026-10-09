/**
 * @file dynamic_array.h
 * @brief A dynamically-resizing array.
 *
 * Required config: value_type, struct_name, prefix.
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
#include "cvx2/names.h"

#define FUNC(X) CVX_(CVX_PFX, NAME##X)
#define VTAB_V CVX_(CVX_SNAME, NAME##_vtabv)

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

/** @brief Initializes self, optionally pre-allocating capacity elements. */
enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_V *vtabv, size_t capacity);
/** @brief Frees the buffer, dropping any elements still stored. */
enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self);
/** @brief Initializes clone as a deep copy of orig. */
enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone);

/** @brief Number of elements currently stored. */
size_t FUNC(_count)(struct CVX_SNAME *self);
/** @brief Current buffer capacity. */
size_t FUNC(_capacity)(struct CVX_SNAME *self);
/** @brief True if count is 0. */
bool FUNC(_empty)(struct CVX_SNAME *self);
/** @brief True if count equals capacity. */
bool FUNC(_full)(struct CVX_SNAME *self);

/** @brief Gets the first element. Fails with CVX_FLAG_EMPTY if empty. */
enum cvx_flags FUNC(_front)(struct CVX_SNAME *self, CVX_VAL *out);
/** @brief Gets the last element. Fails with CVX_FLAG_EMPTY if empty. */
enum cvx_flags FUNC(_back)(struct CVX_SNAME *self, CVX_VAL *out);
/** @brief Gets the element at index. Fails with CVX_FLAG_RANGE if out of bounds. */
enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, size_t index, CVX_VAL *out);

/** @brief Inserts item at the front, growing the buffer if needed. */
enum cvx_flags FUNC(_push_front)(struct CVX_SNAME *self, CVX_VAL item);
/** @brief Inserts item at index, growing the buffer if needed. */
enum cvx_flags FUNC(_push_at)(struct CVX_SNAME *self, CVX_VAL item, size_t index);
/** @brief Inserts item at the back, growing the buffer if needed. */
enum cvx_flags FUNC(_push_back)(struct CVX_SNAME *self, CVX_VAL item);

/** @brief Removes and returns the first element. Fails with CVX_FLAG_EMPTY if empty. */
enum cvx_flags FUNC(_pop_front)(struct CVX_SNAME *self, CVX_VAL *out);
/** @brief Removes and returns the element at index. Fails with CVX_FLAG_RANGE if out of bounds. */
enum cvx_flags FUNC(_pop_at)(struct CVX_SNAME *self, size_t index, CVX_VAL *out);
/** @brief Removes and returns the last element. Fails with CVX_FLAG_EMPTY if empty. */
enum cvx_flags FUNC(_pop_back)(struct CVX_SNAME *self, CVX_VAL *out);
/** @brief Replaces the first element, returning the old value through old_out. */
enum cvx_flags FUNC(_replace_front)(struct CVX_SNAME *self, CVX_VAL new_value, CVX_VAL *old_out);
/** @brief Replaces the last element, returning the old value through old_out. */
enum cvx_flags FUNC(_replace_back)(struct CVX_SNAME *self, CVX_VAL new_value, CVX_VAL *old_out);
/** @brief Swaps the elements at idx1 and idx2. */
enum cvx_flags FUNC(_swap)(struct CVX_SNAME *self, size_t idx1, size_t idx2);

/** @brief Lexicographically compares two arrays using vtabv->comp. */
enum cvx_flags FUNC(_compare)(struct CVX_SNAME *left, struct CVX_SNAME *right, int *out);
/** @brief Sorts in place using vtabv->comp. */
enum cvx_flags FUNC(_sort)(struct CVX_SNAME *self);
