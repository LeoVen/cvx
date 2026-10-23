#include "circular_buffer.h"

#include <stdlib.h>

// Not part of the public API.
static size_t FUNC(__physical)(struct CVX_SNAME *self, size_t index);
static void FUNC(__drop_element)(struct CVX_SNAME *self, size_t physical);
static void FUNC(__reverse)(CVX_VAL *buffer, size_t from, size_t to);

enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_V *vtabv, size_t capacity)
{
    *self = (struct CVX_SNAME){ 0 };
    self->vtabv = vtabv;

    if (capacity == 0)
        return CVX_FLAG_OK;

    self->buffer = malloc(sizeof(CVX_VAL) * capacity);
    if (!self->buffer)
        return CVX_FLAG_ALLOC;

    self->capacity = capacity;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self)
{
    if (!self)
        return CVX_FLAG_OK;

    for (size_t i = 0; i < self->count; i++)
        FUNC(__drop_element)(self, FUNC(__physical)(self, i));

    free(self->buffer);
    self->buffer = NULL;
    self->capacity = 0;
    self->count = 0;
    self->head = 0;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone)
{
    enum cvx_flags flag = FUNC(_init)(clone, orig->vtabv, orig->capacity);
    if (flag != CVX_FLAG_OK)
        return flag;

    for (size_t i = 0; i < orig->count; i++)
    {
        CVX_VAL *src = &orig->buffer[FUNC(__physical)(orig, i)];

        if (clone->vtabv && clone->vtabv->copy)
        {
            flag = clone->vtabv->copy(src, &clone->buffer[i]);
            if (flag != CVX_FLAG_OK)
            {
                FUNC(_drop)(clone);
                return flag;
            }
        }
        else
        {
            clone->buffer[i] = *src;
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

bool FUNC(_full)(struct CVX_SNAME *self)
{
    return self->count == self->capacity;
}

bool FUNC(_is_linearized)(struct CVX_SNAME *self)
{
    return self->head + self->count <= self->capacity;
}

enum cvx_flags FUNC(_front)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    return FUNC(_get)(self, 0, out);
}

enum cvx_flags FUNC(_back)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    return FUNC(_get)(self, self->count - 1, out);
}

enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, size_t index, CVX_VAL *out)
{
    if (index >= self->count)
        return CVX_FLAG_RANGE;

    if (out)
        *out = self->buffer[FUNC(__physical)(self, index)];
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_push_back)(struct CVX_SNAME *self, CVX_VAL item)
{
    if (self->capacity == 0)
        return CVX_FLAG_FULL;

    if (self->count < self->capacity)
    {
        self->buffer[FUNC(__physical)(self, self->count)] = item;
        self->count++;
        return CVX_FLAG_OK;
    }

    FUNC(__drop_element)(self, self->head);
    self->buffer[self->head] = item;
    self->head = (self->head + 1) % self->capacity;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_push_front)(struct CVX_SNAME *self, CVX_VAL item)
{
    if (self->capacity == 0)
        return CVX_FLAG_FULL;

    self->head = (self->head + self->capacity - 1) % self->capacity;

    if (self->count < self->capacity)
        self->count++;
    else
        FUNC(__drop_element)(self, self->head);

    self->buffer[self->head] = item;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_pop_back)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    self->count--;
    size_t physical = FUNC(__physical)(self, self->count);

    if (out)
        *out = self->buffer[physical];
    else
        FUNC(__drop_element)(self, physical);
    self->buffer[physical] = (CVX_VAL){ 0 };
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_pop_front)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    if (out)
        *out = self->buffer[self->head];
    else
        FUNC(__drop_element)(self, self->head);
    self->buffer[self->head] = (CVX_VAL){ 0 };

    self->head = (self->head + 1) % self->capacity;
    self->count--;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_set_capacity)(struct CVX_SNAME *self, size_t capacity)
{
    if (capacity == self->capacity)
        return FUNC(_linearize)(self);

    if (capacity == 0)
        return FUNC(_drop)(self);

    CVX_VAL *buffer = malloc(sizeof(CVX_VAL) * capacity);
    if (!buffer)
        return CVX_FLAG_ALLOC;

    size_t excess = self->count > capacity ? self->count - capacity : 0;
    for (size_t i = 0; i < excess; i++)
        FUNC(__drop_element)(self, FUNC(__physical)(self, i));

    for (size_t i = excess; i < self->count; i++)
        buffer[i - excess] = self->buffer[FUNC(__physical)(self, i)];

    free(self->buffer);
    self->buffer = buffer;
    self->capacity = capacity;
    self->count -= excess;
    self->head = 0;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_linearize)(struct CVX_SNAME *self)
{
    if (self->head == 0)
        return CVX_FLAG_OK;

    // Rotating the whole buffer left by head brings the front element to
    // index 0 while keeping the logical order.
    FUNC(__reverse)(self->buffer, 0, self->head);
    FUNC(__reverse)(self->buffer, self->head, self->capacity);
    FUNC(__reverse)(self->buffer, 0, self->capacity);

    self->head = 0;
    return CVX_FLAG_OK;
}

static size_t FUNC(__physical)(struct CVX_SNAME *self, size_t index)
{
    return (self->head + index) % self->capacity;
}

static void FUNC(__drop_element)(struct CVX_SNAME *self, size_t physical)
{
    if (self->vtabv && self->vtabv->drop)
        self->vtabv->drop(&self->buffer[physical]);
}

// Reverses buffer[from, to).
static void FUNC(__reverse)(CVX_VAL *buffer, size_t from, size_t to)
{
    while (from + 1 < to)
    {
        to--;
        CVX_VAL tmp = buffer[from];
        buffer[from] = buffer[to];
        buffer[to] = tmp;
        from++;
    }
}
