/**
 * @file
 * @author Leonardo Vencovsky
 * @brief A hashtable mapping CVX_KEY -> CVX_VAL, or a bare set of CVX_KEY.
 * @version 0.0.1
 *
 */

#include "cvx2/fallback.h"

// clang-format off
#ifndef CVX_KEY
#error "hashtable.h requires CVX_KEY to be defined (the key type)"
#endif
#if defined(CVX2_KIND_MAP) && !defined(CVX_VAL)
#error "hashtable.h requires CVX_VAL to be defined when variants.kind is \"map\""
#endif
#if defined(CVX2_KIND_SET) && defined(CVX_VAL)
#error "hashtable.h: CVX_VAL must not be defined when variants.kind is \"set\" (a set has no values)"
#endif
#ifndef CVX_SNAME
#error "hashtable.h requires CVX_SNAME to be defined (the struct name)"
#endif
#ifndef CVX_PFX
#error "hashtable.h requires CVX_PFX to be defined (the function prefix)"
#endif
// clang-format on

#include <stdbool.h>
#include <stddef.h>

#include "cvx2/core.h"
#include "cvx2/flags.h"
#include "cvx2/names.h"

#define FUNC(X) CVX_(CVX_PFX, NAME##X)
#define VTAB_K CVX_(CVX_SNAME, NAME##_vtabk)
#define VTAB_V CVX_(CVX_SNAME, NAME##_vtabv)
#define ENTRY CVX_(CVX_SNAME, NAME##_entry)
#define NODE CVX_(CVX_SNAME, NAME##_node)

/**
 * @brief Virtual table of callbacks used to manage CVX_KEY keys.
 *
 * @note hash and comp are required. copy and drop are optional.
 *
 * @note When `copy` is NULL, keys are shallow-copied with memcpy instead.
 * @note When `drop` is NULL, keys are released without any extra cleanup.
 */
struct VTAB_K
{
    CVX_VTAB_COMP(comp, CVX_KEY);
    CVX_VTAB_COPY(copy, CVX_KEY);
    CVX_VTAB_DROP(drop, CVX_KEY);
    CVX_VTAB_HASH(hash, CVX_KEY);
};

#ifdef CVX2_KIND_MAP
/**
 * @brief Virtual table of optional callbacks used to manage CVX_VAL values.
 *
 * @note Only present when variants.kind is "map".
 *
 * @note When `copy` is NULL, values are shallow-copied with memcpy instead.
 * @note When `drop` is NULL, values are released without any extra cleanup.
 */
struct VTAB_V
{
    CVX_VTAB_COPY(copy, CVX_VAL);
    CVX_VTAB_DROP(drop, CVX_VAL);
};
#endif

#ifdef CVX2_COLLISION_OPEN_ADDRESSING
// collision strategy: open addressing

/**
 * @brief One slot of the open-addressing buffer.
 *
 * @note `dist` and `state` are internal robin-hood/tombstone bookkeeping;
 * do not modify directly. `key` (and `val`, for a map) are only meaningful
 * while `state` is filled.
 */
struct ENTRY
{
    CVX_KEY key; //!< Stored key
#ifdef CVX2_KIND_MAP
    CVX_VAL val; //!< Stored value; only present when variants.kind is "map"
#endif
    size_t dist; //!< Displacement from the key's home slot (robin hood)
    int state;   //!< CVX2_HT_ENTRY_EMPTY / _FILLED / _DELETED; internal, do not modify
};

/**
 * @brief A hashtable using open addressing with robin-hood linear probing.
 *
 * @note capacity is always a prime number. Deletions use tombstones, which
 * are purged on the next resize.
 */
struct CVX_SNAME
{
    size_t capacity;      //!< Total buffer slots
    size_t count;         //!< Number of filled entries
    double load;          //!< Resize threshold (count >= capacity * load)
    struct VTAB_K *vtabk; //!< Virtual table for CVX_KEY; hash and comp are required
#ifdef CVX2_KIND_MAP
    struct VTAB_V *vtabv; //!< Virtual table for CVX_VAL; only present when variants.kind is "map"
#endif
    struct ENTRY *buffer; //!< Open-addressing slot buffer, capacity entries long
};
#endif

#ifdef CVX2_COLLISION_SEPARATE_CHAINING
// collision strategy: separate chaining

/**
 * @brief One entry in a bucket's collision chain.
 */
struct NODE
{
    CVX_KEY key; //!< Stored key
#ifdef CVX2_KIND_MAP
    CVX_VAL val; //!< Stored value; only present when variants.kind is "map"
#endif
    struct NODE *next; //!< Next entry in this bucket's chain, or NULL
};

/**
 * @brief A hashtable using separate chaining: one linked list per bucket.
 *
 * @note capacity is always a prime number (the bucket count).
 */
struct CVX_SNAME
{
    size_t capacity;      //!< Bucket count
    size_t count;         //!< Number of stored entries
    double load;          //!< Resize threshold (count >= capacity * load)
    struct VTAB_K *vtabk; //!< Virtual table for CVX_KEY; hash and comp are required
#ifdef CVX2_KIND_MAP
    struct VTAB_V *vtabv; //!< Virtual table for CVX_VAL; only present when variants.kind is "map"
#endif
    struct NODE **buckets; //!< Array of capacity bucket heads, each a chain of struct NODE
};
#endif

#ifdef CVX2_KIND_MAP
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
enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv,
                           size_t capacity);
#else
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
enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_K *vtabk, size_t capacity);
#endif
#ifdef CVX2_KIND_MAP
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
#else
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
#endif
enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone);
#ifdef CVX2_KIND_MAP
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
#else
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
#endif
enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self);
/**
 * @brief Number of entries currently stored.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return the value of self->count
 **/
size_t FUNC(_count)(struct CVX_SNAME *self);
/**
 * @brief Current number of buffer/bucket slots.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return the value of self->capacity
 **/
size_t FUNC(_capacity)(struct CVX_SNAME *self);
/**
 * @brief Resize threshold, as a fraction of capacity.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return the value of self->load
 **/
double FUNC(_load)(struct CVX_SNAME *self);
/**
 * @brief True if count is 0.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 *
 * @return self->count == 0
 **/
bool FUNC(_empty)(struct CVX_SNAME *self);

#ifdef CVX2_KIND_MAP
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
enum cvx_flags FUNC(_insert)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL val);
#else
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
enum cvx_flags FUNC(_insert)(struct CVX_SNAME *self, CVX_KEY key);
#endif
#ifdef CVX2_KIND_MAP
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
enum cvx_flags FUNC(_update)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL new_val,
                             CVX_VAL *old_out);
#endif
#ifdef CVX2_KIND_MAP
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
enum cvx_flags FUNC(_remove)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out);
#else
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
enum cvx_flags FUNC(_remove)(struct CVX_SNAME *self, CVX_KEY key);
#endif
#ifdef CVX2_KIND_MAP
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
enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out);
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
CVX_VAL *FUNC(_get_ref)(struct CVX_SNAME *self, CVX_KEY key);
#endif
/**
 * @brief True if key is present.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 * @param key the key to look up
 *
 * @return true if key is present, false if absent or if vtabk is unset
 **/
bool FUNC(_contains)(struct CVX_SNAME *self, CVX_KEY key);
