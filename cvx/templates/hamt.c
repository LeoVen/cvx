#include "hamt.h"

// Not part of the public API.
// clang-format off
enum cvx_flags FUNC(__make_subtree)(struct NODE *leaf1, struct NODE *leaf2, size_t depth, struct NODE **out);
enum cvx_flags FUNC(__make_node_internal)(size_t size, CVX_HAMT_BITMAP_T bitmap, struct NODE **out);
enum cvx_flags FUNC(__make_node_leaf)(CVX_KEY key, CVX_VAL val, CVX_HAMT_HASH_T hash, struct NODE **out);
enum cvx_flags FUNC(__make_entry)(CVX_KEY key, CVX_VAL val, CVX_HAMT_HASH_T hash, struct ENTRY **out);
bool FUNC(__has_duplicate_chained)(struct CVX_SNAME *self, CVX_KEY *key, struct ENTRY *entry);
void FUNC(__drop_node)(struct CVX_SNAME *self, struct NODE **root);
void FUNC(__drop_node_leaf)(struct CVX_SNAME *self, struct NODE **leaf);
void FUNC(__drop_entry)(struct CVX_SNAME *self, struct ENTRY **entry);
// clang-format on

enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv)
{
    *self = (struct CVX_SNAME){ 0 };

    if (!vtabk || !vtabk->hash || !vtabk->comp)
        return CVX_FLAG_VTAB;

    self->vtabk = vtabk;
    self->vtabv = vtabv;

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self)
{
    if (!self)
        return CVX_FLAG_OK;

    FUNC(__drop_node)(self, &self->root);
    self->count = 0;

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone)
{
    // TODO:
    // This is going to be tricky. If allocation fails mid-cloning, I'll have to
    // cleanup the tree and hope that it isn't in some intermediate state.
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_insert)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL val)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return CVX_FLAG_VTAB;

    enum cvx_flags flag = CVX_FLAG_OK;
    struct NODE **slot = &self->root;
    struct NODE *probe = *slot;

    CVX_HAMT_HASH_T hash = (CVX_HAMT_HASH_T)self->vtabk->hash(&key);

    if (!probe)
    {
        flag = FUNC(__make_node_leaf)(key, val, hash, slot);
        if (flag != CVX_FLAG_OK)
            return flag;

        self->count++;
        return CVX_FLAG_OK;
    }

    for (size_t depth = 0; depth < CVX_HAMT_DEPTH; depth++)
    {
        if (probe->type == CVX_HAMT_NODE_LEAF)
        {
            if (FUNC(__has_duplicate_chained)(self, &key, &probe->as.leaf))
            {
                // TODO: duplicate or insert and replace?
                return CVX_FLAG_DUPLICATE;
            }
            else
            {
                if (probe->as.leaf.hash == hash)
                {
                    // Early chain on exact match
                    struct ENTRY *entry;
                    flag = FUNC(__make_entry)(key, val, hash, &entry);
                    if (flag != CVX_FLAG_OK)
                        return flag;

                    struct ENTRY *entry_probe = &probe->as.leaf;
                    while (entry_probe->next != NULL)
                        entry_probe = entry_probe->next;

                    entry_probe->next = entry;
                    break;
                }

                // Split
                struct NODE *leaf1 = probe;
                struct NODE *leaf2;
                flag = FUNC(__make_node_leaf)(key, val, hash, &leaf2);
                if (flag != CVX_FLAG_OK)
                    return flag;

                flag = FUNC(__make_subtree)(leaf1, leaf2, depth, slot);
                if (flag != CVX_FLAG_OK)
                {
                    free(leaf2);
                    return flag;
                }
                break;
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
                struct NODE *new_leaf;
                flag = FUNC(__make_node_leaf)(key, val, hash, &new_leaf);
                if (flag != CVX_FLAG_OK)
                    return flag;

                struct NODE **new_children =
                    realloc(probe->as.internal.children, (old_count + 1) * sizeof(struct NODE *));
                if (new_children == NULL)
                {
                    free(new_leaf);
                    return CVX_FLAG_ALLOC;
                }

                memmove(&new_children[index + 1], &new_children[index],
                        (old_count - index) * sizeof(struct NODE *));
                new_children[index] = new_leaf;
                probe->as.internal.children = new_children;
                probe->as.internal.bitmap |= mask;

                break;
            }
            else
            {
                slot = &probe->as.internal.children[index];
                probe = *slot;
            }
        }
    }

    self->count++;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out)
{
    if (self->count == 0 || !self->root)
        return CVX_FLAG_EMPTY;

    struct NODE *probe = self->root;

    CVX_HAMT_HASH_T hash = (CVX_HAMT_HASH_T)self->vtabk->hash(&key);

    for (size_t depth = 0; depth < CVX_HAMT_DEPTH; depth++)
    {
        if (probe->type == CVX_HAMT_NODE_LEAF)
        {
            struct ENTRY *entry_probe = &probe->as.leaf;
            while (entry_probe)
            {
                if (self->vtabk->comp(&key, &entry_probe->key) == 0)
                {
                    if (out)
                        *out = entry_probe->val;

                    return CVX_FLAG_OK;
                }
                entry_probe = entry_probe->next;
            }

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

    uint32_t chunk1 = (hash1 >> (depth * CVX_HAMT_CHUNK_BITS)) & CVX_HAMT_CHUNK_MASK;
    uint32_t chunk2 = (hash2 >> (depth * CVX_HAMT_CHUNK_BITS)) & CVX_HAMT_CHUNK_MASK;

    CVX_HAMT_BITMAP_T mask1 = (CVX_HAMT_BITMAP_T)1 << chunk1;
    CVX_HAMT_BITMAP_T mask2 = (CVX_HAMT_BITMAP_T)1 << chunk2;

    if (chunk1 != chunk2)
    {
        struct NODE *parent;
        flag = FUNC(__make_node_internal)(2, mask1 | mask2, &parent);
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
        flag = FUNC(__make_node_internal)(1, mask1, &parent);
        if (flag != CVX_FLAG_OK)
            return flag;

        struct NODE *subtree;
        flag = FUNC(__make_subtree)(leaf1, leaf2, depth + 1, &subtree);
        if (flag != CVX_FLAG_OK)
        {
            free(parent->as.internal.children);
            free(parent);
            return flag;
        }

        parent->as.internal.children[0] = subtree;

        *out = parent;
    }

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(__make_node_internal)(size_t size, CVX_HAMT_BITMAP_T bitmap, struct NODE **out)
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

enum cvx_flags FUNC(__make_node_leaf)(CVX_KEY key, CVX_VAL val, CVX_HAMT_HASH_T hash,
                                      struct NODE **out)
{
    struct NODE *result = malloc(sizeof(struct NODE));
    if (!result)
        return CVX_FLAG_ALLOC;

    result->type = CVX_HAMT_NODE_LEAF;
    result->as.leaf.key = key;
    result->as.leaf.val = val;
    result->as.leaf.hash = hash;
    result->as.leaf.next = NULL;

    *out = result;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(__make_entry)(CVX_KEY key, CVX_VAL val, CVX_HAMT_HASH_T hash,
                                  struct ENTRY **out)
{
    struct ENTRY *result = malloc(sizeof(struct ENTRY));
    if (!result)
        return CVX_FLAG_ALLOC;

    result->hash = hash;
    result->key = key;
    result->val = val;
    result->next = NULL;

    *out = result;
    return CVX_FLAG_OK;
}

bool FUNC(__has_duplicate_chained)(struct CVX_SNAME *self, CVX_KEY *key, struct ENTRY *entry)
{
    struct ENTRY *probe = entry;
    while (probe)
    {
        if (self->vtabk->comp(key, &probe->key) == 0)
            return true;

        probe = probe->next;
    }

    return false;
}

void FUNC(__drop_node)(struct CVX_SNAME *self, struct NODE **root)
{
    if (*root == NULL)
        return;

    if ((*root)->type == CVX_HAMT_NODE_LEAF)
    {
        FUNC(__drop_node_leaf)(self, root);
    }
    else if ((*root)->type == CVX_HAMT_NODE_INTERNAL)
    {
        struct INTERNAL *node = &(*root)->as.internal;
        size_t count = cvx_popcount(node->bitmap);

        for (size_t i = 0; i < count; i++)
        {
            FUNC(__drop_node)(self, &node->children[i]);
        }

        free((*root)->as.internal.children);
        free(*root);
        *root = NULL;
    }
}

void FUNC(__drop_node_leaf)(struct CVX_SNAME *self, struct NODE **leaf)
{
    struct ENTRY *curr = (*leaf)->as.leaf.next;
    while (curr)
    {
        struct ENTRY *next = curr->next;
        FUNC(__drop_entry)(self, &curr);
        curr = next;
    }

    if (self->vtabk && self->vtabk->drop)
        self->vtabk->drop(&(*leaf)->as.leaf.key);
    if (self->vtabv && self->vtabv->drop)
        self->vtabv->drop(&(*leaf)->as.leaf.val);

    free(*leaf);
    *leaf = NULL;
}

void FUNC(__drop_entry)(struct CVX_SNAME *self, struct ENTRY **entry)
{

    if (self->vtabk && self->vtabk->drop)
        self->vtabk->drop(&(*entry)->key);
    if (self->vtabv && self->vtabv->drop)
        self->vtabv->drop(&(*entry)->val);
    free(*entry);
    *entry = NULL;
}
