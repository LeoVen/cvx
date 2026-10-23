#include "dynamic_array.h"

#include <stdlib.h>
#include <string.h>

// Not part of the public API.
static bool FUNC(__assert_capacity)(struct CVX_SNAME *self);
static bool FUNC(__assert_capacity_for)(struct CVX_SNAME *self, size_t additional);
static bool FUNC(__assert_buffer)(struct CVX_SNAME *self, size_t capacity);

enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_V *vtabv, size_t capacity)
{
    *self = (struct CVX_SNAME){ 0 };
    self->vtabv = vtabv;

    if (capacity == 0)
        return CVX_FLAG_OK;

    if (!FUNC(__assert_buffer)(self, capacity))
        return CVX_FLAG_ALLOC;

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self)
{
    if (!self)
        return CVX_FLAG_OK;

    if (self->buffer)
    {
        if (self->vtabv && self->vtabv->drop)
        {
            for (size_t i = 0; i < self->count; i++)
                self->vtabv->drop(&self->buffer[i]);
        }
        free(self->buffer);
        self->buffer = NULL;
    }

    self->capacity = 0;
    self->count = 0;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone)
{
    enum cvx_flags flag = FUNC(_init)(clone, orig->vtabv, orig->capacity);
    if (flag != CVX_FLAG_OK)
        return flag;

    if (!orig->buffer || !clone->buffer)
        return CVX_FLAG_OK;

    if (clone->vtabv && clone->vtabv->copy)
    {
        for (size_t i = 0; i < orig->count; i++)
        {
            CVX_VAL cloned;
            enum cvx_flags flag = clone->vtabv->copy(&orig->buffer[i], &cloned);

            if (flag != CVX_FLAG_OK)
                return flag;

            clone->buffer[i] = cloned;
        }
    }
    else
    {
        memcpy(clone->buffer, orig->buffer, orig->count * sizeof(CVX_VAL));
    }
    clone->count = orig->count;

    return CVX_FLAG_OK;
}

size_t FUNC(_count)(struct CVX_SNAME *self)
{
    return self->count;
}

size_t FUNC(_capacity)(struct CVX_SNAME *self)
{
    return self->capacity;
}

bool FUNC(_empty)(struct CVX_SNAME *self)
{
    return self->count == 0;
}

bool FUNC(_full)(struct CVX_SNAME *self)
{
    return self->count >= self->capacity;
}

enum cvx_flags FUNC(_front)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0 || self->buffer == NULL)
        return CVX_FLAG_EMPTY;

    if (out)
        *out = self->buffer[0];
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_back)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0 || self->buffer == NULL)
        return CVX_FLAG_EMPTY;

    if (out)
        *out = self->buffer[self->count - 1];
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, size_t index, CVX_VAL *out)
{
    if (index >= self->count)
        return CVX_FLAG_RANGE;

    if (out)
        *out = self->buffer[index];
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_push)(struct CVX_SNAME *self, size_t index, CVX_VAL item)
{
    if (index > self->count)
        return CVX_FLAG_RANGE;

    if (!FUNC(__assert_capacity)(self))
        return CVX_FLAG_ALLOC;

    memmove(self->buffer + index + 1, self->buffer + index,
            (self->count - index) * sizeof(CVX_VAL));

    self->buffer[index] = item;
    self->count++;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_pop)(struct CVX_SNAME *self, size_t index, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    if (index >= self->count)
        return CVX_FLAG_RANGE;

    if (out)
        *out = self->buffer[index];
    else if (self->vtabv && self->vtabv->drop)
        self->vtabv->drop(&self->buffer[index]);

    memmove(self->buffer + index, self->buffer + index + 1,
            (self->count - index - 1) * sizeof(CVX_VAL));

    self->buffer[self->count - 1] = (CVX_VAL){ 0 };
    self->count--;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_seq_push)(struct CVX_SNAME *self, size_t index, CVX_VAL *values, size_t size)
{
    if (index > self->count)
        return CVX_FLAG_RANGE;

    if (size == 0)
        return CVX_FLAG_OK;

    if (!FUNC(__assert_capacity_for)(self, size))
        return CVX_FLAG_ALLOC;

    memmove(self->buffer + index + size, self->buffer + index,
            (self->count - index) * sizeof(CVX_VAL));
    memcpy(self->buffer + index, values, size * sizeof(CVX_VAL));

    self->count += size;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_index_of)(struct CVX_SNAME *self, CVX_VAL item, size_t *out)
{
    if (!self->vtabv || !self->vtabv->comp)
        return CVX_FLAG_VTAB;

    for (size_t i = 0; i < self->count; i++)
    {
        if (self->vtabv->comp(&self->buffer[i], &item) == 0)
        {
            if (out)
                *out = i;
            return CVX_FLAG_OK;
        }
    }

    return CVX_FLAG_NOT_FOUND;
}

enum cvx_flags FUNC(_truncate)(struct CVX_SNAME *self, size_t new_count)
{
    if (new_count >= self->count)
        return CVX_FLAG_OK;

    if (self->vtabv && self->vtabv->drop)
    {
        for (size_t i = new_count; i < self->count; i++)
            self->vtabv->drop(&self->buffer[i]);
    }

    memset(self->buffer + new_count, 0, (self->count - new_count) * sizeof(CVX_VAL));
    self->count = new_count;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_sublist)(struct CVX_SNAME *self, size_t from, size_t to, struct CVX_SNAME *out)
{
    if (from > to || to >= self->count)
        return CVX_FLAG_RANGE;

    size_t count = to - from + 1;
    enum cvx_flags flag = FUNC(_init)(out, self->vtabv, count);
    if (flag != CVX_FLAG_OK)
        return flag;

    if (out->vtabv && out->vtabv->copy)
    {
        for (size_t i = from; i <= to; i++)
        {
            flag = out->vtabv->copy(&self->buffer[i], &out->buffer[out->count]);
            if (flag != CVX_FLAG_OK)
                return flag;
            out->count++;
        }
    }
    else
    {
        memcpy(out->buffer, self->buffer + from, count * sizeof(CVX_VAL));
        out->count = count;
    }

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_append)(struct CVX_SNAME *self, struct CVX_SNAME *src)
{
    if (src->count == 0)
        return CVX_FLAG_OK;

    if (!FUNC(__assert_capacity_for)(self, src->count))
        return CVX_FLAG_ALLOC;

    if (self->vtabv && self->vtabv->copy)
    {
        for (size_t i = 0; i < src->count; i++)
        {
            enum cvx_flags flag = self->vtabv->copy(&src->buffer[i], &self->buffer[self->count]);
            if (flag != CVX_FLAG_OK)
                return flag;
            self->count++;
        }
    }
    else
    {
        memcpy(self->buffer + self->count, src->buffer, src->count * sizeof(CVX_VAL));
        self->count += src->count;
    }

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_prepend)(struct CVX_SNAME *self, struct CVX_SNAME *src)
{
    if (src->count == 0)
        return CVX_FLAG_OK;

    if (!FUNC(__assert_capacity_for)(self, src->count))
        return CVX_FLAG_ALLOC;

    memmove(self->buffer + src->count, self->buffer, self->count * sizeof(CVX_VAL));
    self->count += src->count;

    if (self->vtabv && self->vtabv->copy)
    {
        for (size_t i = 0; i < src->count; i++)
        {
            enum cvx_flags flag = self->vtabv->copy(&src->buffer[i], &self->buffer[i]);
            if (flag != CVX_FLAG_OK)
                return flag;
        }
    }
    else
    {
        memcpy(self->buffer, src->buffer, src->count * sizeof(CVX_VAL));
    }

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_replace)(struct CVX_SNAME *self, size_t index, CVX_VAL new_value,
                              CVX_VAL *old_out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;
    if (index >= self->count)
        return CVX_FLAG_RANGE;

    if (old_out)
        *old_out = self->buffer[index];
    else if (self->vtabv && self->vtabv->drop)
        self->vtabv->drop(&self->buffer[index]);

    self->buffer[index] = new_value;

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_swap)(struct CVX_SNAME *self, size_t idx1, size_t idx2)
{
    if (idx1 >= self->count || idx2 >= self->count)
        return CVX_FLAG_RANGE;

    if (idx1 == idx2)
        return CVX_FLAG_OK;

    CVX_VAL tmp = self->buffer[idx1];
    self->buffer[idx1] = self->buffer[idx2];
    self->buffer[idx2] = tmp;
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

    size_t min_count = left->count < right->count ? left->count : right->count;

    for (size_t i = 0; i < min_count; i++)
    {
        int cmp = cmp_func(&left->buffer[i], &right->buffer[i]);
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

enum cvx_flags FUNC(_sort)(struct CVX_SNAME *self)
{
    if (!self->vtabv || !self->vtabv->comp)
        return CVX_FLAG_VTAB;

    if (self->count <= 1)
        return CVX_FLAG_OK;

    // Bubble sort: O(n^2).
    for (size_t i = 0; i < self->count - 1; i++)
    {
        for (size_t j = 0; j < self->count - 1 - i; j++)
        {
            if (self->vtabv->comp(&self->buffer[j], &self->buffer[j + 1]) > 0)
                FUNC(_swap)(self, j, j + 1);
        }
    }

    return CVX_FLAG_OK;
}

static bool FUNC(__assert_capacity)(struct CVX_SNAME *self)
{
    return FUNC(__assert_capacity_for)(self, 1);
}

static bool FUNC(__assert_capacity_for)(struct CVX_SNAME *self, size_t additional)
{
    size_t needed = self->count + additional;
    if (needed <= self->capacity)
        return true;

    size_t capacity = (size_t)(self->capacity * (CVX_BUFFER_GROWTH_RATE));
    if (capacity < CVX_BUFFER_MIN_SIZE)
        capacity = CVX_BUFFER_MIN_SIZE;
    while (capacity < needed)
    {
        // Guards against a configured CVX_BUFFER_GROWTH_RATE too close to 1 to
        // make progress after rounding down.
        size_t next = (size_t)(capacity * (CVX_BUFFER_GROWTH_RATE));
        capacity = next > capacity ? next : capacity + 1;
    }

    return FUNC(__assert_buffer)(self, capacity);
}

static bool FUNC(__assert_buffer)(struct CVX_SNAME *self, size_t capacity)
{
    if (!self->buffer)
    {
        self->buffer = malloc(sizeof(CVX_VAL) * capacity);
        if (!self->buffer)
            return false;
    }
    else
    {
        CVX_VAL *new_buffer = realloc(self->buffer, sizeof(CVX_VAL) * capacity);
        if (!new_buffer)
            return false;
        self->buffer = new_buffer;
    }
    self->capacity = capacity;
    return true;
}

enum cvx_flags FUNC(_shrink)(struct CVX_SNAME *self)
{
    if (self->capacity == self->count)
        return CVX_FLAG_OK;

    if (self->count == 0)
    {
        free(self->buffer);
        self->buffer = NULL;
        self->capacity = 0;
        return CVX_FLAG_OK;
    }

    CVX_VAL *new_buffer = realloc(self->buffer, sizeof(CVX_VAL) * self->count);
    if (!new_buffer)
        return CVX_FLAG_ALLOC;

    self->buffer = new_buffer;
    self->capacity = self->count;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_resize)(struct CVX_SNAME *self, size_t capacity)
{
    if (capacity <= self->capacity)
        return CVX_FLAG_OK;

    if (!FUNC(__assert_buffer)(self, capacity))
        return CVX_FLAG_ALLOC;

    return CVX_FLAG_OK;
}
