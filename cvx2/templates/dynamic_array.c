#include "dynamic_array.h"

#include <stdlib.h>
#include <string.h>

// Private helpers (kept `static`: the generator emits one .c per
// instantiation, so nothing outside this translation unit needs them).
static bool FUNC(__assert_capacity)(struct CVX_SNAME *self);
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
                self->vtabv->drop(self->buffer[i]);
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

    if (clone->vtabv && clone->vtabv->clone)
    {
        for (size_t i = 0; i < orig->count; i++)
        {
            CVX_VAL cloned;
            enum cvx_flags flag = clone->vtabv->clone(orig->buffer[i], &cloned);

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

enum cvx_flags FUNC(_push_front)(struct CVX_SNAME *self, CVX_VAL item)
{
    if (!FUNC(__assert_capacity)(self))
        return CVX_FLAG_ALLOC;

    if (self->count > 0)
        memmove(self->buffer + 1, self->buffer, self->count * sizeof(CVX_VAL));

    self->buffer[0] = item;
    self->count++;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_push_at)(struct CVX_SNAME *self, CVX_VAL item, size_t index)
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

enum cvx_flags FUNC(_push_back)(struct CVX_SNAME *self, CVX_VAL item)
{
    if (!FUNC(__assert_capacity)(self))
        return CVX_FLAG_ALLOC;

    self->buffer[self->count++] = item;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_pop_front)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    if (out)
        *out = self->buffer[0];

    memmove(self->buffer, self->buffer + 1, (self->count - 1) * sizeof(CVX_VAL));

    self->buffer[self->count - 1] = (CVX_VAL){ 0 };
    self->count--;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_pop_at)(struct CVX_SNAME *self, size_t index, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    if (index >= self->count)
        return CVX_FLAG_RANGE;

    if (out)
        *out = self->buffer[index];

    memmove(self->buffer + index, self->buffer + index + 1,
            (self->count - index - 1) * sizeof(CVX_VAL));

    self->buffer[self->count - 1] = (CVX_VAL){ 0 };
    self->count--;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_pop_back)(struct CVX_SNAME *self, CVX_VAL *out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    if (out)
        *out = self->buffer[self->count - 1];

    self->buffer[self->count - 1] = (CVX_VAL){ 0 };
    self->count--;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_replace_front)(struct CVX_SNAME *self, CVX_VAL new_value, CVX_VAL *old_out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    if (old_out)
        *old_out = self->buffer[0];
    self->buffer[0] = new_value;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_replace_back)(struct CVX_SNAME *self, CVX_VAL new_value, CVX_VAL *old_out)
{
    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    if (old_out)
        *old_out = self->buffer[self->count - 1];
    self->buffer[self->count - 1] = new_value;
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

enum cvx_flags FUNC(_sort)(struct CVX_SNAME *self)
{
    if (!self->vtabv || !self->vtabv->comp)
        return CVX_FLAG_VTAB;

    if (self->count <= 1)
        return CVX_FLAG_OK;

    // TODO: optimize
    for (size_t i = 0; i < self->count - 1; i++)
    {
        for (size_t j = 0; j < self->count - 1 - i; j++)
        {
            if (self->vtabv->comp(self->buffer[j], self->buffer[j + 1]) > 0)
                FUNC(_swap)(self, j, j + 1);
        }
    }

    return CVX_FLAG_OK;
}

static bool FUNC(__assert_capacity)(struct CVX_SNAME *self)
{
    if (self->count < self->capacity)
        return true;
    size_t capacity = (size_t)(self->capacity * (CVX_BUFFER_GROWTH_RATE));
    if (capacity < CVX_BUFFER_MIN_SIZE)
        capacity = CVX_BUFFER_MIN_SIZE;
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
