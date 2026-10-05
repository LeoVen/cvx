#include "dynamic_array.h"
#include <stdlib.h>
#include <string.h>

// Not part of the public API.
static bool da__assert_capacity(struct DynamicArray *self);
static bool da__assert_capacity_for(struct DynamicArray *self, size_t additional);
static bool da__assert_buffer(struct DynamicArray *self, size_t capacity);

enum cvx_flags da_init(struct DynamicArray *self, struct DynamicArrayVtabV *vtabv, size_t capacity)
{
    *self = (struct DynamicArray){ 0 };
    self->vtabv = vtabv;

    if (capacity == 0)
        return CVX_FLAG_OK;

    if (!da__assert_buffer(self, capacity))
        return CVX_FLAG_ALLOC;

    return CVX_FLAG_OK;
}

enum cvx_flags da_drop(struct DynamicArray *self)
{
    if (!self)
        return CVX_FLAG_OK;

    if (self->buffer)
    {
        if (self->vtabv && self->vtabv->drop)
        {
            for (size_t i = 0; i < self->count; i++)
                self->vtabv->drop(self->buffer[i]);
        }
        free(self->buffer);
        self->buffer = NULL;
    }

    self->capacity = 0;
    self->count = 0;
    return CVX_FLAG_OK;
}

enum cvx_flags da_clone(struct DynamicArray *orig, struct DynamicArray *clone)
{
    enum cvx_flags flag = da_init(clone, orig->vtabv, orig->capacity);
    if (flag != CVX_FLAG_OK)
        return flag;

    if (!orig->buffer || !clone->buffer)
        return CVX_FLAG_OK;

    if (clone->vtabv && clone->vtabv->copy)
    {
        for (size_t i = 0; i < orig->count; i++)
        {
            void *cloned;
            enum cvx_flags flag = clone->vtabv->copy(orig->buffer[i], &cloned);

            if (flag != CVX_FLAG_OK)
                return flag;

            clone->buffer[i] = cloned;
        }
    }
    else
    {
        memcpy(clone->buffer, orig->buffer, orig->count * sizeof(void *));
    }
    clone->count = orig->count;

    return CVX_FLAG_OK;
}

size_t da_count(struct DynamicArray *self)
{
    return self->count;
}

size_t da_capacity(struct DynamicArray *self)
{
    return self->capacity;
}

bool da_empty(struct DynamicArray *self)
{
    return self->count == 0;
}

bool da_full(struct DynamicArray *self)
{
    return self->count >= self->capacity;
}

enum cvx_flags da_front(struct DynamicArray *self, void **out)
{
    if (self->count == 0 || self->buffer == NULL)
        return CVX_FLAG_EMPTY;

    if (out)
        *out = self->buffer[0];
    return CVX_FLAG_OK;
}

enum cvx_flags da_back(struct DynamicArray *self, void **out)
{
    if (self->count == 0 || self->buffer == NULL)
        return CVX_FLAG_EMPTY;

    if (out)
        *out = self->buffer[self->count - 1];
    return CVX_FLAG_OK;
}

enum cvx_flags da_get(struct DynamicArray *self, size_t index, void **out)
{
    if (index >= self->count)
        return CVX_FLAG_RANGE;

    if (out)
        *out = self->buffer[index];
    return CVX_FLAG_OK;
}

enum cvx_flags da_push(struct DynamicArray *self, size_t index, void *item)
{
    if (index > self->count)
        return CVX_FLAG_RANGE;

    if (!da__assert_capacity(self))
        return CVX_FLAG_ALLOC;

    memmove(self->buffer + index + 1, self->buffer + index, (self->count - index) * sizeof(void *));

    self->buffer[index] = item;
    self->count++;
    return CVX_FLAG_OK;
}

enum cvx_flags da_pop(struct DynamicArray *self, size_t index, void **out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    if (index >= self->count)
        return CVX_FLAG_RANGE;

    if (out)
        *out = self->buffer[index];

    memmove(self->buffer + index, self->buffer + index + 1,
            (self->count - index - 1) * sizeof(void *));

    self->buffer[self->count - 1] = (void *){ 0 };
    self->count--;
    return CVX_FLAG_OK;
}

enum cvx_flags da_seq_push(struct DynamicArray *self, size_t index, void **values, size_t size)
{
    if (index > self->count)
        return CVX_FLAG_RANGE;

    if (size == 0)
        return CVX_FLAG_OK;

    if (!da__assert_capacity_for(self, size))
        return CVX_FLAG_ALLOC;

    memmove(self->buffer + index + size, self->buffer + index,
            (self->count - index) * sizeof(void *));
    memcpy(self->buffer + index, values, size * sizeof(void *));

    self->count += size;
    return CVX_FLAG_OK;
}

enum cvx_flags da_index_of(struct DynamicArray *self, void *item, size_t *out)
{
    if (!self->vtabv || !self->vtabv->comp)
        return CVX_FLAG_VTAB;

    for (size_t i = 0; i < self->count; i++)
    {
        if (self->vtabv->comp(self->buffer[i], item) == 0)
        {
            if (out)
                *out = i;
            return CVX_FLAG_OK;
        }
    }

    return CVX_FLAG_NOT_FOUND;
}

enum cvx_flags da_truncate(struct DynamicArray *self, size_t new_count)
{
    if (new_count >= self->count)
        return CVX_FLAG_OK;

    if (self->vtabv && self->vtabv->drop)
    {
        for (size_t i = new_count; i < self->count; i++)
            self->vtabv->drop(self->buffer[i]);
    }

    memset(self->buffer + new_count, 0, (self->count - new_count) * sizeof(void *));
    self->count = new_count;
    return CVX_FLAG_OK;
}

enum cvx_flags da_sublist(struct DynamicArray *self, size_t from, size_t to,
                          struct DynamicArray *out)
{
    if (from > to || to >= self->count)
        return CVX_FLAG_RANGE;

    size_t count = to - from + 1;
    enum cvx_flags flag = da_init(out, self->vtabv, count);
    if (flag != CVX_FLAG_OK)
        return flag;

    if (out->vtabv && out->vtabv->copy)
    {
        for (size_t i = from; i <= to; i++)
        {
            flag = out->vtabv->copy(self->buffer[i], &out->buffer[out->count]);
            if (flag != CVX_FLAG_OK)
                return flag;
            out->count++;
        }
    }
    else
    {
        memcpy(out->buffer, self->buffer + from, count * sizeof(void *));
        out->count = count;
    }

    return CVX_FLAG_OK;
}

enum cvx_flags da_append(struct DynamicArray *self, struct DynamicArray *src)
{
    if (src->count == 0)
        return CVX_FLAG_OK;

    if (!da__assert_capacity_for(self, src->count))
        return CVX_FLAG_ALLOC;

    if (self->vtabv && self->vtabv->copy)
    {
        for (size_t i = 0; i < src->count; i++)
        {
            enum cvx_flags flag = self->vtabv->copy(src->buffer[i], &self->buffer[self->count]);
            if (flag != CVX_FLAG_OK)
                return flag;
            self->count++;
        }
    }
    else
    {
        memcpy(self->buffer + self->count, src->buffer, src->count * sizeof(void *));
        self->count += src->count;
    }

    return CVX_FLAG_OK;
}

enum cvx_flags da_prepend(struct DynamicArray *self, struct DynamicArray *src)
{
    if (src->count == 0)
        return CVX_FLAG_OK;

    if (!da__assert_capacity_for(self, src->count))
        return CVX_FLAG_ALLOC;

    memmove(self->buffer + src->count, self->buffer, self->count * sizeof(void *));
    self->count += src->count;

    if (self->vtabv && self->vtabv->copy)
    {
        for (size_t i = 0; i < src->count; i++)
        {
            enum cvx_flags flag = self->vtabv->copy(src->buffer[i], &self->buffer[i]);
            if (flag != CVX_FLAG_OK)
                return flag;
        }
    }
    else
    {
        memcpy(self->buffer, src->buffer, src->count * sizeof(void *));
    }

    return CVX_FLAG_OK;
}

enum cvx_flags da_replace(struct DynamicArray *self, size_t index, void *new_value, void **old_out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;
    if (index >= self->count)
        return CVX_FLAG_RANGE;

    if (old_out)
        *old_out = self->buffer[index];

    self->buffer[index] = new_value;

    return CVX_FLAG_OK;
}

enum cvx_flags da_swap(struct DynamicArray *self, size_t idx1, size_t idx2)
{
    if (idx1 >= self->count || idx2 >= self->count)
        return CVX_FLAG_RANGE;

    if (idx1 == idx2)
        return CVX_FLAG_OK;

    void *tmp = self->buffer[idx1];
    self->buffer[idx1] = self->buffer[idx2];
    self->buffer[idx2] = tmp;
    return CVX_FLAG_OK;
}

enum cvx_flags da_compare(struct DynamicArray *left, struct DynamicArray *right, int *out)
{
    CVX_VTAB_COMP(cmp_func, void *) = NULL;

    if (left->vtabv && left->vtabv->comp)
        cmp_func = left->vtabv->comp;
    else if (right->vtabv && right->vtabv->comp)
        cmp_func = right->vtabv->comp;
    else
        return CVX_FLAG_VTAB;

    size_t min_count = left->count < right->count ? left->count : right->count;

    for (size_t i = 0; i < min_count; i++)
    {
        int cmp = cmp_func(left->buffer[i], right->buffer[i]);
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

enum cvx_flags da_sort(struct DynamicArray *self)
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
            if (self->vtabv->comp(self->buffer[j], self->buffer[j + 1]) > 0)
                da_swap(self, j, j + 1);
        }
    }

    return CVX_FLAG_OK;
}

static bool da__assert_capacity(struct DynamicArray *self)
{
    return da__assert_capacity_for(self, 1);
}

static bool da__assert_capacity_for(struct DynamicArray *self, size_t additional)
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

    return da__assert_buffer(self, capacity);
}

static bool da__assert_buffer(struct DynamicArray *self, size_t capacity)
{
    if (!self->buffer)
    {
        self->buffer = malloc(sizeof(void *) * capacity);
        if (!self->buffer)
            return false;
    }
    else
    {
        void **new_buffer = realloc(self->buffer, sizeof(void *) * capacity);
        if (!new_buffer)
            return false;
        self->buffer = new_buffer;
    }
    self->capacity = capacity;
    return true;
}

enum cvx_flags da_shrink(struct DynamicArray *self)
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

    void **new_buffer = realloc(self->buffer, sizeof(void *) * self->count);
    if (!new_buffer)
        return CVX_FLAG_ALLOC;

    self->buffer = new_buffer;
    self->capacity = self->count;
    return CVX_FLAG_OK;
}

enum cvx_flags da_resize(struct DynamicArray *self, size_t capacity)
{
    if (capacity <= self->capacity)
        return CVX_FLAG_OK;

    if (!da__assert_buffer(self, capacity))
        return CVX_FLAG_ALLOC;

    return CVX_FLAG_OK;
}
