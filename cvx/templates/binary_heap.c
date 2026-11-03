#include "binary_heap.h"

#include <stdlib.h>
#include <string.h>

// Not part of the public API.
static void FUNC(__sift_up)(struct CVX_SNAME *self, size_t index);
static void FUNC(__sift_down)(struct CVX_SNAME *self, size_t index);
static void FUNC(__swap)(struct CVX_SNAME *self, size_t a, size_t b);
static void FUNC(__release)(struct CVX_SNAME *self, CVX_VAL *value, CVX_VAL *out);
static bool FUNC(__assert_capacity_for)(struct CVX_SNAME *self, size_t additional);
static bool FUNC(__assert_buffer)(struct CVX_SNAME *self, size_t capacity);

enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_V *vtabv, size_t capacity)
{
    *self = (struct CVX_SNAME){ 0 };

    if (!vtabv || !vtabv->comp)
        return CVX_FLAG_VTAB;

    self->vtabv = vtabv;

    if (capacity > 0 && !FUNC(__assert_buffer)(self, capacity))
        return CVX_FLAG_ALLOC;

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_heapify)(struct CVX_SNAME *self, struct VTAB_V *vtabv, CVX_VAL *values,
                              size_t count)
{
    enum cvx_flags flag = FUNC(_init)(self, vtabv, count);
    if (flag != CVX_FLAG_OK)
        return flag;

    if (count == 0)
        return CVX_FLAG_OK;

    memcpy(self->buffer, values, count * sizeof(CVX_VAL));
    self->count = count;

    for (size_t i = self->count / 2; i > 0; i--)
        FUNC(__sift_down)(self, i - 1);

    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self)
{
    if (!self)
        return CVX_FLAG_OK;

    FUNC(_clear)(self);
    free(self->buffer);
    self->buffer = NULL;
    self->capacity = 0;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone)
{
    enum cvx_flags flag = FUNC(_init)(clone, orig->vtabv, orig->capacity);
    if (flag != CVX_FLAG_OK)
        return flag;

    if (orig->count == 0)
        return CVX_FLAG_OK;

    if (!clone->vtabv->copy)
    {
        memcpy(clone->buffer, orig->buffer, orig->count * sizeof(CVX_VAL));
        clone->count = orig->count;
        return CVX_FLAG_OK;
    }

    for (size_t i = 0; i < orig->count; i++)
    {
        flag = clone->vtabv->copy(&orig->buffer[i], &clone->buffer[i]);
        if (flag != CVX_FLAG_OK)
        {
            FUNC(_drop)(clone);
            return flag;
        }
        clone->count++;
    }

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

enum cvx_flags FUNC(_peek)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    if (out)
        *out = self->buffer[0];
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_push)(struct CVX_SNAME *self, CVX_VAL item)
{
    if (!FUNC(__assert_capacity_for)(self, 1))
        return CVX_FLAG_ALLOC;

    self->buffer[self->count] = item;
    self->count++;
    FUNC(__sift_up)(self, self->count - 1);
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_pop)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    FUNC(__release)(self, &self->buffer[0], out);

    self->count--;
    self->buffer[0] = self->buffer[self->count];
    self->buffer[self->count] = (CVX_VAL){ 0 };
    FUNC(__sift_down)(self, 0);
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_push_pop)(struct CVX_SNAME *self, CVX_VAL item, CVX_VAL *out)
{
    if (self->count == 0 || self->vtabv->comp(&item, &self->buffer[0]) >= 0)
    {
        FUNC(__release)(self, &item, out);
        return CVX_FLAG_OK;
    }

    return FUNC(_replace)(self, item, out);
}

enum cvx_flags FUNC(_replace)(struct CVX_SNAME *self, CVX_VAL item, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    FUNC(__release)(self, &self->buffer[0], out);

    self->buffer[0] = item;
    FUNC(__sift_down)(self, 0);
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_merge)(struct CVX_SNAME *self, struct CVX_SNAME *other)
{
    if (other == self)
        return CVX_FLAG_INVALID;
    if (other->count == 0)
        return CVX_FLAG_OK;
    if (!FUNC(__assert_capacity_for)(self, other->count))
        return CVX_FLAG_ALLOC;

    size_t first_new = self->count;
    memcpy(self->buffer + self->count, other->buffer, other->count * sizeof(CVX_VAL));
    self->count += other->count;

    if (other->count >= first_new)
    {
        for (size_t i = self->count / 2; i > 0; i--)
            FUNC(__sift_down)(self, i - 1);
    }
    else
    {
        for (size_t i = first_new; i < self->count; i++)
            FUNC(__sift_up)(self, i);
    }

    memset(other->buffer, 0, other->count * sizeof(CVX_VAL));
    other->count = 0;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_clear)(struct CVX_SNAME *self)
{
    if (self->count == 0)
        return CVX_FLAG_OK;

    if (self->vtabv && self->vtabv->drop)
    {
        for (size_t i = 0; i < self->count; i++)
            self->vtabv->drop(&self->buffer[i]);
    }

    memset(self->buffer, 0, self->count * sizeof(CVX_VAL));
    self->count = 0;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_reserve)(struct CVX_SNAME *self, size_t capacity)
{
    if (capacity <= self->capacity)
        return CVX_FLAG_OK;

    if (!FUNC(__assert_buffer)(self, capacity))
        return CVX_FLAG_ALLOC;

    return CVX_FLAG_OK;
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

    if (!FUNC(__assert_buffer)(self, self->count))
        return CVX_FLAG_ALLOC;

    return CVX_FLAG_OK;
}

static void FUNC(__sift_up)(struct CVX_SNAME *self, size_t index)
{
    while (index > 0)
    {
        size_t parent = (index - 1) / 2;
        if (self->vtabv->comp(&self->buffer[index], &self->buffer[parent]) <= 0)
            return;

        FUNC(__swap)(self, index, parent);
        index = parent;
    }
}

static void FUNC(__sift_down)(struct CVX_SNAME *self, size_t index)
{
    for (;;)
    {
        size_t left = 2 * index + 1;
        size_t right = left + 1;
        size_t greatest = index;

        if (left < self->count &&
            self->vtabv->comp(&self->buffer[left], &self->buffer[greatest]) > 0)
            greatest = left;

        if (right < self->count &&
            self->vtabv->comp(&self->buffer[right], &self->buffer[greatest]) > 0)
            greatest = right;

        if (greatest == index)
            return;

        FUNC(__swap)(self, index, greatest);
        index = greatest;
    }
}

static void FUNC(__swap)(struct CVX_SNAME *self, size_t a, size_t b)
{
    CVX_VAL tmp = self->buffer[a];
    self->buffer[a] = self->buffer[b];
    self->buffer[b] = tmp;
}

static void FUNC(__release)(struct CVX_SNAME *self, CVX_VAL *value, CVX_VAL *out)
{
    if (out)
        *out = *value;
    else if (self->vtabv->drop)
        self->vtabv->drop(value);
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
        size_t next = (size_t)(capacity * (CVX_BUFFER_GROWTH_RATE));
        capacity = next > capacity ? next : capacity + 1;
    }

    return FUNC(__assert_buffer)(self, capacity);
}

static bool FUNC(__assert_buffer)(struct CVX_SNAME *self, size_t capacity)
{
    CVX_VAL *buffer = realloc(self->buffer, sizeof(CVX_VAL) * capacity);
    if (!buffer)
        return false;

    self->buffer = buffer;
    self->capacity = capacity;
    return true;
}
