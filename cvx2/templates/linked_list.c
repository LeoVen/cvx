#include "linked_list.h"

#include <stdlib.h>

// Not part of the public API.
static struct NODE *FUNC(__node_at)(struct CVX_SNAME *self, size_t index);
static enum cvx_flags FUNC(__link_after)(struct CVX_SNAME *self, struct NODE *prev, CVX_VAL item);
static void FUNC(__unlink)(struct CVX_SNAME *self, struct NODE *prev, struct NODE *node,
                           CVX_VAL *out);
static void FUNC(__free_from)(struct CVX_SNAME *self, struct NODE *node);
static struct NODE *FUNC(__merge_sort)(struct CVX_SNAME *self, struct NODE *head, size_t count);

enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_V *vtabv)
{
    *self = (struct CVX_SNAME){ 0 };
    self->vtabv = vtabv;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self)
{
    if (!self)
        return CVX_FLAG_OK;

    FUNC(__free_from)(self, self->head);
    self->head = NULL;
    self->tail = NULL;
    self->count = 0;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone)
{
    FUNC(_init)(clone, orig->vtabv);

    for (struct NODE *node = orig->head; node; node = node->next)
    {
        CVX_VAL value = node->value;
        bool deep = clone->vtabv && clone->vtabv->copy;

        if (deep)
        {
            enum cvx_flags flag = clone->vtabv->copy(&node->value, &value);
            if (flag != CVX_FLAG_OK)
            {
                FUNC(_drop)(clone);
                return flag;
            }
        }

        if (FUNC(__link_after)(clone, clone->tail, value) != CVX_FLAG_OK)
        {
            if (deep && clone->vtabv->drop)
                clone->vtabv->drop(&value);
            FUNC(_drop)(clone);
            return CVX_FLAG_ALLOC;
        }
    }

    return CVX_FLAG_OK;
}

size_t FUNC(_count)(struct CVX_SNAME *self)
{
    return self->count;
}

bool FUNC(_empty)(struct CVX_SNAME *self)
{
    return self->count == 0;
}

enum cvx_flags FUNC(_front)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    if (out)
        *out = self->head->value;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_back)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    if (out)
        *out = self->tail->value;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, size_t index, CVX_VAL *out)
{
    if (index >= self->count)
        return CVX_FLAG_RANGE;

    if (out)
        *out = FUNC(__node_at)(self, index)->value;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_replace)(struct CVX_SNAME *self, size_t index, CVX_VAL new_value,
                              CVX_VAL *old_out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;
    if (index >= self->count)
        return CVX_FLAG_RANGE;

    struct NODE *node = FUNC(__node_at)(self, index);

    if (old_out)
        *old_out = node->value;
    else if (self->vtabv && self->vtabv->drop)
        self->vtabv->drop(&node->value);

    node->value = new_value;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_push)(struct CVX_SNAME *self, size_t index, CVX_VAL item)
{
    if (index > self->count)
        return CVX_FLAG_RANGE;

    struct NODE *prev = index == 0 ? NULL : FUNC(__node_at)(self, index - 1);
    return FUNC(__link_after)(self, prev, item);
}

enum cvx_flags FUNC(_pop)(struct CVX_SNAME *self, size_t index, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;
    if (index >= self->count)
        return CVX_FLAG_RANGE;

#ifdef CVX2_NODE_DOUBLE
    return FUNC(_remove)(self, FUNC(__node_at)(self, index), out);
#else
    struct NODE *prev = index == 0 ? NULL : FUNC(__node_at)(self, index - 1);
    return FUNC(_remove_after)(self, prev, out);
#endif
}

enum cvx_flags FUNC(_truncate)(struct CVX_SNAME *self, size_t new_count)
{
    if (new_count >= self->count)
        return CVX_FLAG_OK;

    if (new_count == 0)
        return FUNC(_drop)(self);

    struct NODE *last = FUNC(__node_at)(self, new_count - 1);
    FUNC(__free_from)(self, last->next);
    last->next = NULL;
    self->tail = last;
    self->count = new_count;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_index_of)(struct CVX_SNAME *self, CVX_VAL item, size_t *out)
{
    if (!self->vtabv || !self->vtabv->comp)
        return CVX_FLAG_VTAB;

    size_t index = 0;
    for (struct NODE *node = self->head; node; node = node->next, index++)
    {
        if (self->vtabv->comp(&node->value, &item) == 0)
        {
            if (out)
                *out = index;
            return CVX_FLAG_OK;
        }
    }

    return CVX_FLAG_NOT_FOUND;
}

enum cvx_flags FUNC(_find)(struct CVX_SNAME *self, CVX_VAL item, struct NODE **out)
{
    if (!self->vtabv || !self->vtabv->comp)
        return CVX_FLAG_VTAB;

    for (struct NODE *node = self->head; node; node = node->next)
    {
        if (self->vtabv->comp(&node->value, &item) == 0)
        {
            if (out)
                *out = node;
            return CVX_FLAG_OK;
        }
    }

    return CVX_FLAG_NOT_FOUND;
}

enum cvx_flags FUNC(_insert_after)(struct CVX_SNAME *self, struct NODE *node, CVX_VAL item)
{
    return FUNC(__link_after)(self, node, item);
}

#ifdef CVX2_NODE_SINGLE
enum cvx_flags FUNC(_remove_after)(struct CVX_SNAME *self, struct NODE *node, CVX_VAL *out)
{
    if (!node && self->count == 0)
        return CVX_FLAG_EMPTY;

    struct NODE *removed = node ? node->next : self->head;
    if (!removed)
        return CVX_FLAG_RANGE;

    FUNC(__unlink)(self, node, removed, out);
    return CVX_FLAG_OK;
}
#endif

#ifdef CVX2_NODE_DOUBLE
enum cvx_flags FUNC(_insert_before)(struct CVX_SNAME *self, struct NODE *node, CVX_VAL item)
{
    return FUNC(__link_after)(self, node ? node->prev : self->tail, item);
}

enum cvx_flags FUNC(_remove)(struct CVX_SNAME *self, struct NODE *node, CVX_VAL *out)
{
    FUNC(__unlink)(self, node->prev, node, out);
    return CVX_FLAG_OK;
}
#endif

enum cvx_flags FUNC(_splice)(struct CVX_SNAME *self, size_t index, struct CVX_SNAME *other)
{
    if (other == self)
        return CVX_FLAG_INVALID;
    if (index > self->count)
        return CVX_FLAG_RANGE;
    if (other->count == 0)
        return CVX_FLAG_OK;

    struct NODE *prev = index == 0 ? NULL : FUNC(__node_at)(self, index - 1);
    struct NODE *next = prev ? prev->next : self->head;

    if (prev)
        prev->next = other->head;
    else
        self->head = other->head;

    other->tail->next = next;
    if (!next)
        self->tail = other->tail;

#ifdef CVX2_NODE_DOUBLE
    other->head->prev = prev;
    if (next)
        next->prev = other->tail;
#endif

    self->count += other->count;
    other->head = NULL;
    other->tail = NULL;
    other->count = 0;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_reverse)(struct CVX_SNAME *self)
{
    struct NODE *prev = NULL;
    struct NODE *node = self->head;
    self->tail = self->head;

    while (node)
    {
        struct NODE *next = node->next;
        node->next = prev;
#ifdef CVX2_NODE_DOUBLE
        node->prev = next;
#endif
        prev = node;
        node = next;
    }

    self->head = prev;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_sort)(struct CVX_SNAME *self)
{
    if (!self->vtabv || !self->vtabv->comp)
        return CVX_FLAG_VTAB;

    self->head = FUNC(__merge_sort)(self, self->head, self->count);

    // The merge only maintains next, so tail (and prev) are rebuilt here.
    struct NODE *prev = NULL;
    for (struct NODE *node = self->head; node; node = node->next)
    {
#ifdef CVX2_NODE_DOUBLE
        node->prev = prev;
#endif
        prev = node;
    }
    self->tail = prev;

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_compare)(struct CVX_SNAME *left, struct CVX_SNAME *right, int *out)
{
    CVX_VTAB_COMP(cmp_func, CVX_VAL) = NULL;

    if (left->vtabv && left->vtabv->comp)
        cmp_func = left->vtabv->comp;
    else if (right->vtabv && right->vtabv->comp)
        cmp_func = right->vtabv->comp;
    else
        return CVX_FLAG_VTAB;

    struct NODE *l = left->head;
    struct NODE *r = right->head;
    for (; l && r; l = l->next, r = r->next)
    {
        int cmp = cmp_func(&l->value, &r->value);
        if (cmp != 0)
        {
            if (out)
                *out = cmp;
            return CVX_FLAG_OK;
        }
    }

    if (out)
        *out = (int)left->count - (int)right->count;
    return CVX_FLAG_OK;
}

static struct NODE *FUNC(__node_at)(struct CVX_SNAME *self, size_t index)
{
    if (index == self->count - 1)
        return self->tail;

#ifdef CVX2_NODE_DOUBLE
    if (index > self->count / 2)
    {
        struct NODE *node = self->tail;
        for (size_t i = self->count - 1; i > index; i--)
            node = node->prev;
        return node;
    }
#endif

    struct NODE *node = self->head;
    for (size_t i = 0; i < index; i++)
        node = node->next;
    return node;
}

// prev is NULL to link at the front.
static enum cvx_flags FUNC(__link_after)(struct CVX_SNAME *self, struct NODE *prev, CVX_VAL item)
{
    struct NODE *node = malloc(sizeof(struct NODE));
    if (!node)
        return CVX_FLAG_ALLOC;

    struct NODE *next = prev ? prev->next : self->head;
    node->value = item;
    node->next = next;

    if (prev)
        prev->next = node;
    else
        self->head = node;

    if (!next)
        self->tail = node;

#ifdef CVX2_NODE_DOUBLE
    node->prev = prev;
    if (next)
        next->prev = node;
#endif

    self->count++;
    return CVX_FLAG_OK;
}

// prev is node's predecessor, or NULL when node is the head.
static void FUNC(__unlink)(struct CVX_SNAME *self, struct NODE *prev, struct NODE *node,
                           CVX_VAL *out)
{
    struct NODE *next = node->next;

    if (prev)
        prev->next = next;
    else
        self->head = next;

    if (!next)
        self->tail = prev;

#ifdef CVX2_NODE_DOUBLE
    if (next)
        next->prev = prev;
#endif

    if (out)
        *out = node->value;
    else if (self->vtabv && self->vtabv->drop)
        self->vtabv->drop(&node->value);

    free(node);
    self->count--;
}

// Frees node and every node after it, without fixing up self's links.
static void FUNC(__free_from)(struct CVX_SNAME *self, struct NODE *node)
{
    while (node)
    {
        struct NODE *next = node->next;
        if (self->vtabv && self->vtabv->drop)
            self->vtabv->drop(&node->value);
        free(node);
        node = next;
    }
}

// Sorts the count nodes starting at head by their next links, returning the new head.
static struct NODE *FUNC(__merge_sort)(struct CVX_SNAME *self, struct NODE *head, size_t count)
{
    if (count <= 1)
        return head;

    size_t left_count = count / 2;
    struct NODE *left_last = head;
    for (size_t i = 1; i < left_count; i++)
        left_last = left_last->next;

    struct NODE *right = left_last->next;
    left_last->next = NULL;

    struct NODE *left = FUNC(__merge_sort)(self, head, left_count);
    right = FUNC(__merge_sort)(self, right, count - left_count);

    struct NODE *merged = NULL;
    struct NODE **link = &merged;
    while (left && right)
    {
        // Taking from the left on ties keeps the sort stable.
        struct NODE **smaller = self->vtabv->comp(&right->value, &left->value) < 0 ? &right : &left;
        *link = *smaller;
        link = &(*smaller)->next;
        *smaller = (*smaller)->next;
    }
    *link = left ? left : right;

    return merged;
}
