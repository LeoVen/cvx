#ifndef HASHSET_H
#define HASHSET_H

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
struct HashSetVtabK
{
    CVX_VTAB_COMP(comp, void *);
    CVX_VTAB_COPY(copy, void *);
    CVX_VTAB_DROP(drop, void *);
    CVX_VTAB_HASH(hash, void *);
};

// collision strategy: open addressing

/**
 * @brief One slot of the open-addressing buffer.
 *
 * @note `dist` and `state` are internal robin-hood/tombstone bookkeeping;
 * do not modify directly. `key` (and `val`, for a map) are only meaningful
 * while `state` is filled.
 */
struct HashSetEntry
{
    void *key;   //!< Stored key
    size_t dist; //!< Displacement from the key's home slot (robin hood)
    int state;   //!< CVX2_HT_ENTRY_EMPTY / _FILLED / _DELETED; internal, do not modify
};

/**
 * @brief A hashtable using open addressing with robin-hood linear probing.
 *
 * @note capacity is always a prime number. Deletions use tombstones, which
 * are purged on the next resize.
 */
struct HashSet
{
    size_t capacity;             //!< Total buffer slots
    size_t count;                //!< Number of filled entries
    double load;                 //!< Resize threshold (count >= capacity * load)
    struct HashSetVtabK *vtabk;  //!< Virtual table for CVX_KEY; hash and comp are required
    struct HashSetEntry *buffer; //!< Open-addressing slot buffer, capacity entries long
};

/**
 * @brief Initializes self, optionally pre-allocating capacity slots.
 *
 * @param self a non-NULL pointer to an uninitialized hashtable
 * @param vtabk the vtable used to hash/compare/copy/drop keys; hash and comp are required
 * @param capacity number of slots to pre-allocate; may be 0, in which case
 * the buffer stays unallocated until the first insert
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabk is NULL, or vtabk->hash or vtabk->comp is NULL
 * - CVX_FLAG_ALLOC: if allocation of the initial buffer fails
 **/
enum cvx_flags hs_init(struct HashSet *self, struct HashSetVtabK *vtabk, size_t capacity);
/**
 * @brief Initializes clone as a deep copy of orig.
 *
 * @param orig a non-NULL pointer to an initialized hashtable
 * @param clone a non-NULL pointer to an uninitialized hashtable
 *
 * @note Keys are copied with vtabk->copy if set, otherwise shallow-copied.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation of clone's buffer fails
 **/
enum cvx_flags hs_clone(struct HashSet *orig, struct HashSet *clone);
/**
 * @brief Frees the table, dropping any entries still stored.
 *
 * @param self a pointer to an initialized hashtable; NULL is accepted and ignored
 *
 * @note If vtabk->drop is set, it is called for every stored key before the
 * buffer itself is freed.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
enum cvx_flags hs_drop(struct HashSet *self);
/**
 * @brief Number of entries currently stored.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return the value of self->count
 **/
size_t hs_count(struct HashSet *self);
/**
 * @brief Current number of buffer/bucket slots.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return the value of self->capacity
 **/
size_t hs_capacity(struct HashSet *self);
/**
 * @brief Resize threshold, as a fraction of capacity.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return the value of self->load
 **/
double hs_load(struct HashSet *self);
/**
 * @brief True if count is 0.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return self->count == 0
 **/
bool hs_empty(struct HashSet *self);

/**
 * @brief Inserts key.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 * @param key the key to insert
 *
 * @note If the table is full enough to cross the load factor, it is resized
 * before insertion.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabk, vtabk->hash, or vtabk->comp is not set
 * - CVX_FLAG_DUPLICATE: if key is already present
 * - CVX_FLAG_ALLOC: if a required resize fails to allocate
 **/
enum cvx_flags hs_insert(struct HashSet *self, void *key);
/**
 * @brief Removes key.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 * @param key the key to remove, must be present
 *
 * @note The removed key is dropped via vtabk->drop, if set.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabk, vtabk->hash, or vtabk->comp is not set
 * - CVX_FLAG_EMPTY: if the table is empty
 * - CVX_FLAG_NOT_FOUND: if key is absent
 **/
enum cvx_flags hs_remove(struct HashSet *self, void *key);
/**
 * @brief True if key is present.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 * @param key the key to look up
 *
 * @return true if key is present, false if absent or if vtabk is unset
 **/
bool hs_contains(struct HashSet *self, void *key);

#endif /* HASHSET_H */
