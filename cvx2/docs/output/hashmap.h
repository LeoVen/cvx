#ifndef HASHMAP_H
#define HASHMAP_H

#include "cvx2/core.h"
#include "cvx2/flags.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @file hashtable.h
 * @author Leonardo Vencovsky
 * @brief A hashtable mapping CVX_KEY -> CVX_VAL, or a bare set of CVX_KEY.
 * @version 0.0.1
 *
 */

/**
 * @brief Virtual table of callbacks used to manage CVX_KEY keys.
 *
 * @note hash and comp are required. copy and drop are optional.
 *
 * @note When `copy` is NULL, keys are shallow-copied with memcpy instead.
 * @note When `drop` is NULL, keys are released without any extra cleanup.
 */
struct HashMapVtabK
{
    CVX_VTAB_COMP(comp, void *);
    CVX_VTAB_COPY(copy, void *);
    CVX_VTAB_DROP(drop, void *);
    CVX_VTAB_HASH(hash, void *);
};

/**
 * @brief Virtual table of optional callbacks used to manage CVX_VAL values.
 *
 * @note Only present when variants.kind is "map".
 *
 * @note When `copy` is NULL, values are shallow-copied with memcpy instead.
 * @note When `drop` is NULL, values are released without any extra cleanup.
 */
struct HashMapVtabV
{
    CVX_VTAB_COPY(copy, void *);
    CVX_VTAB_DROP(drop, void *);
};

// collision strategy: separate chaining

/**
 * @brief One entry in a bucket's collision chain.
 */
struct HashMapNode
{
    void *key;                //!< Stored key
    void *val;                //!< Stored value; only present when variants.kind is "map"
    struct HashMapNode *next; //!< Next entry in this bucket's chain, or NULL
};

/**
 * @brief A hashtable using separate chaining: one linked list per bucket.
 *
 * @note capacity is always a prime number (the bucket count).
 */
struct HashMap
{
    size_t capacity;            //!< Bucket count
    size_t count;               //!< Number of stored entries
    double load;                //!< Resize threshold (count >= capacity * load)
    struct HashMapVtabK *vtabk; //!< Virtual table for CVX_KEY; hash and comp are required
    struct HashMapVtabV
        *vtabv; //!< Virtual table for CVX_VAL; only present when variants.kind is "map"
    struct HashMapNode **buckets; //!< Array of capacity bucket heads, each a chain of struct NODE
};

/**
 * @brief Initializes self, optionally pre-allocating capacity slots.
 *
 * @param self a non-NULL pointer to an uninitialized hashtable
 * @param vtabk the vtable used to hash/compare/copy/drop keys; hash and comp are required
 * @param vtabv the vtable used to copy/drop values, may be NULL
 * @param capacity number of slots to pre-allocate; may be 0, in which case
 * the buffer stays unallocated until the first insert
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabk is NULL, or vtabk->hash or vtabk->comp is NULL
 * - CVX_FLAG_ALLOC: if allocation of the initial buffer fails
 **/
enum cvx_flags hm_init(struct HashMap *self, struct HashMapVtabK *vtabk, struct HashMapVtabV *vtabv,
                       size_t capacity);
/**
 * @brief Initializes clone as a deep copy of orig.
 *
 * @param orig a non-NULL pointer to an initialized hashtable
 * @param clone a non-NULL pointer to an uninitialized hashtable
 *
 * @note Keys are copied with vtabk->copy if set, otherwise shallow-copied.
 * Values are copied with vtabv->copy if set, otherwise shallow-copied.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation of clone's buffer fails
 **/
enum cvx_flags hm_clone(struct HashMap *orig, struct HashMap *clone);
/**
 * @brief Frees the table, dropping any entries still stored.
 *
 * @param self a pointer to an initialized hashtable; NULL is accepted and ignored
 *
 * @note If vtabk->drop is set, it is called for every stored key before the
 * buffer itself is freed. If vtabv->drop is set, it is called for every
 * stored value the same way.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags hm_drop(struct HashMap *self);
/**
 * @brief Number of entries currently stored.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return the value of self->count
 **/
size_t hm_count(struct HashMap *self);
/**
 * @brief Current number of buffer/bucket slots.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return the value of self->capacity
 **/
size_t hm_capacity(struct HashMap *self);
/**
 * @brief Resize threshold, as a fraction of capacity.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return the value of self->load
 **/
double hm_load(struct HashMap *self);
/**
 * @brief True if count is 0.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return self->count == 0
 **/
bool hm_empty(struct HashMap *self);

/**
 * @brief Inserts key/val.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 * @param key the key to insert
 * @param val the value to associate with key
 *
 * @note If the table is full enough to cross the load factor, it is resized
 * before insertion.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabk, vtabk->hash, or vtabk->comp is not set
 * - CVX_FLAG_DUPLICATE: if key is already present
 * - CVX_FLAG_ALLOC: if a required resize fails to allocate
 **/
enum cvx_flags hm_insert(struct HashMap *self, void *key, void *val);
/**
 * @brief Replaces the value for key, returning the old value through old_out.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 * @param key the key to update, must already be present
 * @param new_val the value written for key
 * @param old_out out parameter for the value previously associated with key
 *
 * @note If `old_out` is NULL, the old value is dropped via vtabv->drop (if
 * set) instead of returned.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabk, vtabk->hash, or vtabk->comp is not set
 * - CVX_FLAG_NOT_FOUND: if key is absent
 **/
enum cvx_flags hm_update(struct HashMap *self, void *key, void *new_val, void **old_out);
/**
 * @brief Removes key, returning its value through out.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 * @param key the key to remove, must be present
 * @param out out parameter for the removed value
 *
 * @note If `out` is NULL, the value is dropped via vtabv->drop (if set)
 * instead of returned. The removed key is always dropped via vtabk->drop,
 * if set.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabk, vtabk->hash, or vtabk->comp is not set
 * - CVX_FLAG_EMPTY: if the table is empty
 * - CVX_FLAG_NOT_FOUND: if key is absent
 **/
enum cvx_flags hm_remove(struct HashMap *self, void *key, void **out);
/**
 * @brief Gets the value for key.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 * @param key the key to look up
 * @param out out parameter for the value associated with key
 *
 * @note If `out` is NULL, the value is not set and ignored -- this is also
 * how to just check whether key is present, though `_contains` reads clearer.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabk, vtabk->hash, or vtabk->comp is not set
 * - CVX_FLAG_NOT_FOUND: if key is absent
 **/
enum cvx_flags hm_get(struct HashMap *self, void *key, void **out);
/**
 * @brief Gets a pointer to the stored value for key, or NULL if absent.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 * @param key the key to look up
 *
 * @note The returned pointer is invalidated by any operation that may
 * resize the table (e.g. `_insert`).
 *
 * @return a pointer to the stored value, or NULL if vtabk is unset or key is absent
 **/
void **hm_get_ref(struct HashMap *self, void *key);
/**
 * @brief True if key is present.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 * @param key the key to look up
 *
 * @return true if key is present, false if absent or if vtabk is unset
 **/
bool hm_contains(struct HashMap *self, void *key);

#endif /* HASHMAP_H */
