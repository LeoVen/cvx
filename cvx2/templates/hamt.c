#include "hamt.h"
#include "cvx2/core.h"
#include "cvx2/flags.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Not part of the public API.
enum cvx_flags FUNC(__make_subtree)(struct NODE *leaf1, struct NODE *leaf2, size_t depth,
                                    struct NODE **out);
/**
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation fails
 */
enum cvx_flags FUNC(__make_internal)(size_t size, CVX_HAMT_BITMAP_T bitmap, struct NODE **out);
/**
 * Error Handling
 * - CVX_FLAG_ALLOC: if allocation fails
 */
enum cvx_flags FUNC(__make_collision)(struct NODE *leaf1, struct NODE *leaf2, CVX_HAMT_HASH_T hash,
                                      struct NODE **out);
enum cvx_flags FUNC(__make_leaf)(CVX_KEY key, CVX_VAL val, CVX_HAMT_HASH_T hash, struct NODE **out);

enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv)
{
    *self = (struct CVX_SNAME){ 0 };

    if (!vtabk || !vtabk->hash || !vtabk->comp)
        return CVX_FLAG_VTAB;

    self->vtabk = vtabk;
    self->vtabv = vtabv;

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_insert)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL val)
{
    enum cvx_flags flag = CVX_FLAG_OK;
    struct NODE *probe = self->root;

    size_t hash = self->vtabk->hash(key);

    for (size_t depth = 0; depth < CVX_HAMT_DEPTH; depth++)
    {
        if (probe->type == CVX_HAMT_NODE_LEAF)
        {
            // TODO: duplicate or insert and replace?
            if (self->vtabk->comp(key, probe->as.leaf.key) == 0)
                return CVX_FLAG_DUPLICATE;
            else
            {
                struct NODE *leaf1 = probe;
                struct NODE *leaf2;
                flag = FUNC(__make_leaf)(key, val, hash, &leaf2);
                if (flag != CVX_FLAG_OK)
                {
                    // TODO: handle
                }
                if (CVX_HAMT_DEPTH - 1 == depth)
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
            CVX_HAMT_BITMAP_T bitmap = probe->as.internal.bitmap;

            size_t chunk = (hash >> (depth * CVX_HAMT_CHUNK_BITS)) & CVX_HAMT_CHUNK_MASK;
            CVX_HAMT_BITMAP_T mask = (CVX_HAMT_BITMAP_T)1 << chunk;

            size_t old_count = cvx_popcount(bitmap);
            size_t index = cvx_popcount(bitmap & (mask - 1));

            if ((bitmap & mask) == 0)
            {
                struct NODE **new_children =
                    realloc(probe->as.internal.children, (old_count + 1) * sizeof(struct NODE *));
                memmove(&new_children[index + 1], &new_children[index],
                        (old_count - index) * sizeof(struct NODE *));

                new_children[index] = malloc(sizeof(struct NODE));
                *new_children[index] = (struct NODE){
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
            struct COLLISION slot = probe->as.collision;
            // TODO: push new entry into slot
        }

        return CVX_FLAG_DUPLICATE;
    }

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out)
{
    if (self->count == 0 || !self->root)
        return CVX_FLAG_EMPTY;

    struct NODE *probe = self->root;

    CVX_HAMT_HASH_T hash = (CVX_HAMT_HASH_T)self->vtabk->hash(key);

    for (size_t depth = 0; depth < CVX_HAMT_DEPTH; depth++)
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
            CVX_HAMT_BITMAP_T bitmap = probe->as.internal.bitmap;

            size_t chunk = (hash >> (depth * CVX_HAMT_CHUNK_BITS)) & CVX_HAMT_CHUNK_MASK;
            CVX_HAMT_BITMAP_T mask = (CVX_HAMT_BITMAP_T)1 << chunk;

            if ((bitmap & mask) == 0)
                return CVX_FLAG_NOT_FOUND;

            size_t index = cvx_popcount(bitmap & (mask - 1));

            probe = probe->as.internal.children[index];
        }
    }

    return CVX_FLAG_NOT_FOUND;
}

enum cvx_flags FUNC(__make_subtree)(struct NODE *leaf1, struct NODE *leaf2, size_t depth,
                                    struct NODE **out)
{
    enum cvx_flags flag = CVX_FLAG_OK;
    CVX_HAMT_HASH_T hash1 = leaf1->as.leaf.hash;
    CVX_HAMT_HASH_T hash2 = leaf2->as.leaf.hash;

    if (depth >= CVX_HAMT_DEPTH)
    {
        struct NODE *collision;
        flag = FUNC(__make_collision)(leaf1, leaf2, hash1, &collision);
        if (flag != CVX_FLAG_OK)
            return flag;

        *out = collision;
        return CVX_FLAG_OK;
    }

    uint32_t chunk1 = (hash1 >> (depth * CVX_HAMT_CHUNK_BITS)) & CVX_HAMT_CHUNK_MASK;
    uint32_t chunk2 = (hash2 >> (depth * CVX_HAMT_CHUNK_BITS)) & CVX_HAMT_CHUNK_MASK;

    if (chunk1 != chunk2)
    {
        struct NODE *parent;
        flag = FUNC(__make_internal)(2, (1U << chunk1) | (1U << chunk2), &parent);
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
        struct NODE *parent;
        flag = FUNC(__make_internal)(1, (1U << chunk1), &parent);
        if (flag != CVX_FLAG_OK)
            return flag;

        struct NODE *subtree;
        flag = FUNC(__make_subtree)(leaf1, leaf2, depth + 1, &subtree);
        if (flag != CVX_FLAG_OK)
            return flag;

        parent->as.internal.children[0] = subtree;

        *out = parent;
    }

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(__make_internal)(size_t size, CVX_HAMT_BITMAP_T bitmap, struct NODE **out)
{
    struct NODE *result = malloc(sizeof(struct NODE));
    if (!result)
        return CVX_FLAG_ALLOC;

    result->type = CVX_HAMT_NODE_INTERNAL;
    result->as.internal.bitmap = bitmap;

    if (size > 0)
    {
        struct NODE **children = malloc(sizeof(struct NODE *) * size);
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

enum cvx_flags FUNC(__make_collision)(struct NODE *leaf1, struct NODE *leaf2, CVX_HAMT_HASH_T hash,
                                      struct NODE **out)
{
    struct NODE *result = malloc(sizeof(struct NODE));
    if (!result)
        return CVX_FLAG_ALLOC;

    result->type = CVX_HAMT_NODE_COLLISION;

    result->as.collision.count = 2;
    result->as.collision.hash = hash;

    struct NODE **entries = malloc(sizeof(struct NODE) * 2);
    if (!entries)
        return CVX_FLAG_ALLOC;

    entries[0] = leaf1;
    entries[1] = leaf2;

    result->as.collision.entries = entries;

    *out = result;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(__make_leaf)(CVX_KEY key, CVX_VAL val, CVX_HAMT_HASH_T hash, struct NODE **out)
{
    struct NODE *result = malloc(sizeof(struct NODE));
    if (!result)
        return CVX_FLAG_ALLOC;

    result->type = CVX_HAMT_NODE_LEAF;
    result->as.leaf.key = key;
    result->as.leaf.val = val;
    result->as.leaf.hash = hash;

    *out = result;
    return CVX_FLAG_OK;
}

void FUNC(__free_internal)(struct NODE *node)
{
    if (node->type != CVX_HAMT_NODE_INTERNAL)
        return;
}
