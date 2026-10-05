/**
 * @file hashtable.h
 * @brief cvx2 template: a configurable hashtable that maps CVX_KEY -> CVX_VALUE
 *
 * ## Required config fields
 * - **key_type**: key type (maps to CVX_KEY)
 * - **value_type**: value type (maps to CVX_VALUE)
 * - **struct_name**: name of the generated struct (maps to CVX_SNAME)
 * - **prefix**: prefix of all generated functions (maps to CVX_PFX)
 * - **tag**: a unique integer tag for the generated type (maps to CVX_TAG)
 *
 * ## Variant axes
 * - **collision** (required): "open_addressing" (robin-hood linear probing)
 *   or "separate_chaining" (singly-linked buckets).
 *
 * Both variants expose the same public API and struct field names
 * (`super`, `capacity`, `count`, `load`, `vtabk`, `vtabv`) -- only the
 * internal storage field (`buffer` vs `buckets`) and algorithms differ.
 * See the `@cvx2:variant axis="collision" ...` blocks below and in
 * hashtable.c.
 *
 * Keys are hashed with vtabk->hash and compared with vtabk->comp. Both are
 * required; vtabk->clone / vtabk->drop and vtabv->clone / vtabv->drop are
 * optional. A resize is triggered whenever `count >= capacity * load`
 * (default load = 0.7); capacity is always rounded up to a prime.
 */

#include "cvx2/fallback.h"

// clang-format off
#ifndef CVX_KEY
#error "hashtable.h requires CVX_KEY to be defined (the key type)"
#endif
#ifndef CVX_VALUE
#error "hashtable.h requires CVX_VALUE to be defined (the value type)"
#endif
#ifndef CVX_SNAME
#error "hashtable.h requires CVX_SNAME to be defined (the struct name)"
#endif
#ifndef CVX_PFX
#error "hashtable.h requires CVX_PFX to be defined (the function prefix)"
#endif
#ifndef CVX_TAG
#error "hashtable.h requires CVX_TAG to be defined (a unique integer tag)"
#endif
// clang-format on

#include <stdbool.h>
#include <stddef.h>

#include "cvx2/core.h"

#define FUNC(X) CVX2_(CVX_PFX, X)
#define VTAB_K CVX2_(CVX_SNAME, _vtabk)
#define VTAB_V CVX2_(CVX_SNAME, _vtabv)
#define ENTRY CVX2_(CVX_SNAME, _entry)
#define NODE CVX2_(CVX_SNAME, _node)

struct VTAB_K
{
    CVX2_VTAB_DEFINITION(CVX_KEY)
};

struct VTAB_V
{
    CVX2_VTAB_DEFINITION(CVX_VALUE)
};

// @cvx2:variant axis="collision" name="open_addressing"
#ifdef CVX2_COLLISION_OPEN_ADDRESSING
struct ENTRY
{
    CVX_KEY key;
    CVX_VALUE val;
    size_t dist; // displacement from home slot (robin hood)
    // `state` is one of the CVX2_HT_ENTRY_* constants defined in the .c --
    // not declared here since it's purely a private implementation detail,
    // and keeping it out of the header avoids an unqualified enum colliding
    // across two open_addressing instantiations included in one TU.
    int state;
};

struct CVX_SNAME
{
    cvx2_container super;
    size_t capacity;      // total buffer slots
    size_t count;         // number of filled entries
    double load;          // resize threshold (count >= capacity * load)
    struct VTAB_K *vtabk; // hash and comp are required for operations
    struct VTAB_V *vtabv;
    struct ENTRY *buffer;
};
#endif
// @cvx2:endvariant

// @cvx2:variant axis="collision" name="separate_chaining"
#ifdef CVX2_COLLISION_SEPARATE_CHAINING
struct NODE
{
    CVX_KEY key;
    CVX_VALUE val;
    struct NODE *next;
};

struct CVX_SNAME
{
    cvx2_container super;
    size_t capacity;      // bucket count
    size_t count;         // number of stored entries
    double load;          // resize threshold (count >= capacity * load)
    struct VTAB_K *vtabk; // hash and comp are required for operations
    struct VTAB_V *vtabv;
    struct NODE **buckets;
};
#endif
// @cvx2:endvariant

// ---- Initializers ----
enum cvx2_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv, size_t capacity);
enum cvx2_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone);

// ---- Destructor ----
enum cvx2_flags FUNC(_drop)(struct CVX_SNAME *self);

// ---- Getters (no failure mode beyond struct state, so these keep direct returns) ----
size_t FUNC(_count)(struct CVX_SNAME *self);
size_t FUNC(_capacity)(struct CVX_SNAME *self);
double FUNC(_load)(struct CVX_SNAME *self);
bool FUNC(_empty)(struct CVX_SNAME *self);

// ---- Operations ----
enum cvx2_flags FUNC(_insert)(struct CVX_SNAME *self, CVX_KEY key, CVX_VALUE val);
enum cvx2_flags FUNC(_update)(struct CVX_SNAME *self, CVX_KEY key, CVX_VALUE new_val, CVX_VALUE *old_out);
enum cvx2_flags FUNC(_remove)(struct CVX_SNAME *self, CVX_KEY key, CVX_VALUE *out);
enum cvx2_flags FUNC(_get)(struct CVX_SNAME *self, CVX_KEY key, CVX_VALUE *out);
CVX_VALUE *FUNC(_get_ref)(struct CVX_SNAME *self, CVX_KEY key);
bool FUNC(_contains)(struct CVX_SNAME *self, CVX_KEY key);
