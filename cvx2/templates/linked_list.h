/**
 * @file
 * @author Leonardo Vencovsky
 * @brief A linked list, either singly (variants.node = "single") or doubly
 * (variants.node = "double") linked.
 * @version 0.0.1
 *
 * The list owns its nodes: they are allocated by the insert functions and freed
 * by the remove functions and `_drop`. Nodes are public, so the list can be
 * walked directly and a `struct NODE *` can be used as a position:
 *
 * ```
 * for (struct NODE *node = list.head; node; node = node->next)
 *     use(node->value);
 * ```
 *
 * ## Complexity
 *
 * | Operation                          | single   | double            |
 * |------------------------------------|----------|-------------------|
 * | push(0), push(count), pop(0)       | O(1)     | O(1)              |
 * | pop(count - 1)                     | O(n)     | O(1)              |
 * | get, push, pop, replace at index i | O(i)     | O(min(i, n - i))  |
 * | insert_after, remove_after         | O(1)     | O(1) / -          |
 * | insert_before, remove              | -        | O(1)              |
 * | splice at the front or back        | O(1)     | O(1)              |
 * | sort                               | O(n log n), stable, no allocation |
 *
 * `_remove_after` only exists for single lists, `_insert_before` and `_remove`
 * only for double lists.
 */

#include "cvx2/fallback.h"

// clang-format off
#ifndef CVX_VAL
#error "linked_list.h requires CVX_VAL to be defined (the element type)"
#endif
#ifndef CVX_SNAME
#error "linked_list.h requires CVX_SNAME to be defined (the struct name)"
#endif
#ifndef CVX_PFX
#error "linked_list.h requires CVX_PFX to be defined (the function prefix)"
#endif
#if defined(CVX2_NODE_SINGLE) == defined(CVX2_NODE_DOUBLE)
#error "linked_list.h requires exactly one of CVX2_NODE_SINGLE or CVX2_NODE_DOUBLE"
#endif
// clang-format on

#include <stdbool.h>
#include <stddef.h>

#include "cvx2/core.h"
#include "cvx2/flags.h"
#include "cvx2/names.h"

#define FUNC(X) CVX_(CVX_PFX, NAME##X)
#define VTAB_V CVX_(CVX_SNAME, NAME##_vtabv)
#define NODE CVX_(CVX_SNAME, NAME##_node)

/**
 * @brief Virtual table of optional callbacks used to manage CVX_VAL elements.
 *
 * @note All callbacks are optional and may be left as NULL (zero-initialized).
 *
 * @note When `copy` is NULL, elements are shallow-copied instead.
 * @note When `drop` is NULL, elements are released without any extra cleanup.
 * @note When `comp` is NULL, operations that need ordering return CVX_FLAG_VTAB.
 */
struct VTAB_V
{
    CVX_VTAB_COPY(copy, CVX_VAL); //!< Deep-copies one element
    CVX_VTAB_COMP(comp, CVX_VAL); //!< Ordering of two elements
    CVX_VTAB_DROP(drop, CVX_VAL); //!< Releases resources owned by one element
};

/**
 * @brief One element of the list.
 *
 * @note Only `value` may be modified directly. The links are maintained by the
 * list functions.
 */
struct NODE
{
    CVX_VAL value; //!< Stored element
#ifdef CVX2_NODE_DOUBLE
    struct NODE *prev; //!< Previous node, or NULL for the head
#endif
    struct NODE *next; //!< Next node, or NULL for the tail
};

/**
 * @brief A linked list owning its nodes.
 *
 * Indexing is 0 based.
 *
 * @note head and tail are both NULL exactly when count is 0.
 */
struct CVX_SNAME
{
    size_t count;         //!< How many elements are stored
    struct VTAB_V *vtabv; //!< Virtual table for the CVX_VAL type
    struct NODE *head;    //!< First node, or NULL when empty
    struct NODE *tail;    //!< Last node, or NULL when empty
};

/**
 * @brief Initializes self as an empty list.
 *
 * @param self a non-NULL pointer to an uninitialized linked list
 * @param vtabv the vtable used to copy/compare/drop elements, may be NULL
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_V *vtabv);
/**
 * @brief Frees every node, dropping the elements they hold.
 *
 * @param self a pointer to an initialized linked list; NULL is accepted and ignored
 *
 * @note If vtabv->drop is set, it is called for every stored element. self is
 * left as an empty list with the same vtabv.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self);
/**
 * @brief Initializes clone as a deep copy of orig.
 *
 * @param orig a non-NULL pointer to an initialized linked list
 * @param clone a non-NULL pointer to an uninitialized linked list
 *
 * @note If vtabv->copy is set, it is used to copy each element individually;
 * otherwise the elements are shallow-copied.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocating a node fails; clone is left dropped
 * - any flag returned by vtabv->copy, if copying an element fails; clone is
 * left dropped
 **/
enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone);
/**
 * @brief Number of elements currently stored.
 *
 * @param self a non-NULL pointer to an initialized linked list
 *
 * @return the value of self->count
 **/
size_t FUNC(_count)(struct CVX_SNAME *self);
/**
 * @brief True if count is 0.
 *
 * @param self a non-NULL pointer to an initialized linked list
 *
 * @return self->count == 0
 **/
bool FUNC(_empty)(struct CVX_SNAME *self);
/**
 * @brief Gets the first element.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param out out parameter for the first element
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the list is empty
 **/
enum cvx_flags FUNC(_front)(struct CVX_SNAME *self, CVX_VAL *out);
/**
 * @brief Gets the last element.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param out out parameter for the last element
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the list is empty
 **/
enum cvx_flags FUNC(_back)(struct CVX_SNAME *self, CVX_VAL *out);
/**
 * @brief Gets the element at index.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param index the position to read, must be less than count
 * @param out out parameter for the element at `index`
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if index is greater than or equal to count
 **/
enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, size_t index, CVX_VAL *out);
/**
 * @brief Replaces the element at index, returning the old value through old_out.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param index the position to overwrite, must be less than count
 * @param new_value the value written at `index`
 * @param old_out out parameter for the value previously stored at `index`
 *
 * @note If `old_out` is NULL, the old value is dropped (vtabv->drop, if set).
 * Otherwise ownership of it moves to the caller.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the list is empty
 * - CVX_FLAG_RANGE: if index is greater than or equal to count
 **/
enum cvx_flags FUNC(_replace)(struct CVX_SNAME *self, size_t index, CVX_VAL new_value,
                              CVX_VAL *old_out);
/**
 * @brief Inserts item at index.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param index the position to insert at, must be 0 <= index <= count
 * (0 = front, count = back)
 * @param item the value to insert
 *
 * Error Handling
 * - CVX_FLAG_RANGE: if index is greater than count
 * - CVX_FLAG_ALLOC: if allocating the node fails
 **/
enum cvx_flags FUNC(_push)(struct CVX_SNAME *self, size_t index, CVX_VAL item);
/**
 * @brief Removes and returns the element at index.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param index the position to remove, must be less than count
 * (0 = front, count - 1 = back)
 * @param out out parameter for the removed element
 *
 * @note If `out` is NULL, the element is dropped (vtabv->drop, if set).
 * Otherwise ownership of it moves to the caller.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if the list is empty
 * - CVX_FLAG_RANGE: if index is greater than or equal to count
 **/
enum cvx_flags FUNC(_pop)(struct CVX_SNAME *self, size_t index, CVX_VAL *out);
/**
 * @brief Shrinks the list down to new_count elements, dropping the rest.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param new_count desired element count
 *
 * @note Passing 0 empties the list (cvx2's "clear"). A no-op if new_count is
 * already >= count.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags FUNC(_truncate)(struct CVX_SNAME *self, size_t new_count);
/**
 * @brief Finds the index of the first element comparing equal to item, using vtabv->comp.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param item the value to search for
 * @param out out parameter for the index of the first match
 *
 * @note If `out` is NULL, the value is not set and ignored, which is also how
 * to just check whether item is present: CVX_FLAG_OK means it is.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabv or vtabv->comp is not set
 * - CVX_FLAG_NOT_FOUND: if no element compares equal to item
 **/
enum cvx_flags FUNC(_index_of)(struct CVX_SNAME *self, CVX_VAL item, size_t *out);
/**
 * @brief Finds the node of the first element comparing equal to item, using vtabv->comp.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param item the value to search for
 * @param out out parameter for the first matching node
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabv or vtabv->comp is not set
 * - CVX_FLAG_NOT_FOUND: if no element compares equal to item
 **/
enum cvx_flags FUNC(_find)(struct CVX_SNAME *self, CVX_VAL item, struct NODE **out);
/**
 * @brief Inserts item right after node.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param node a node of self, or NULL to insert at the front
 * @param item the value to insert
 *
 * @note The new node is `node->next` afterwards (`self->head` when node is NULL).
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocating the node fails
 **/
enum cvx_flags FUNC(_insert_after)(struct CVX_SNAME *self, struct NODE *node, CVX_VAL item);
#ifdef CVX2_NODE_SINGLE
/**
 * @brief Removes the node right after node, returning its element.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param node a node of self, or NULL to remove the front
 * @param out out parameter for the removed element
 *
 * @note If `out` is NULL, the element is dropped (vtabv->drop, if set).
 * Otherwise ownership of it moves to the caller.
 *
 * Error Handling
 * - CVX_FLAG_EMPTY: if node is NULL and the list is empty
 * - CVX_FLAG_RANGE: if node is the tail, so nothing comes after it
 **/
enum cvx_flags FUNC(_remove_after)(struct CVX_SNAME *self, struct NODE *node, CVX_VAL *out);
#endif
#ifdef CVX2_NODE_DOUBLE
/**
 * @brief Inserts item right before node.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param node a node of self, or NULL to insert at the back
 * @param item the value to insert
 *
 * @note The new node is `node->prev` afterwards (`self->tail` when node is NULL).
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocating the node fails
 **/
enum cvx_flags FUNC(_insert_before)(struct CVX_SNAME *self, struct NODE *node, CVX_VAL item);
/**
 * @brief Removes node from the list, returning its element.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param node a non-NULL node of self; it is freed, so it must not be used afterwards
 * @param out out parameter for the removed element
 *
 * @note If `out` is NULL, the element is dropped (vtabv->drop, if set).
 * Otherwise ownership of it moves to the caller.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags FUNC(_remove)(struct CVX_SNAME *self, struct NODE *node, CVX_VAL *out);
#endif
/**
 * @brief Moves every node of other into self at index, leaving other empty.
 *
 * @param self a non-NULL pointer to an initialized linked list
 * @param index the position to insert at, must be 0 <= index <= count
 * (0 = front, count = back)
 * @param other a non-NULL pointer to an initialized linked list, other than self
 *
 * @note No element is copied or allocated: other's nodes are relinked into
 * self, keeping their order, and from then on are managed with self->vtabv.
 *
 * Error Handling
 * - CVX_FLAG_INVALID: if other is self
 * - CVX_FLAG_RANGE: if index is greater than count
 **/
enum cvx_flags FUNC(_splice)(struct CVX_SNAME *self, size_t index, struct CVX_SNAME *other);
/**
 * @brief Reverses the order of the elements in place.
 *
 * @param self a non-NULL pointer to an initialized linked list
 *
 * @note Nodes are relinked, not reallocated, so node pointers stay valid.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags FUNC(_reverse)(struct CVX_SNAME *self);
/**
 * @brief Sorts in ascending order using vtabv->comp.
 *
 * @param self a non-NULL pointer to an initialized linked list
 *
 * @note Merge sort: stable, O(n log n) and without allocating. Nodes are
 * relinked, not reallocated, so node pointers stay valid.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabv or vtabv->comp is not set
 **/
enum cvx_flags FUNC(_sort)(struct CVX_SNAME *self);
/**
 * @brief Lexicographically compares two lists using vtabv->comp.
 *
 * @param left a non-NULL pointer to an initialized linked list
 * @param right a non-NULL pointer to an initialized linked list
 * @param out out parameter for the comparison result: negative if left < right,
 * 0 if left == right, positive if left > right
 *
 * @note The comparator is taken from left->vtabv->comp, falling back to
 * right->vtabv->comp if left does not provide one. Elements are compared
 * pairwise until they differ; if one list is a prefix of the other, the
 * shorter list compares as smaller.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if neither left nor right provide a comp function
 **/
enum cvx_flags FUNC(_compare)(struct CVX_SNAME *left, struct CVX_SNAME *right, int *out);
