/**
 * @file
 * @author Leonardo Vencovsky
 * @brief Hash Array Mapped Trie
 * @version 0.0.1
 *
 */

#include "cvx/fallback.h"

// clang-format off
#ifndef CVX_KEY
#error "hamt.h requires CVX_KEY to be defined (the key type)"
#endif
#ifndef CVX_VAL
#error "hamt.h requires CVX_VAL to be defined (the element type)"
#endif
#ifndef CVX_SNAME
#error "hamt.h requires CVX_SNAME to be defined (the struct name)"
#endif
#ifndef CVX_PFX
#error "hamt.h requires CVX_PFX to be defined (the function prefix)"
#endif
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cvx/core.h"
#include "cvx/flags.h"
#include "cvx/names.h"

#define FUNC(X) CVX_(CVX_PFX, NAME##X)
#define VTAB_K CVX_(CVX_SNAME, NAME##_vtabk)
#define VTAB_V CVX_(CVX_SNAME, NAME##_vtabv)
#define NODE CVX_(CVX_SNAME, NAME##_node)
#define ENTRY CVX_(CVX_SNAME, NAME##_entry)
#define INTERNAL CVX_(CVX_SNAME, NAME##_internal)

// clang-format off
#if defined(CVX_HAMT_BITMAP_8) + defined(CVX_HAMT_BITMAP_16) + defined(CVX_HAMT_BITMAP_32) + defined(CVX_HAMT_BITMAP_64) > 1
#error "hamt.h: define at most one of CVX_HAMT_BITMAP_8, CVX_HAMT_BITMAP_16, CVX_HAMT_BITMAP_32, CVX_HAMT_BITMAP_64"
#endif
#if defined(CVX_HAMT_HASH_32) + defined(CVX_HAMT_HASH_64) > 1
#error "hamt.h: define at most one of CVX_HAMT_HASH_32, CVX_HAMT_HASH_64"
#endif
// clang-format on

#if defined(CVX_HAMT_BITMAP_8)
#define CVX_HAMT_BITMAP_T uint8_t
#define CVX_HAMT_CHUNK_BITS 3
#elif defined(CVX_HAMT_BITMAP_16)
#define CVX_HAMT_BITMAP_T uint16_t
#define CVX_HAMT_CHUNK_BITS 4
#elif defined(CVX_HAMT_BITMAP_64)
#define CVX_HAMT_BITMAP_T uint64_t
#define CVX_HAMT_CHUNK_BITS 6
#else
#define CVX_HAMT_BITMAP_T uint32_t
#define CVX_HAMT_CHUNK_BITS 5
#endif

#if defined(CVX_HAMT_HASH_64)
#define CVX_HAMT_HASH_T uint64_t
#define CVX_HAMT_HASH_BITS 64
#else
#define CVX_HAMT_HASH_T uint32_t
#define CVX_HAMT_HASH_BITS 32
#endif

#define CVX_HAMT_CHUNK_MASK ((1U << CVX_HAMT_CHUNK_BITS) - 1)
#define CVX_HAMT_DEPTH ((CVX_HAMT_HASH_BITS + CVX_HAMT_CHUNK_BITS - 1) / CVX_HAMT_CHUNK_BITS)

struct VTAB_K
{
    CVX_VTAB_COMP(comp, CVX_KEY);
    CVX_VTAB_COPY(copy, CVX_KEY);
    CVX_VTAB_DROP(drop, CVX_KEY);
    CVX_VTAB_HASH(hash, CVX_KEY);
};

struct VTAB_V
{
    CVX_VTAB_COPY(copy, CVX_VAL);
    CVX_VTAB_DROP(drop, CVX_VAL);
};

struct ENTRY
{
    CVX_HAMT_HASH_T hash;
    CVX_KEY key;
    CVX_VAL val;
    struct ENTRY *next;
};

struct INTERNAL
{
    CVX_HAMT_BITMAP_T bitmap;
    struct NODE **children;
};

struct NODE
{
    enum cvx_hamt_node_t type;
    union
    {
        struct ENTRY leaf;
        struct INTERNAL internal;
    } as;
};

struct CVX_SNAME
{
    size_t count;
    struct VTAB_K *vtabk;
    struct VTAB_V *vtabv;
    struct NODE *root;
};

/**
 * @brief Initializes self.
 *
 * @param self a non-NULL pointer to an uninitialized HAMT
 * @param vtabk the vtable used to hash/compare/copy/drop keys; hash and comp are required
 * @param vtabv the vtable used to copy/drop values, may be NULL
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabk is NULL, or vtabk->hash or vtabk->comp is NULL
 **/
enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv);
/**
 * @brief Frees the HAMT, dropping any entries still stored.
 *
 * @param self a pointer to an initialized HAMT; NULL is accepted and ignored
 *
 * @note If vtabk->drop is set, it is called for every stored key before the
 * buffer itself is freed. Same for values if vtabk->drop is set.
 *
 * Error Handling
 * - always returns CVX_FLAG_OK
 **/
void FUNC(_drop)(struct CVX_SNAME *self);
/**
 * @brief Initializes clone as a deep copy of orig.
 *
 * @param orig a non-NULL pointer to an initialized hashtable
 * @param clone a non-NULL pointer to an uninitialized hashtable
 *
 * @note Keys are copied with vtabk->copy if set, otherwise shallow-copied.
 *
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation of clone's nodes or buffers fails
 **/
enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone);
/**
 * @brief Inserts a key value pair.
 *
 * @param self a non-NULL pointer to an initialized HAMT
 * @param key the key to insert
 * @param val the value to associate with key
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabk, vtabk->hash, or vtabk->comp is not set
 * - CVX_FLAG_DUPLICATE: if key is already present
 * - CVX_FLAG_ALLOC: if allocation for nodes or buffers fails
 **/
enum cvx_flags FUNC(_insert)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL val);
/**
 * @brief Gets the value for key.
 *
 * @param self a non-NULL pointer to an initialized hashtable
 * @param key the key to look up
 * @param out out parameter for the value associated with key
 *
 * @note If `out` is NULL, the value is not set and ignored.
 *
 * Error Handling
 * - CVX_FLAG_VTAB: if vtabk, vtabk->hash, or vtabk->comp is not set
 * - CVX_FLAG_NOT_FOUND: if key is absent
 **/
enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out);
