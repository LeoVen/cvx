#include "segment_tree.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Not part of the public API.
static void FUNC(__pull)(struct CVX_SNAME *self, size_t node);
static bool FUNC(__is_power_of_two)(size_t value);

enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, CVX_VTAB_COMBINE(combine, CVX_VAL),
                           CVX_VAL identity, size_t count)
{
    *self = (struct CVX_SNAME){ .combine = combine, .identity = identity };

    if (!combine)
        return CVX_FLAG_VTAB;

    size_t max_leaves = SIZE_MAX / 2 / sizeof(CVX_VAL);
    size_t leaves = 1;
    while (leaves < count)
    {
        if (leaves > max_leaves / 2)
            return CVX_FLAG_ALLOC;
        leaves *= 2;
    }

    self->tree = malloc(2 * leaves * sizeof(CVX_VAL));
    if (!self->tree)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < 2 * leaves; i++)
        self->tree[i] = identity;

    self->count = count;
    self->leaves = leaves;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_build)(struct CVX_SNAME *self, CVX_VTAB_COMBINE(combine, CVX_VAL),
                            CVX_VAL identity, CVX_VAL const *values, size_t count)
{
    enum cvx_flags flag = FUNC(_init)(self, combine, identity, count);
    if (flag != CVX_FLAG_OK)
        return flag;

    if (count == 0)
        return CVX_FLAG_OK;

    memcpy(self->tree + self->leaves, values, count * sizeof(CVX_VAL));
    for (size_t node = self->leaves - 1; node > 0; node--)
        FUNC(__pull)(self, node);

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self)
{
    if (!self)
        return CVX_FLAG_OK;

    free(self->tree);
    *self = (struct CVX_SNAME){ 0 };
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone)
{
    enum cvx_flags flag = FUNC(_init)(clone, orig->combine, orig->identity, orig->count);
    if (flag != CVX_FLAG_OK)
        return flag;

    memcpy(clone->tree, orig->tree, 2 * orig->leaves * sizeof(CVX_VAL));
    return CVX_FLAG_OK;
}

size_t FUNC(_count)(struct CVX_SNAME *self)
{
    return self->count;
}

enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, size_t index, CVX_VAL *out)
{
    if (index >= self->count)
        return CVX_FLAG_RANGE;

    if (out)
        *out = self->tree[self->leaves + index];

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_set)(struct CVX_SNAME *self, size_t index, CVX_VAL value)
{
    if (index >= self->count)
        return CVX_FLAG_RANGE;

    size_t node = self->leaves + index;
    self->tree[node] = value;

    for (node /= 2; node > 0; node /= 2)
        FUNC(__pull)(self, node);

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_query)(struct CVX_SNAME *self, size_t from, size_t to, CVX_VAL *out)
{
    if (from > to || to > self->count)
        return CVX_FLAG_RANGE;

    CVX_VAL left = self->identity;
    CVX_VAL right = self->identity;
    from += self->leaves;
    to += self->leaves;

    while (from < to)
    {
        if (from % 2 == 1)
        {
            left = self->combine(&left, &self->tree[from]);
            from++;
        }
        if (to % 2 == 1)
        {
            to--;
            right = self->combine(&self->tree[to], &right);
        }
        from /= 2;
        to /= 2;
    }

    *out = self->combine(&left, &right);
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_all)(struct CVX_SNAME *self, CVX_VAL *out)
{
    *out = self->tree[1];
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_max_right)(struct CVX_SNAME *self, size_t from, CVX_VTAB_BOOL(pred, CVX_VAL),
                                size_t *out)
{
    if (from > self->count)
        return CVX_FLAG_RANGE;
    if (!pred || !pred(&self->identity))
        return CVX_FLAG_INVALID;

    *out = self->count;
    if (from == self->count)
        return CVX_FLAG_OK;

    CVX_VAL sum = self->identity;
    size_t node = from + self->leaves;
    do
    {
        while (node % 2 == 0)
            node /= 2;

        CVX_VAL next = self->combine(&sum, &self->tree[node]);
        if (!pred(&next))
        {
            while (node < self->leaves)
            {
                node *= 2;
                next = self->combine(&sum, &self->tree[node]);
                if (pred(&next))
                {
                    sum = next;
                    node++;
                }
            }
            *out = node - self->leaves;
            return CVX_FLAG_OK;
        }

        sum = next;
        node++;
    } while (!FUNC(__is_power_of_two)(node));

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_min_left)(struct CVX_SNAME *self, size_t to, CVX_VTAB_BOOL(pred, CVX_VAL),
                               size_t *out)
{
    if (to > self->count)
        return CVX_FLAG_RANGE;
    if (!pred || !pred(&self->identity))
        return CVX_FLAG_INVALID;

    *out = 0;
    if (to == 0)
        return CVX_FLAG_OK;

    CVX_VAL sum = self->identity;
    size_t node = to + self->leaves;
    do
    {
        node--;
        while (node > 1 && node % 2 == 1)
            node /= 2;

        CVX_VAL next = self->combine(&self->tree[node], &sum);
        if (!pred(&next))
        {
            while (node < self->leaves)
            {
                node = 2 * node + 1;
                next = self->combine(&self->tree[node], &sum);
                if (pred(&next))
                {
                    sum = next;
                    node--;
                }
            }
            *out = node + 1 - self->leaves;
            return CVX_FLAG_OK;
        }

        sum = next;
    } while (!FUNC(__is_power_of_two)(node));

    return CVX_FLAG_OK;
}

static void FUNC(__pull)(struct CVX_SNAME *self, size_t node)
{
    self->tree[node] = self->combine(&self->tree[2 * node], &self->tree[2 * node + 1]);
}

static bool FUNC(__is_power_of_two)(size_t value)
{
    return (value & (value - 1)) == 0;
}
