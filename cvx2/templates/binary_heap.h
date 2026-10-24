/**
 * @file
 * @author Leonardo Vencovsky
 * @brief A priority queue stored as a binary heap in a growable array.
 * @version 0.0.1
 *
 * The element that compares greatest with vtabv->comp is always on top, so
 * this is a max-heap. For a min-heap, give it a comp with the arguments
 * reversed.
 *
 * The array simulates a binary tree. Given an element at index i:
 *
 * ```
 * parent(i) = (i - 1) / 2   (for i > 0)
 *   left(i) = 2i + 1
 *  right(i) = 2i + 2
 * ```
 *
 * Tree structure:
 *
 * ```
 *               [ 95 ]                    <-- top (index 0)
 *              /      \
 *         [ 70 ]      [ 80 ]
 *         /    \      /    \
 *     [ 40 ] [ 50 ] [ 10 ] [ 30 ]
 * ```
 *
 * Array layout (what is actually stored in buffer):
 *
 * ```
 * index:    0      1      2      3      4      5      6
 *         +------+------+------+------+------+------+------+
 * buffer: |  95  |  70  |  80  |  40  |  50  |  10  |  30  |
 *         +------+------+------+------+------+------+------+
 * ```
 *
 * buffer[0] .. buffer[count - 1] can be read directly to visit every element,
 * in no particular order. The heap is not stable: elements comparing equal
 * are not popped in insertion order.
 */

#include "cvx2/fallback.h"

// clang-format off
#ifndef CVX_VAL
#error "binary_heap.h requires CVX_VAL to be defined (the element type)"
#endif
#ifndef CVX_SNAME
#error "binary_heap.h requires CVX_SNAME to be defined (the struct name)"
#endif
#ifndef CVX_PFX
#error "binary_heap.h requires CVX_PFX to be defined (the function prefix)"
#endif
// clang-format on

#include <stdbool.h>
#include <stddef.h>

#include "cvx2/core.h"
#include "cvx2/flags.h"
#include "cvx2/names.h"

#define FUNC(X) CVX_(CVX_PFX, NAME##X)
#define VTAB_V CVX_(CVX_SNAME, NAME##_vtabv)

/**
 * @brief Virtual table of callbacks used to manage CVX_VAL elements.
 *
 * @note comp is required. copy and drop are optional.
 *
 * @note When `copy` is NULL, elements are shallow-copied instead.
 * @note When `drop` is NULL, elements are released without any extra cleanup.
 */
struct VTAB_V
{
    CVX_VTAB_COPY(copy, CVX_VAL); //!< Deep-copies one element
    CVX_VTAB_COMP(comp, CVX_VAL); //!< Priority order; the greatest element is on top
    CVX_VTAB_DROP(drop, CVX_VAL); //!< Releases resources owned by one element
};

/**
 * @brief A max-heap ordered by vtabv->comp.
 *
 * @note count is always <= capacity. buffer is NULL exactly when capacity is 0.
 */
struct CVX_SNAME
{
    size_t capacity;      //!< Current buffer size
    size_t count;         //!< How many elements are stored
    struct VTAB_V *vtabv; //!< Virtual table for the CVX_VAL type; comp is required
    CVX_VAL *buffer;      //!< Elements in heap order; buffer[0] is the top
};

/**
 * @brief Initializes self as an empty heap, optionally pre-allocating capacity elements.
 *
 * @param self a non-NULL pointer to an uninitialized binary heap
 * @param vtabv the vtable used to compare/copy/drop elements; comp is required
 * @param capacity number of elements to pre-allocate; may be 0, in which case
 * the buffer stays unallocated until the first push
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabv or vtabv->comp is not set
 * - CVX_FLAG_ALLOC: if allocation of the initial buffer fails
 **/
enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_V *vtabv, size_t capacity);
/**
 * @brief Initializes self as a heap holding count values, built in O(count).
 *
 * @param self a non-NULL pointer to an uninitialized binary heap
 * @param vtabv the vtable used to compare/copy/drop elements; comp is required
 * @param values a pointer to the first of `count` elements to add; may be NULL
 * when count is 0
 * @param count number of elements to read from `values`; also the initial
 * capacity
 *
 * @note Faster than pushing the values one by one (O(count log count)). The
 * values are shallow-copied in and from then on owned by the heap.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabv or vtabv->comp is not set
 * - CVX_FLAG_ALLOC: if allocation of the buffer fails; the caller still owns
 * the values
 **/
enum cvx_flags FUNC(_heapify)(struct CVX_SNAME *self, struct VTAB_V *vtabv, CVX_VAL *values,
                              size_t count);
/**
 * @brief Frees the buffer, dropping any elements still stored.
 *
 * @param self a pointer to an initialized binary heap; NULL is accepted and ignored
 *
 * @note If vtabv->drop is set, it is called for every stored element before the
 * buffer itself is freed.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self);
/**
 * @brief Initializes clone as a deep copy of orig, with the same capacity.
 *
 * @param orig a non-NULL pointer to an initialized binary heap
 * @param clone a non-NULL pointer to an uninitialized binary heap
 *
 * @note If vtabv->copy is set, it is used to copy each element individually;
 * otherwise the elements are shallow-copied.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation of clone's buffer fails
 * - any flag returned by vtabv->copy, if copying an element fails; clone is
 * left dropped
 **/
enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone);
/**
 * @brief Number of elements currently stored.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 *
 * @return the value of self->count
 **/
size_t FUNC(_count)(struct CVX_SNAME *self);
/**
 * @brief Current buffer capacity.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 *
 * @return the value of self->capacity
 **/
size_t FUNC(_capacity)(struct CVX_SNAME *self);
/**
 * @brief True if count is 0.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 *
 * @return self->count == 0
 **/
bool FUNC(_empty)(struct CVX_SNAME *self);
/**
 * @brief Gets the top (greatest) element without removing it.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 * @param out out parameter for the top element
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the heap is empty
 **/
enum cvx_flags FUNC(_peek)(struct CVX_SNAME *self, CVX_VAL *out);
/**
 * @brief Adds item to the heap, growing the buffer if needed.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 * @param item the value to insert
 *
 * @note O(log n). If the buffer is full, it gets reallocated. The new buffer's
 * size depends on `CVX_BUFFER_GROWTH_RATE` and `CVX_BUFFER_MIN_SIZE`.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if reallocation of the buffer fails
 **/
enum cvx_flags FUNC(_push)(struct CVX_SNAME *self, CVX_VAL item);
/**
 * @brief Removes and returns the top (greatest) element.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 * @param out out parameter for the removed element
 *
 * @note O(log n). If `out` is NULL, the element is dropped (vtabv->drop, if
 * set). Otherwise ownership of it moves to the caller.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the heap is empty
 **/
enum cvx_flags FUNC(_pop)(struct CVX_SNAME *self, CVX_VAL *out);
/**
 * @brief Pushes item, then pops the top element, in a single pass.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 * @param item the value to insert
 * @param out out parameter for the removed element
 *
 * @note O(log n), and faster than `_push` followed by `_pop`. If item compares
 * greater than or equal to the top (or the heap is empty), item itself is
 * returned and the heap is unchanged. Never allocates. If `out` is NULL, the
 * removed element is dropped (vtabv->drop, if set).
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags FUNC(_push_pop)(struct CVX_SNAME *self, CVX_VAL item, CVX_VAL *out);
/**
 * @brief Pops the top element, then pushes item, in a single pass.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 * @param item the value to insert
 * @param out out parameter for the removed element
 *
 * @note O(log n), and faster than `_pop` followed by `_push`. Unlike
 * `_push_pop`, the old top is always removed, even if item is greater. Never
 * allocates. If `out` is NULL, the removed element is dropped (vtabv->drop, if set).
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the heap is empty; item is not inserted and the caller
 * still owns it
 **/
enum cvx_flags FUNC(_replace)(struct CVX_SNAME *self, CVX_VAL item, CVX_VAL *out);
/**
 * @brief Moves every element of other into self, leaving other empty.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 * @param other a non-NULL pointer to an initialized binary heap, other than self
 *
 * @note Elements are moved, not copied, and from then on are managed with
 * self->vtabv. other keeps its buffer. When other has at least as many elements
 * as self, the heap is rebuilt in O(n + m); otherwise each element is sifted up
 * in O(m log(n + m)).
 *
 * Error Handling
 * - CVX_FLAG_INVALID: if other is self
 * - CVX_FLAG_ALLOC: if reallocation of the buffer fails; both heaps are left unchanged
 **/
enum cvx_flags FUNC(_merge)(struct CVX_SNAME *self, struct CVX_SNAME *other);
/**
 * @brief Drops every element, keeping the buffer.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 *
 * @note If vtabv->drop is set, it is called for every stored element. Capacity
 * is not touched; call `_shrink` afterward to reclaim memory.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags FUNC(_clear)(struct CVX_SNAME *self);
/**
 * @brief Grows the buffer to at least capacity, even if it is not yet full.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 * @param capacity desired minimum capacity for buffer
 *
 * @note A no-op if capacity is already <= self->capacity: this only grows; use
 * `_shrink` to reclaim memory instead.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if reallocation of the buffer fails
 **/
enum cvx_flags FUNC(_reserve)(struct CVX_SNAME *self, size_t capacity);
/**
 * @brief Shrinks the buffer's capacity down to count, freeing unused memory.
 *
 * @param self a non-NULL pointer to an initialized binary heap
 *
 * @note A no-op if capacity already equals count. If count is 0, the buffer is
 * freed and set to NULL.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if reallocation of the buffer fails
 **/
enum cvx_flags FUNC(_shrink)(struct CVX_SNAME *self);
