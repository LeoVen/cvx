/// dynamic_array.h
///
/// Status
///
///   [x] concept
///   [x] v1
///   [ ] tests
///   [ ] refine
///   [ ] stabilize
///

#include "cvx/fallback.h"

// clang-format off
#ifndef V
#error "cvx/dynamic_array.h requires V to be defined (the element type, e.g. #define V int)"
#endif
#ifndef SNAME
#error "cvx/dynamic_array.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_array)"
#endif
#ifndef PFX
#error "cvx/dynamic_array.h requires PFX to be defined (the function prefix, e.g. #define PFX ma)"
#endif
#ifndef TAG
#error "cvx/dynamic_array.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
#endif
// clang-format on

#include <stdlib.h>
#include <string.h>

#include "cvx/core.h"

#define FUNC(X) CVX_(PFX, X)
#define FUNC_PROXY(X) CVX_(PFX, CVX_(__proxy, X))
#define VTAB_V CVX_(SNAME, _vtabv)

struct VTAB_V
{
    CVX_VTAB_DEFINITION(V)
};

struct SNAME
{
    cvx_container super;
    size_t capacity;
    size_t count;
    struct VTAB_V *vtabv;
    V *buffer;
};

// Initializers and destructors
void FUNC(_init)(struct SNAME *self, struct VTAB_V *vtabv, size_t capacity);
void FUNC(_drop)(struct SNAME *self);
void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone);
// Getters
enum cvx_flags FUNC(_flag)(struct SNAME *self);
size_t FUNC(_count)(struct SNAME *self);
size_t FUNC(_capacity)(struct SNAME *self);
// Operations
bool FUNC(_empty)(struct SNAME *self);
bool FUNC(_full)(struct SNAME *self);
V FUNC(_front)(struct SNAME *self);
V FUNC(_back)(struct SNAME *self);
V FUNC(_get)(struct SNAME *self, size_t index);
/**
 * @brief Inserts an element at position 0.
 * @note If the buffer is full, it gets reallocated. The new buffer's size
 * depends on `CVX_BUFFER_GROWTH_RATE` and `CVX_BUFFER_MIN_SIZE`.
 * @param self a non-NULL pointer to a initialized dynamic array
 * @param item the item to be inserted at the front
 *
 * **Error Handling**
 * - `CVX_FLAG_ALLOC` - if reallocation of the buffer fails
 */
void FUNC(_push_front)(struct SNAME *self, V item);
/**
 * @brief Inserts an `item` at position `index`.
 * @note If the buffer is full, it gets reallocated. The new buffer's size
 * depends on `CVX_BUFFER_GROWTH_RATE` and `CVX_BUFFER_MIN_SIZE`.
 * @param self a non-NULL pointer to a initialized dynamic array
 * @param item the item to be inserted at `index`
 * @param index must be 0 <= index <= count
 *
 * **Error Handling**
 * - `CVX_FLAG_ALLOC` - if reallocation of the buffer fails
 */
void FUNC(_push_at)(struct SNAME *self, V item, size_t index);
/**
 * @brief Inserts an element at the last position.
 * @note If the buffer is full, it gets reallocated. The new buffer's size
 * depends on `CVX_BUFFER_GROWTH_RATE` and `CVX_BUFFER_MIN_SIZE`.
 * @param self a non-NULL pointer to a initialized dynamic array
 * @param item the item to be inserted at the back
 *
 * **Error Handling**
 * - `CVX_FLAG_ALLOC` - if reallocation of the buffer fails
 */
void FUNC(_push_back)(struct SNAME *self, V item);
/**
 * @brief Removes an item at the index 0.
 * @note `_pop*` functions do not cause the buffer to shrink.
 * @param self a non-NULL pointer to a initialized dynamic array
 * @return the item at position 0, or a "0" initialized item if the array
 * is empty.
 *
 * **Error Handling**
 * - `CVX_FLAG_EMPTY` - if there are no items in the dynamic array.
 */
V FUNC(_pop_front)(struct SNAME *self);
V FUNC(_pop_at)(struct SNAME *self, size_t index);
V FUNC(_pop_back)(struct SNAME *self);
V FUNC(_replace_front)(struct SNAME *self, V _new_);
V FUNC(_replace_back)(struct SNAME *self, V _new_);
void FUNC(_swap)(struct SNAME *self, size_t idx1, size_t idx2);
// Extras
int FUNC(_compare)(struct SNAME *left, struct SNAME *right);
void FUNC(_sort)(struct SNAME *self);

// TODO:
// void FUNC(_seq_push_front)(cvx_container *_col_, V *values, size_t size);
// void FUNC(_seq_push_back)(cvx_container *_col_, V *values, size_t size);
// cvx_contaier *FUNC(_sublist)(cvx_container *_col_, size_t from, size_t to);
// bool FUNC(_contains)(cvx_container *_col_, V item);
// void FUNC(indexof)(cvx_container *_col_, V item, size_t *out);

// Private functions
bool FUNC(__assert_capacity)(struct SNAME *self);
bool FUNC(__assert_buffer)(struct SNAME *self, size_t capacity);

void FUNC(_init)(struct SNAME *self, struct VTAB_V *vtabv, size_t capacity)
{
    *self = (struct SNAME){ 0 };
    self->vtabv = vtabv;
    if (0 == capacity)
        self->buffer = NULL;
    else
    {
        if (!FUNC(__assert_buffer)(self, capacity))
            return;
    }
    self->super.tag = TAG;
    self->super.flag = CVX_FLAG_OK;
}

void FUNC(_drop)(struct SNAME *self)
{
    if (!self)
        return;
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
}

void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone)
{
    FUNC(_init)(clone, orig->vtabv, orig->capacity);
    orig->super.flag = CVX_FLAG_OK;
    if (!orig->buffer || !clone->buffer)
        return;
    if (clone->vtabv && clone->vtabv->clone)
    {
        for (size_t i = 0; i < orig->count; i++)
            clone->buffer[i] = clone->vtabv->clone(orig->buffer[i]);
    }
    else
    {
        memcpy(clone->buffer, orig->buffer, orig->count * sizeof(V));
    }
    clone->count = orig->count;
}

enum cvx_flags FUNC(_flag)(struct SNAME *self)
{
    return self->super.flag;
}

size_t FUNC(_count)(struct SNAME *self)
{
    return self->count;
}

size_t FUNC(_capacity)(struct SNAME *self)
{
    return self->capacity;
}

bool FUNC(_empty)(struct SNAME *self)
{
    return self->count == 0;
}

bool FUNC(_full)(struct SNAME *self)
{
    return self->count >= self->capacity;
}

V FUNC(_front)(struct SNAME *self)
{
    if (self->count == 0 || self->buffer == NULL)
    {
        self->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    self->super.flag = CVX_FLAG_OK;
    return self->buffer[0];
}

V FUNC(_back)(struct SNAME *self)
{
    if (self->count == 0 || self->buffer == NULL)
    {
        self->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    self->super.flag = CVX_FLAG_OK;
    return self->buffer[self->count - 1];
}

V FUNC(_get)(struct SNAME *self, size_t index)
{
    if (index >= self->count)
    {
        self->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    self->super.flag = CVX_FLAG_OK;
    return self->buffer[index];
}

void FUNC(_push_front)(struct SNAME *self, V item)
{
    if (!FUNC(__assert_capacity)(self))
        return;

    if (self->count > 0)
    {
        memmove(self->buffer + 1, self->buffer, self->count * sizeof(V));
    }

    self->buffer[0] = item;
    self->count++;
}

void FUNC(_push_at)(struct SNAME *self, V item, size_t index)
{
    // TODO: allow negative indices
    // e.g. if negative -> insert at self->capacity + index
    if (index > self->count)
    {
        self->super.flag = CVX_FLAG_RANGE;
        return;
    }

    if (!FUNC(__assert_capacity)(self))
        return;

    memmove(self->buffer + index + 1, self->buffer + index, (self->count - index) * sizeof(V));

    self->buffer[index] = item;
    self->count++;
    self->super.flag = CVX_FLAG_OK;
}

void FUNC(_push_back)(struct SNAME *self, V item)
{
    if (!FUNC(__assert_capacity)(self))
        return;

    self->buffer[self->count++] = item;
}

V FUNC(_pop_front)(struct SNAME *self)
{
    if (self->count == 0)
    {
        self->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _val_ = self->buffer[0];

    memmove(self->buffer, self->buffer + 1, (self->count - 1) * sizeof(V));

    self->buffer[self->count - 1] = (V){ 0 };
    self->count--;
    self->super.flag = CVX_FLAG_OK;

    return _val_;
}

V FUNC(_pop_at)(struct SNAME *self, size_t index)
{
    // TODO: allow negative indices
    // e.g. if negative -> insert at self->capacity + index
    if (self->count == 0)
    {
        self->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    if (index >= self->count)
    {
        self->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    V _val_ = self->buffer[index];

    memmove(self->buffer + index, self->buffer + index + 1, (self->count - index - 1) * sizeof(V));

    self->buffer[self->count - 1] = (V){ 0 };
    self->count--;
    self->super.flag = CVX_FLAG_OK;

    return _val_;
}

V FUNC(_pop_back)(struct SNAME *self)
{
    if (self->count == 0)
    {
        self->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _val_ = self->buffer[self->count - 1];

    self->buffer[self->count - 1] = (V){ 0 };
    self->count--;
    self->super.flag = CVX_FLAG_OK;

    return _val_;
}

V FUNC(_replace_front)(struct SNAME *self, V _new_)
{
    if (self->count == 0)
    {
        self->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = self->buffer[0];
    self->buffer[0] = _new_;
    self->super.flag = CVX_FLAG_OK;

    return _old_;
}

V FUNC(_replace_back)(struct SNAME *self, V _new_)
{
    if (self->count == 0)
    {
        self->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = self->buffer[self->count - 1];
    self->buffer[self->count - 1] = _new_;
    self->super.flag = CVX_FLAG_OK;

    return _old_;
}

int FUNC(_compare)(struct SNAME *left, struct SNAME *right)
{
    CVX_VTAB_COMP(cmp_func, V) = NULL;

    if (left->vtabv && left->vtabv->comp)
        cmp_func = left->vtabv->comp;
    else if (right->vtabv && right->vtabv->comp)
        cmp_func = right->vtabv->comp;
    else
    {
        left->super.flag = CVX_FLAG_VTAB;
        right->super.flag = CVX_FLAG_VTAB;
        return 0;
    }

    size_t _min_ = left->count < right->count ? left->count : right->count;

    for (size_t i = 0; i < _min_; i++)
    {
        size_t cmp = cmp_func(left->buffer[i], right->buffer[i]);
        if (cmp != 0)
            return cmp;
    }

    return left->count - right->count;
}

void FUNC(_sort)(struct SNAME *self)
{
    if (!self->vtabv || !self->vtabv->comp)
    {
        self->super.flag = CVX_FLAG_VTAB;
        return;
    }

    if (self->count <= 1)
        return;

    // TODO: optimize
    for (size_t i = 0; i < self->count - 1; i++)
    {
        for (size_t j = 0; j < self->count - 1 - i; j++)
        {
            if (self->vtabv->comp(self->buffer[j], self->buffer[j + 1]) > 0)
                FUNC(_swap)(self, j, j + 1);
        }
    }
}

void FUNC(_swap)(struct SNAME *self, size_t idx1, size_t idx2)
{
    if (idx1 > self->count || idx2 > self->count)
    {
        self->super.flag = CVX_FLAG_RANGE;
        return;
    }

    if (idx1 == idx2)
        return;

    V tmp = self->buffer[idx1];
    self->buffer[idx1] = self->buffer[idx2];
    self->buffer[idx2] = tmp;
}

///
///
/// PRIVATE FUNCTIONS
///
///

bool FUNC(__assert_capacity)(struct SNAME *self)
{
    if (self->count < self->capacity)
        return true;
    size_t capacity = (size_t)(self->capacity * (CVX_BUFFER_GROWTH_RATE));
    if (capacity < CVX_BUFFER_MIN_SIZE)
        capacity = CVX_BUFFER_MIN_SIZE;
    return FUNC(__assert_buffer)(self, capacity);
}

bool FUNC(__assert_buffer)(struct SNAME *self, size_t capacity)
{
    if (!self->buffer)
    {
        self->buffer = malloc(sizeof(V) * capacity);
        if (!self->buffer)
        {
            self->super.flag = CVX_FLAG_ALLOC;
            return false;
        }
    }
    else
    {
        V *new_buffer = realloc(self->buffer, sizeof(V) * capacity);
        if (!new_buffer)
        {
            self->super.flag = CVX_FLAG_ALLOC;
            return false;
        }
        self->buffer = new_buffer;
    }
    self->capacity = capacity;
    return true;
}

///
///
/// ITERATOR
///
///
#ifdef ITERATOR
#define EMBEDDED
#define IT_V V
#define IT_SNAME CVX_(SNAME, _iter)
#define IT_PFX CVX_(PFX, _iter)
#define IT_TAG (TAG * CVX_ITER_TAG_MULT)
#include "cvx/buffer_iterator.h"
#define IT_SNAME CVX_(SNAME, _iter)
struct IT_SNAME FUNC(_iter_start)(struct SNAME *self)
{
    return FUNC(_iter__start)(self->buffer, self->capacity, self->count);
}
struct IT_SNAME FUNC(_iter_end)(struct SNAME *self)
{
    return FUNC(_iter__end)(self->buffer, self->capacity, self->count);
}
#endif

#include "cvx/undef.h"
