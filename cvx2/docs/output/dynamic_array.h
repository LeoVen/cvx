#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "cvx2/core.h"
#include "cvx2/flags.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @file hamt.h
 * @author Leonardo Vencovsky
 * @brief A dynamically-resizing array.
 * @version 0.0.1
 *
 */

/**
 * @brief Virtual table of optional callbacks used to manage CVX_VAL elements.
 *
 * @note All callbacks are optional and may be left as NULL (zero-initialized).
 *
 * @note When `copy` is NULL, elements are shallow-copied with memcpy instead.
 * @note When `drop` is NULL, elements are released without any extra cleanup.
 * @note When `comp` is NULL, operations that need ordering return CVX_FLAG_VTAB.
 */
struct DynamicArrayVtabV
{
    CVX_VTAB_COPY(copy, void *); //!< Deep-copies one element
    CVX_VTAB_COMP(comp, void *); //!< Ordering of two elements
    CVX_VTAB_DROP(drop, void *); //!< Releases resources owned by one element
};

/**
 * @brief A dynamically sized array that grows when needed.
 *
 * Indexing is 0 based.
 *
 * @note count is always <= capacity. buffer is NULL exactly when capacity is 0.
 */
struct DynamicArray
{
    size_t capacity;                 //!< Current buffer size
    size_t count;                    //!< How many elements are stored
    struct DynamicArrayVtabV *vtabv; //!< Virtual table for the CVX_VAL type
    void **buffer;                   //!< Buffer of elements
};

/**
 * @brief Initializes self, optionally pre-allocating capacity elements.
 *
 * @param self a non-NULL pointer to an uninitialized dynamic array
 * @param vtabv the vtable used to copy/compare/drop elements, may be NULL
 * @param capacity number of elements to pre-allocate; may be 0, in which case
 * the buffer stays unallocated until the first push
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation of the initial buffer fails
 **/
enum cvx_flags da_init(struct DynamicArray *self, struct DynamicArrayVtabV *vtabv, size_t capacity);
/**
 * @brief Frees the buffer, dropping any elements still stored.
 *
 * @param self a pointer to an initialized dynamic array; NULL is accepted and ignored
 *
 * @note If vtabv->drop is set, it is called for every stored element before the
 * buffer itself is freed.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags da_drop(struct DynamicArray *self);
/**
 * @brief Initializes clone as a deep copy of orig.
 *
 * @param orig a non-NULL pointer to an initialized dynamic array
 * @param clone a non-NULL pointer to an uninitialized dynamic array
 *
 * @note If vtabv->copy is set, it is used to copy each element individually;
 * otherwise the elements are copied with a raw memcpy.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation of clone's buffer fails
 * - any flag returned by vtabv->copy, if copying an element fails
 **/
enum cvx_flags da_clone(struct DynamicArray *orig, struct DynamicArray *clone);
/**
 * @brief Number of elements currently stored.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 *
 * @return the value of self->count
 **/
size_t da_count(struct DynamicArray *self);
/**
 * @brief Current buffer capacity.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 *
 * @return the value of self->capacity
 **/
size_t da_capacity(struct DynamicArray *self);
/**
 * @brief True if count is 0.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 *
 * @return self->count == 0
 **/
bool da_empty(struct DynamicArray *self);
/**
 * @brief True if count equals capacity.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 *
 * @return self->count >= self->capacity
 **/
bool da_full(struct DynamicArray *self);
/**
 * @brief Gets the first element.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param out out parameter for the first element
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the array is empty
 **/
enum cvx_flags da_front(struct DynamicArray *self, void **out);
/**
 * @brief Gets the last element.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param out out parameter for the last element
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the array is empty
 **/
enum cvx_flags da_back(struct DynamicArray *self, void **out);
/**
 * @brief Gets the element at index.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param index the position to read, must be less than count
 * @param out out parameter for the element at `index`
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if index is greater than or equal to count
 **/
enum cvx_flags da_get(struct DynamicArray *self, size_t index, void **out);
/**
 * @brief Inserts item at index, growing the buffer if needed.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param index the position to insert at, must be 0 <= index <= count
 * (0 = front, count = back)
 * @param item the value to insert
 *
 * @note If the buffer is full, it gets reallocated. The new buffer's size
 * depends on `CVX_BUFFER_GROWTH_RATE` and `CVX_BUFFER_MIN_SIZE`.
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if index is greater than count
 * - CVX_FLAG_ALLOC: if reallocation of the buffer fails
 **/
enum cvx_flags da_push(struct DynamicArray *self, size_t index, void *item);
/**
 * @brief Removes and returns the element at index.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param index the position to remove, must be less than count
 * (0 = front, count - 1 = back)
 * @param out out parameter for the removed element
 *
 * @note If `out` is NULL, the value is not set and ignored. The vacated slot
 * at the end of the buffer is zero-initialized; the buffer itself does not shrink.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the array is empty
 * - CVX_FLAG_RANGE: if index is greater than or equal to count
 **/
enum cvx_flags da_pop(struct DynamicArray *self, size_t index, void **out);
/**
 * @brief Replaces the element at a given index, returning the old value through old_out.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param index the position to overwrite, must be less than count
 * @param new_value the value written at `index`
 * @param old_out out parameter for the value previously stored at `index`
 *
 * @note If `old_out` is NULL, the old value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the array is empty
 * - CVX_FLAG_RANGE: if index is greater than or equal to count
 **/
enum cvx_flags da_replace(struct DynamicArray *self, size_t index, void *new_value, void **old_out);
/**
 * @brief Swaps the elements at idx1 and idx2.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param idx1 the position of the first element, must be less than count
 * @param idx2 the position of the second element, must be less than count
 *
 * @note If idx1 equals idx2, this is a no-op.
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if idx1 or idx2 is greater than or equal to count
 **/
enum cvx_flags da_swap(struct DynamicArray *self, size_t idx1, size_t idx2);
/**
 * @brief Lexicographically compares two arrays using vtabv->comp.
 *
 * @param left a non-NULL pointer to an initialized dynamic array
 * @param right a non-NULL pointer to an initialized dynamic array
 * @param out out parameter for the comparison result: negative if left < right,
 * 0 if left == right, positive if left > right
 *
 * @note The comparator is taken from left->vtabv->comp, falling back to
 * right->vtabv->comp if left does not provide one. Elements are compared
 * pairwise until they differ; if one array is a prefix of the other, the
 * shorter array compares as smaller.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if neither left nor right provide a comp function
 **/
enum cvx_flags da_compare(struct DynamicArray *left, struct DynamicArray *right, int *out);
/**
 * @brief Sorts in place using vtabv->comp.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 *
 * @note Sorts in ascending order with a bubble sort, O(n^2).
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabv or vtabv->comp is not set
 **/
enum cvx_flags da_sort(struct DynamicArray *self);
/**
 * @brief Inserts a sequence of values at index, preserving their order.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param index the position to insert at, must be 0 <= index <= count
 * (0 = front, count = back)
 * @param values a non-NULL pointer to the first of `size` elements to insert
 * @param size number of elements to read from `values`
 *
 * @note If the buffer does not have enough free capacity, it gets reallocated.
 * After the call, values[0]..values[size-1] occupy indices index..index+size-1,
 * in order, with the array's contents at and after `index` shifted back by `size`.
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if index is greater than count
 * - CVX_FLAG_ALLOC: if reallocation of the buffer fails
 **/
enum cvx_flags da_seq_push(struct DynamicArray *self, size_t index, void **values, size_t size);
/**
 * @brief Initializes out with a copy of the elements in the inclusive range [from, to].
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param from the first index included in the sublist, must be <= to
 * @param to the last index included in the sublist, must be less than count
 * @param out a non-NULL pointer to an uninitialized dynamic array
 *
 * @note Elements are copied with self->vtabv->copy if set, otherwise shallow-copied.
 * `self` is left unchanged.
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if from > to, or to is greater than or equal to count
 * - CVX_FLAG_ALLOC: if allocation of out's buffer fails
 * - any flag returned by vtabv->copy, if copying an element fails
 **/
enum cvx_flags da_sublist(struct DynamicArray *self, size_t from, size_t to,
                          struct DynamicArray *out);
/**
 * @brief Appends a copy of every element of src to the back of self.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param src a non-NULL pointer to an initialized dynamic array
 *
 * @note Elements are copied with self->vtabv->copy if set, otherwise shallow-copied.
 * `src` is left unchanged. The buffer grows as needed.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if reallocation of the buffer fails
 * - any flag returned by vtabv->copy, if copying an element fails
 **/
enum cvx_flags da_append(struct DynamicArray *self, struct DynamicArray *src);
/**
 * @brief Inserts a copy of every element of src at the front of self, preserving src's order.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param src a non-NULL pointer to an initialized dynamic array
 *
 * @note Elements are copied with self->vtabv->copy if set, otherwise shallow-copied.
 * `src` is left unchanged. The buffer grows as needed.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if reallocation of the buffer fails
 * - any flag returned by vtabv->copy, if copying an element fails
 **/
enum cvx_flags da_prepend(struct DynamicArray *self, struct DynamicArray *src);
/**
 * @brief Finds the index of the first element comparing equal to item, using vtabv->comp.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param item the value to search for
 * @param out out parameter for the index of the first match
 *
 * @note If `out` is NULL, the value is not set and ignored -- this is also how
 * to just check whether item is present: CVX_FLAG_OK means it is.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabv or vtabv->comp is not set
 * - CVX_FLAG_NOT_FOUND: if no element compares equal to item
 **/
enum cvx_flags da_index_of(struct DynamicArray *self, void *item, size_t *out);
/**
 * @brief Shrinks the array down to new_count elements, dropping the rest.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param new_count desired element count
 *
 * @note Passing 0 empties the array entirely, without freeing the buffer --
 * this is cvx2's equivalent of other languages' "clear". Capacity is never
 * touched; call `_shrink` afterward to reclaim memory. A no-op if new_count
 * is already >= count.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags da_truncate(struct DynamicArray *self, size_t new_count);
/**
 * @brief Shrinks the buffer's capacity down to count, freeing unused memory.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 *
 * @note This is a no-op if capacity already equals count. If count is 0, the
 * buffer is freed and set to NULL.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if reallocation of the buffer fails
 **/
enum cvx_flags da_shrink(struct DynamicArray *self);
/**
 * @brief Grows the buffer to at least capacity, even if it is not yet full.
 *
 * @param self a non-NULL pointer to an initialized dynamic array
 * @param capacity desired minimum capacity for buffer
 *
 * @note A no-op if capacity is already <= self->capacity -- this only grows;
 * use `_shrink` to reclaim memory instead.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if reallocation of the buffer fails
 **/
enum cvx_flags da_resize(struct DynamicArray *self, size_t capacity);

#endif /* DYNAMIC_ARRAY_H */
