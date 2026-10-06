/**
 * @file hashtable.h
 * @brief A hashtable mapping CVX_KEY -> CVX_VAL.
 *
 * Required config: key_type, value_type, struct_name, prefix.
 * Variant "collision": "open_addressing" or "separate_chaining".
 *
 * vtabk->hash and vtabk->comp are required; copy/drop callbacks are
 * optional. Resizes when count >= capacity * load (default 0.7); capacity
 * is always rounded up to a prime.
 */

#include "cvx2/fallback.h"

// clang-format off
#ifndef CVX_KEY
#error "hashtable.h requires CVX_KEY to be defined (the key type)"
#endif
#ifndef CVX_VAL
#error "hashtable.h requires CVX_VAL to be defined (the value type)"
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

#define FUNC(X) CVX_(CVX_PFX, X)
#define VTAB_K CVX_(CVX_SNAME, _vtabk)
#define VTAB_V CVX_(CVX_SNAME, _vtabv)
#define ENTRY CVX_(CVX_SNAME, _entry)
#define NODE CVX_(CVX_SNAME, _node)

struct VTAB_K
{
    CVX_VTAB_DEFINITION(CVX_KEY)
};

struct VTAB_V
{
    CVX_VTAB_DEFINITION(CVX_VAL)
};

#ifdef CVX2_COLLISION_OPEN_ADDRESSING
// collision strategy: open addressing
struct ENTRY
{
    CVX_KEY key;
    CVX_VAL val;
    size_t dist; // displacement from home slot (robin hood)
    int state;   // internal bookkeeping, do not modify
};

struct CVX_SNAME
{
    size_t capacity;      // total buffer slots
    size_t count;         // number of filled entries
    double load;          // resize threshold (count >= capacity * load)
    struct VTAB_K *vtabk; // hash and comp are required for operations
    struct VTAB_V *vtabv;
    struct ENTRY *buffer;
};
#endif

#ifdef CVX2_COLLISION_SEPARATE_CHAINING
// collision strategy: separate chaining
struct NODE
{
    CVX_KEY key;
    CVX_VAL val;
    struct NODE *next;
};

struct CVX_SNAME
{
    size_t capacity;      // bucket count
    size_t count;         // number of stored entries
    double load;          // resize threshold (count >= capacity * load)
    struct VTAB_K *vtabk; // hash and comp are required for operations
    struct VTAB_V *vtabv;
    struct NODE **buckets;
};
#endif

/** @brief Initializes self, optionally pre-allocating capacity slots. */
enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv, size_t capacity);
/** @brief Initializes clone as a deep copy of orig. */
enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone);
/** @brief Frees the table, dropping any keys/values still stored. */
enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self);

/** @brief Number of entries currently stored. */
size_t FUNC(_count)(struct CVX_SNAME *self);
/** @brief Current number of buffer/bucket slots. */
size_t FUNC(_capacity)(struct CVX_SNAME *self);
/** @brief Resize threshold, as a fraction of capacity. */
double FUNC(_load)(struct CVX_SNAME *self);
/** @brief True if count is 0. */
bool FUNC(_empty)(struct CVX_SNAME *self);

/** @brief Inserts key/val. Fails with CVX_FLAG_DUPLICATE if key already exists. */
enum cvx_flags FUNC(_insert)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL val);
/** @brief Replaces the value for key, returning the old value through old_out. Fails with CVX_FLAG_NOT_FOUND if absent. */
enum cvx_flags FUNC(_update)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL new_val, CVX_VAL *old_out);
/** @brief Removes key, returning its value through out. Fails with CVX_FLAG_NOT_FOUND if absent. */
enum cvx_flags FUNC(_remove)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out);
/** @brief Gets the value for key. Fails with CVX_FLAG_NOT_FOUND if absent. */
enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out);
/** @brief Gets a pointer to the stored value for key, or NULL if absent. */
CVX_VAL *FUNC(_get_ref)(struct CVX_SNAME *self, CVX_KEY key);
/** @brief True if key is present. */
bool FUNC(_contains)(struct CVX_SNAME *self, CVX_KEY key);
