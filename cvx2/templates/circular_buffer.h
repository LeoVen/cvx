/**
 * @file
 * @author Leonardo Vencovsky
 * @brief A fixed-sized array where inserts and deletes wrap around the ends.
 * @version 0.0.1
 *
 * Insertion and deletion happen at the ends of the array and can wrap around.
 * Once the buffer is full, the next insertion overwrites the other end.
 *
 */

#include "cvx2/fallback.h"

// clang-format off
#ifndef CVX_VAL
#error "circular_buffer.h requires CVX_VAL to be defined (the element type)"
#endif
#ifndef CVX_SNAME
#error "circular_buffer.h requires CVX_SNAME to be defined (the struct name)"
#endif
#ifndef CVX_PFX
#error "circular_buffer.h requires CVX_PFX to be defined (the function prefix)"
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
 * @brief Virtual table of optional callbacks used to manage CVX_VAL elements.
 *
 * @note All callbacks are optional and may be left as NULL (zero-initialized).
 *
 * @note When `copy` is NULL, elements are shallow-copied instead.
 * @note When `drop` is NULL, elements are released without any extra cleanup.
 */
struct VTAB_V
{
    CVX_VTAB_COPY(copy, CVX_VAL); //!< Deep-copies one element
    CVX_VTAB_DROP(drop, CVX_VAL); //!< Releases resources owned by one element
};

/**
 * @brief A fixed-capacity ring of elements, overwriting the opposite end when full.
 *
 * Indexing is 0 based and logical: index 0 is always the front element,
 * wherever it physically sits in buffer.
 *
 * @note count is always <= capacity. buffer is NULL exactly when capacity is 0.
 */
struct CVX_SNAME
{
    size_t capacity;      //!< Buffer size; the buffer never grows on its own
    size_t count;         //!< How many elements are stored
    size_t head;          //!< Physical index of the front element
    struct VTAB_V *vtabv; //!< Virtual table for the CVX_VAL type
    CVX_VAL *buffer;      //!< Buffer of elements
};

/**
 * @brief Initializes self with room for exactly capacity elements.
 *
 * @param self a non-NULL pointer to an uninitialized circular buffer
 * @param vtabv the vtable used to copy/drop elements, may be NULL
 * @param capacity number of elements the buffer holds; may be 0, in which case
 * every push fails with CVX_FLAG_FULL until `_set_capacity` is called
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation of the buffer fails
 **/
enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_V *vtabv, size_t capacity);
/**
 * @brief Frees the buffer, dropping any elements still stored.
 *
 * @param self a pointer to an initialized circular buffer; NULL is accepted and ignored
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
 * @param orig a non-NULL pointer to an initialized circular buffer
 * @param clone a non-NULL pointer to an uninitialized circular buffer
 *
 * @note If vtabv->copy is set, it is used to copy each element individually;
 * otherwise the elements are shallow-copied. The clone is linearized: its
 * front element is at physical index 0.
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
 * @param self a non-NULL pointer to an initialized circular buffer
 *
 * @return the value of self->count
 **/
size_t FUNC(_count)(struct CVX_SNAME *self);
/**
 * @brief Number of elements the buffer can hold.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 *
 * @return the value of self->capacity
 **/
size_t FUNC(_capacity)(struct CVX_SNAME *self);
/**
 * @brief True if count is 0.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 *
 * @return self->count == 0
 **/
bool FUNC(_empty)(struct CVX_SNAME *self);
/**
 * @brief True if count equals capacity, meaning the next push overwrites an element.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 *
 * @return self->count == self->capacity
 **/
bool FUNC(_full)(struct CVX_SNAME *self);
/**
 * @brief True if the elements are contiguous in buffer, without wrapping around its end.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 *
 * @note When true, buffer[head] .. buffer[head + count - 1] are the elements in
 * logical order. An empty buffer is always linearized.
 *
 * @return self->head + self->count <= self->capacity
 **/
bool FUNC(_is_linearized)(struct CVX_SNAME *self);
/**
 * @brief Gets the front element.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 * @param out out parameter for the front element
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the buffer is empty
 **/
enum cvx_flags FUNC(_front)(struct CVX_SNAME *self, CVX_VAL *out);
/**
 * @brief Gets the back element.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 * @param out out parameter for the back element
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the buffer is empty
 **/
enum cvx_flags FUNC(_back)(struct CVX_SNAME *self, CVX_VAL *out);
/**
 * @brief Gets the element at a logical index.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 * @param index the logical position to read (0 = front), must be less than count
 * @param out out parameter for the element at `index`
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if index is greater than or equal to count
 **/
enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, size_t index, CVX_VAL *out);
/**
 * @brief Adds item at the back. When full, the front element is overwritten.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 * @param item the value to insert
 *
 * @note When full, vtabv->drop (if set) is called on the overwritten front
 * element, and the element after it becomes the new front.
 *
 * Error Handling
 * - CVX_FLAG_FULL: if capacity is 0
 **/
enum cvx_flags FUNC(_push_back)(struct CVX_SNAME *self, CVX_VAL item);
/**
 * @brief Adds item at the front. When full, the back element is overwritten.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 * @param item the value to insert
 *
 * @note When full, vtabv->drop (if set) is called on the overwritten back
 * element, and the element before it becomes the new back.
 *
 * Error Handling
 * - CVX_FLAG_FULL: if capacity is 0
 **/
enum cvx_flags FUNC(_push_front)(struct CVX_SNAME *self, CVX_VAL item);
/**
 * @brief Removes and returns the back element.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 * @param out out parameter for the removed element
 *
 * @note If `out` is NULL, the value is not set and ignored. The vacated slot is
 * zero-initialized.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the buffer is empty
 **/
enum cvx_flags FUNC(_pop_back)(struct CVX_SNAME *self, CVX_VAL *out);
/**
 * @brief Removes and returns the front element.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 * @param out out parameter for the removed element
 *
 * @note If `out` is NULL, the value is not set and ignored. The vacated slot is
 * zero-initialized.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the buffer is empty
 **/
enum cvx_flags FUNC(_pop_front)(struct CVX_SNAME *self, CVX_VAL *out);
/**
 * @brief Changes the capacity, keeping the elements in logical order.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 * @param capacity the new capacity; 0 drops every element and frees the buffer
 *
 * @note When shrinking below count, the excess front elements are dropped
 * (vtabv->drop, if set), keeping the newest elements at the back. The result
 * is linearized.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation of the new buffer fails; self is left unchanged
 **/
enum cvx_flags FUNC(_set_capacity)(struct CVX_SNAME *self, size_t capacity);
/**
 * @brief Rearranges the buffer in place so the front element is at physical index 0.
 *
 * @param self a non-NULL pointer to an initialized circular buffer
 *
 * @note Afterwards, buffer[0] .. buffer[count - 1] are the elements in logical
 * order, so buffer can be passed to code expecting a plain array. Logical
 * indices are unchanged. Done in place, without allocating.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags FUNC(_linearize)(struct CVX_SNAME *self);
