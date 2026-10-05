#include "hamt.h"
#include "cvx2/core.h"
#include "cvx2/flags.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Not part of the public API.
enum cvx_flags hamt__make_subtree(struct HAMTNode *leaf1, struct HAMTNode *leaf2, size_t depth,
                                  struct HAMTNode **out);
/**
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation fails
 */
enum cvx_flags hamt__make_internal(size_t size, uint32_t bitmap, struct HAMTNode **out);
/**
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation fails
 */
enum cvx_flags hamt__make_collision(struct HAMTNode *leaf1, struct HAMTNode *leaf2, uint32_t hash,
                                    struct HAMTNode **out);
enum cvx_flags hamt__make_leaf(void *key, void *val, uint32_t hash, struct HAMTNode **out);

enum cvx_flags hamt_init(struct HAMT *self, struct HAMTVtabK *vtabk, struct HAMTVtabV *vtabv)
{
    *self = (struct HAMT){ 0 };

    if (!vtabk || !vtabk->hash || !vtabk->comp)
        return CVX_FLAG_VTAB;

    self->vtabk = vtabk;
    self->vtabv = vtabv;

    return CVX_FLAG_OK;
}

enum cvx_flags hamt_insert(struct HAMT *self, void *key, void *val)
{
    enum cvx_flags flag = CVX_FLAG_OK;
    struct HAMTNode *probe = self->root;

    size_t hash = self->vtabk->hash(key);

    for (size_t depth = 0; depth < ((32 + 5 - 1) / 5); depth++)
    {
        if (probe->type == CVX_HAMT_NODE_LEAF)
        {
            // TODO: duplicate or insert and replace?
            if (self->vtabk->comp(key, probe->as.leaf.key) == 0)
                return CVX_FLAG_DUPLICATE;
            else
            {
                struct HAMTNode *leaf1 = probe;
                struct HAMTNode *leaf2;
                flag = hamt__make_leaf(key, val, hash, &leaf2);
                if (flag != CVX_FLAG_OK)
                {
                    // TODO: handle
                }
                if (((32 + 5 - 1) / 5) - 1 == depth)
                {
                    // Hash collision
                }
                else
                {
                    // while (C1 == C2)
                    // Convert leaf node into internal node
                }
            }
        }

        if (probe->type == CVX_HAMT_NODE_INTERNAL)
        {
            uint32_t bitmap = probe->as.internal.bitmap;

            size_t chunk = (hash >> (depth * 5)) & ((1U << 5) - 1);
            uint32_t mask = (uint32_t)1 << chunk;

            size_t old_count = cvx_popcount(bitmap);
            size_t index = cvx_popcount(bitmap & (mask - 1));

            if ((bitmap & mask) == 0)
            {
                struct HAMTNode **new_children = realloc(
                    probe->as.internal.children, (old_count + 1) * sizeof(struct HAMTNode *));
                memmove(&new_children[index + 1], &new_children[index],
                        (old_count - index) * sizeof(struct HAMTNode *));

                new_children[index] = malloc(sizeof(struct HAMTNode));
                *new_children[index] = (struct HAMTNode){
                    .type = CVX_HAMT_NODE_LEAF,
                    .as.leaf = {
                        .hash = hash,
                        .key = key,
                        .val = val,
                    },
                };

                probe->as.internal.children = new_children;
                probe->as.internal.bitmap |= (1U << chunk);

                return CVX_FLAG_OK;
            }
            else
            {
                probe = probe->as.internal.children[index];
            }
        }

        if (probe->type == CVX_HAMT_NODE_COLLISION)
        {
            struct HAMT_collision slot = probe->as.collision;
            // TODO: push new entry into slot
        }

        return CVX_FLAG_DUPLICATE;
    }

    return CVX_FLAG_OK;
}

enum cvx_flags hamt_get(struct HAMT *self, void *key, void **out)
{
    if (self->count == 0 || !self->root)
        return CVX_FLAG_EMPTY;

    struct HAMTNode *probe = self->root;

    uint32_t hash = (uint32_t)self->vtabk->hash(key);

    for (size_t depth = 0; depth < ((32 + 5 - 1) / 5); depth++)
    {
        if (probe->type == CVX_HAMT_NODE_LEAF)
        {
            if (self->vtabk->comp(key, probe->as.leaf.key) == 0)
            {
                *out = probe->as.leaf.val;
                return CVX_FLAG_OK;
            }
            else
                return CVX_FLAG_NOT_FOUND;
        }

        if (probe->type == CVX_HAMT_NODE_INTERNAL)
        {
            uint32_t bitmap = probe->as.internal.bitmap;

            size_t chunk = (hash >> (depth * 5)) & ((1U << 5) - 1);
            uint32_t mask = (uint32_t)1 << chunk;

            if ((bitmap & mask) == 0)
                return CVX_FLAG_NOT_FOUND;

            size_t index = cvx_popcount(bitmap & (mask - 1));

            probe = probe->as.internal.children[index];
        }
    }

    return CVX_FLAG_NOT_FOUND;
}

enum cvx_flags hamt__make_subtree(struct HAMTNode *leaf1, struct HAMTNode *leaf2, size_t depth,
                                  struct HAMTNode **out)
{
    enum cvx_flags flag = CVX_FLAG_OK;
    uint32_t hash1 = leaf1->as.leaf.hash;
    uint32_t hash2 = leaf2->as.leaf.hash;

    if (depth >= ((32 + 5 - 1) / 5))
    {
        struct HAMTNode *collision;
        flag = hamt__make_collision(leaf1, leaf2, hash1, &collision);
        if (flag != CVX_FLAG_OK)
            return flag;

        *out = collision;
        return CVX_FLAG_OK;
    }

    uint32_t chunk1 = (hash1 >> (depth * 5)) & ((1U << 5) - 1);
    uint32_t chunk2 = (hash2 >> (depth * 5)) & ((1U << 5) - 1);

    if (chunk1 != chunk2)
    {
        struct HAMTNode *parent;
        flag = hamt__make_internal(2, (1U << chunk1) | (1U << chunk2), &parent);
        if (flag != CVX_FLAG_OK)
            return flag;

        if (chunk1 < chunk2)
        {
            parent->as.internal.children[0] = leaf1;
            parent->as.internal.children[1] = leaf2;
        }
        else
        {
            parent->as.internal.children[0] = leaf2;
            parent->as.internal.children[1] = leaf1;
        }

        *out = parent;
    }
    else
    {
        struct HAMTNode *parent;
        flag = hamt__make_internal(1, (1U << chunk1), &parent);
        if (flag != CVX_FLAG_OK)
            return flag;

        struct HAMTNode *subtree;
        flag = hamt__make_subtree(leaf1, leaf2, depth + 1, &subtree);
        if (flag != CVX_FLAG_OK)
            return flag;

        parent->as.internal.children[0] = subtree;

        *out = parent;
    }

    return CVX_FLAG_OK;
}

enum cvx_flags hamt__make_internal(size_t size, uint32_t bitmap, struct HAMTNode **out)
{
    struct HAMTNode *result = malloc(sizeof(struct HAMTNode));
    if (!result)
        return CVX_FLAG_ALLOC;

    result->type = CVX_HAMT_NODE_INTERNAL;
    result->as.internal.bitmap = bitmap;

    if (size > 0)
    {
        struct HAMTNode **children = malloc(sizeof(struct HAMTNode *) * size);
        if (!children)
        {
            free(result);
            return CVX_FLAG_ALLOC;
        }

        result->as.internal.children = children;
    }
    else
        result->as.internal.children = NULL;

    *out = result;
    return CVX_FLAG_OK;
}

enum cvx_flags hamt__make_collision(struct HAMTNode *leaf1, struct HAMTNode *leaf2, uint32_t hash,
                                    struct HAMTNode **out)
{
    struct HAMTNode *result = malloc(sizeof(struct HAMTNode));
    if (!result)
        return CVX_FLAG_ALLOC;

    result->type = CVX_HAMT_NODE_COLLISION;

    result->as.collision.count = 2;
    result->as.collision.hash = hash;

    struct HAMTNode **entries = malloc(sizeof(struct HAMTNode) * 2);
    if (!entries)
        return CVX_FLAG_ALLOC;

    entries[0] = leaf1;
    entries[1] = leaf2;

    result->as.collision.entries = entries;

    *out = result;
    return CVX_FLAG_OK;
}

enum cvx_flags hamt__make_leaf(void *key, void *val, uint32_t hash, struct HAMTNode **out)
{
    struct HAMTNode *result = malloc(sizeof(struct HAMTNode));
    if (!result)
        return CVX_FLAG_ALLOC;

    result->type = CVX_HAMT_NODE_LEAF;
    result->as.leaf.key = key;
    result->as.leaf.val = val;
    result->as.leaf.hash = hash;

    *out = result;
    return CVX_FLAG_OK;
}

void hamt__free_internal(struct HAMTNode *node)
{
    if (node->type != CVX_HAMT_NODE_INTERNAL)
        return;
}
