#ifndef HAMT_H
#define HAMT_H

#include "cvx2/core.h"
#include "cvx2/flags.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file hamt.h
 * @author Leonardo Vencovsky
 * @brief Hash Array Mapped Trie
 * @version 0.0.1
 *
 */

struct HAMTVtabK
{
    CVX_VTAB_COMP(comp, void *);
    CVX_VTAB_COPY(copy, void *);
    CVX_VTAB_DROP(drop, void *);
    CVX_VTAB_HASH(hash, void *);
};

struct HAMTVtabV
{
    CVX_VTAB_COPY(copy, void *);
    CVX_VTAB_DROP(drop, void *);
};

struct HAMTEntry
{
    uint32_t hash;
    void *key;
    void *val;
    // TODO: *next pointer and remove COLLISION
};

struct HAMT_collision
{
    uint32_t hash;
    size_t count;
    struct HAMTNode **entries;
};

struct HAMTInternal
{
    uint32_t bitmap;
    struct HAMTNode **children;
};

struct HAMTNode
{
    size_t ref_count;
    enum cvx_hamt_node_t type;
    union
    {
        struct HAMTEntry leaf;
        struct HAMTInternal internal;
        struct HAMT_collision collision;
    } as;
};

struct HAMT
{
    size_t count;
    struct HAMTVtabK *vtabk;
    struct HAMTVtabV *vtabv;
    struct HAMTNode *root;
};

// Initializers and destructors
enum cvx_flags hamt_init(struct HAMT *self, struct HAMTVtabK *vtabk, struct HAMTVtabV *vtabv);
enum cvx_flags hamt_insert(struct HAMT *self, void *key, void *val);
enum cvx_flags hamt_get(struct HAMT *self, void *key, void **out);

#endif /* HAMT_H */
