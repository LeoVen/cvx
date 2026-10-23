/**
 * @file hamt.h
 * @author Leonardo Vencovsky
 * @brief Hash Array Mapped Trie
 * @version 0.0.1
 *
 */

#include "cvx2/fallback.h"

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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cvx2/core.h"
#include "cvx2/flags.h"

#define FUNC(X) CVX_(CVX_PFX, X)
#define VTAB_K CVX_(CVX_SNAME, _vtabk)
#define VTAB_V CVX_(CVX_SNAME, _vtabv)
#define NODE CVX_(CVX_SNAME, _node)
#define ENTRY CVX_(CVX_SNAME, _entry)
#define COLLISION CVX_(CVX_SNAME, _collision)
#define INTERNAL CVX_(CVX_SNAME, _internal)

// clang-format off
#if defined(CVX_HAMT_BITMAP_8) + defined(CVX_HAMT_BITMAP_16) + defined(CVX_HAMT_BITMAP_32) + defined(CVX_HAMT_BITMAP_64) > 1
#error "hamt.h: define at most one of CVX_HAMT_BITMAP_8, CVX_HAMT_BITMAP_16, CVX_HAMT_BITMAP_32, CVX_HAMT_BITMAP_64"
#endif
#if defined(CVX_HAMT_HASH_32) + defined(CVX_HAMT_HASH_64) > 1
#error "hamt.h: define at most one of CVX_HAMT_HASH_32, CVX_HAMT_HASH_64"
#endif
// clang-format on

// Bitmap width: one bit per child slot, so each level consumes log2(width)
// bits of the hash. Defaults to 32 (5 bits per level).
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

// Hash width: how many bits of vtabk->hash's result are used (the rest is
// truncated). Defaults to 32.
#if defined(CVX_HAMT_HASH_64)
#define CVX_HAMT_HASH_T uint64_t
#define CVX_HAMT_HASH_BITS 64
#else
#define CVX_HAMT_HASH_T uint32_t
#define CVX_HAMT_HASH_BITS 32
#endif

// Selects a level's chunk once the hash has been shifted down to it
#define CVX_HAMT_CHUNK_MASK ((1U << CVX_HAMT_CHUNK_BITS) - 1)
// Levels needed to consume every hash bit (the last chunk may be partial)
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

#ifndef CVX_HAMT_NODE_T
#define CVX_HAMT_NODE_T

enum cvx_hamt_node_t
{
    CVX_HAMT_NODE_LEAF,
    CVX_HAMT_NODE_INTERNAL,
    CVX_HAMT_NODE_COLLISION,
};

#endif

struct ENTRY
{
    CVX_HAMT_HASH_T hash;
    CVX_KEY key;
    CVX_VAL val;
    // TODO: *next pointer and remove COLLISION
};

struct COLLISION
{
    uint32_t hash;
    size_t count;
    struct NODE **entries;
};

struct INTERNAL
{
    CVX_HAMT_BITMAP_T bitmap;
    struct NODE **children;
};

struct NODE
{
    size_t ref_count;
    enum cvx_hamt_node_t type;
    union
    {
        struct ENTRY leaf;
        struct INTERNAL internal;
        struct COLLISION collision;
    } as;
};

struct CVX_SNAME
{
    size_t count;
    struct VTAB_K *vtabk;
    struct VTAB_V *vtabv;
    struct NODE *root;
};

// Initializers and destructors
enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv);
enum cvx_flags FUNC(_insert)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL val);
enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out);
