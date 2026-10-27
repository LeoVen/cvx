/**
 * @file
 * @author Leonardo Vencovsky
 * @brief A fixed-size array answering range queries and point updates in O(log n).
 * @version 0.0.1
 *
 * A query combines every element of a half-open range [from, to) with
 * `combine`, which must be associative: combine(a, combine(b, c)) equals
 * combine(combine(a, b), c). It doesn't need to be commutative, since elements
 * are always combined left to right. `identity` must leave any element
 * unchanged: combine(identity, x) and combine(x, identity) both equal x.
 *
 * | Query   | combine          | identity  |
 * |---------|------------------|-----------|
 * | sum     | a + b            | 0         |
 * | minimum | a < b ? a : b    | INT_MAX   |
 * | maximum | a > b ? a : b    | INT_MIN   |
 * | gcd     | gcd(a, b)        | 0         |
 *
 * The tree is stored in an array of 2 * leaves nodes, where leaves is count
 * rounded up to a power of two. tree[1] is the root, the children of node i
 * are 2i and 2i + 1, and element i is the leaf tree[leaves + i]. Unused leaves
 * hold identity.
 *
 * ```
 *                 [ 1 ]
 *             /           \
 *         [ 2 ]           [ 3 ]
 *        /     \         /     \
 *     [ 4 ]   [ 5 ]   [ 6 ]   [ 7 ]
 *      / \     / \     / \     / \
 *     8   9  10   11  12  13  14  15     <-- leaves, elements 0 to 7
 * ```
 *
 * Elements are plain values: combine creates new ones for the inner nodes, and
 * the tree never copies or drops them.
 */

#include "cvx2/fallback.h"

// clang-format off
#ifndef CVX_VAL
#error "segment_tree.h requires CVX_VAL to be defined (the element type)"
#endif
#ifndef CVX_SNAME
#error "segment_tree.h requires CVX_SNAME to be defined (the struct name)"
#endif
#ifndef CVX_PFX
#error "segment_tree.h requires CVX_PFX to be defined (the function prefix)"
#endif
// clang-format on

#include <stdbool.h>
#include <stddef.h>

#include "cvx2/core.h"
#include "cvx2/flags.h"
#include "cvx2/names.h"

#define FUNC(X) CVX_(CVX_PFX, NAME##X)

/**
 * @brief A segment tree over count elements.
 *
 * @note leaves is a power of two >= count, and at least 1.
 */
struct CVX_SNAME
{
    size_t count;                       //!< Number of elements
    size_t leaves;                      //!< Number of leaf slots
    CVX_VTAB_COMBINE(combine, CVX_VAL); //!< Associative operation of every query
    CVX_VAL identity;                   //!< Neutral element of combine
    CVX_VAL *tree;                      //!< 2 * leaves nodes; tree[0] is unused
};

/**
 * @brief Initializes self with count elements, all equal to identity.
 *
 * @param self a non-NULL pointer to an uninitialized segment tree
 * @param combine the associative operation every query applies
 * @param identity the neutral element of combine
 * @param count number of elements; may be 0
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if combine is NULL
 * - CVX_FLAG_ALLOC: if allocation of the tree fails, or its size overflows
 **/
enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, CVX_VTAB_COMBINE(combine, CVX_VAL),
                           CVX_VAL identity, size_t count);
/**
 * @brief Initializes self with a copy of count values, built in O(count).
 *
 * @param self a non-NULL pointer to an uninitialized segment tree
 * @param combine the associative operation every query applies
 * @param identity the neutral element of combine
 * @param values a pointer to the first of `count` elements; may be NULL when
 * count is 0
 * @param count number of elements to read from `values`
 *
 * @note Faster than setting the values one by one (O(count log count)).
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if combine is NULL
 * - CVX_FLAG_ALLOC: if allocation of the tree fails, or its size overflows
 **/
enum cvx_flags FUNC(_build)(struct CVX_SNAME *self, CVX_VTAB_COMBINE(combine, CVX_VAL),
                            CVX_VAL identity, CVX_VAL const *values, size_t count);
/**
 * @brief Frees the tree.
 *
 * @param self a pointer to an initialized segment tree; NULL is accepted and ignored
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self);
/**
 * @brief Initializes clone as a copy of orig.
 *
 * @param orig a non-NULL pointer to an initialized segment tree
 * @param clone a non-NULL pointer to an uninitialized segment tree
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation of clone's tree fails
 **/
enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone);
/**
 * @brief Number of elements.
 *
 * @param self a non-NULL pointer to an initialized segment tree
 *
 * @return the value of self->count
 **/
size_t FUNC(_count)(struct CVX_SNAME *self);
/**
 * @brief Gets the element at index.
 *
 * @param self a non-NULL pointer to an initialized segment tree
 * @param index index of the element
 * @param out out parameter for the element
 *
 * @note O(1). If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if index >= count
 **/
enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, size_t index, CVX_VAL *out);
/**
 * @brief Replaces the element at index with value.
 *
 * @param self a non-NULL pointer to an initialized segment tree
 * @param index index of the element
 * @param value the new element
 *
 * @note O(log n). Every node above the element is recombined.
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if index >= count
 **/
enum cvx_flags FUNC(_set)(struct CVX_SNAME *self, size_t index, CVX_VAL value);
/**
 * @brief Combines the elements from index `from` up to, but not including, `to`.
 *
 * @param self a non-NULL pointer to an initialized segment tree
 * @param from index of the first element
 * @param to index one past the last element
 * @param out out parameter for the combined value; identity if from == to
 *
 * @note O(log n).
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if from > to or to > count
 **/
enum cvx_flags FUNC(_query)(struct CVX_SNAME *self, size_t from, size_t to, CVX_VAL *out);
/**
 * @brief Combines every element.
 *
 * @param self a non-NULL pointer to an initialized segment tree
 * @param out out parameter for the combined value; identity if count is 0
 *
 * @note O(1).
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags FUNC(_all)(struct CVX_SNAME *self, CVX_VAL *out);
/**
 * @brief Finds how far right of `from` the combined range still satisfies pred.
 *
 * @param self a non-NULL pointer to an initialized segment tree
 * @param from index of the first element of the range
 * @param pred a predicate that holds for identity, and stops holding at most
 * once as the range grows
 * @param out out parameter for the largest `to` such that pred holds for the
 * query [from, to); count if it holds for every range
 *
 * @note O(log n). For example, with a sum tree of non-negative elements and a
 * pred of `sum <= x`, out is one past the longest range starting at `from`
 * whose sum is at most x.
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if from > count
 * - CVX_FLAG_INVALID: if pred is NULL, or does not hold for identity
 **/
enum cvx_flags FUNC(_max_right)(struct CVX_SNAME *self, size_t from, CVX_VTAB_BOOL(pred, CVX_VAL),
                                size_t *out);
/**
 * @brief Finds how far left of `to` the combined range still satisfies pred.
 *
 * @param self a non-NULL pointer to an initialized segment tree
 * @param to index one past the last element of the range
 * @param pred a predicate that holds for identity, and stops holding at most
 * once as the range grows
 * @param out out parameter for the smallest `from` such that pred holds for the
 * query [from, to); 0 if it holds for every range
 *
 * @note O(log n). The mirror image of `_max_right`.
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if to > count
 * - CVX_FLAG_INVALID: if pred is NULL, or does not hold for identity
 **/
enum cvx_flags FUNC(_min_left)(struct CVX_SNAME *self, size_t to, CVX_VTAB_BOOL(pred, CVX_VAL),
                               size_t *out);
