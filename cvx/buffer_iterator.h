/// buffer_iterator.h
///
/// Status
///
///   [x] concept
///   [ ] v1
///   [ ] tests
///   [ ] refine
///   [ ] stabilize
///
/// The implementation of the buffer iterator can be tuned depending on what the
/// underlying data structure needs.
///
/// This is the basic interface:
/// for (struct iterator it = init_iter(&arr); !iter_at_end(&it); iter_next(&it))
/// {
///     char *key = iter_key(&it);
///     int value = iter_value(&it);
///     ...
/// }
///
/// Parameters
///
/// - SPARSE: if set, items are checked against SPARSE and skipped if so
/// - CIRCULAR: if enabled, the pointer wraps around the buffer's capacity
///

#include "cvx/fallback.h"

// clang-format off
#ifndef T
#error "cvx/circular_buffer.h requires T to be defined (the element type, e.g. #define T int)"
#endif
#ifndef SNAME
#error "cvx/circular_buffer.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_cbuf)"
#endif
#ifndef PFX
#error "cvx/circular_buffer.h requires PFX to be defined (the function prefix, e.g. #define PFX cb)"
#endif
#ifndef TAG
#error "cvx/circular_buffer.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
#endif
// clang-format on

#include "cvx/core.h"

#define FUNC(X) CVX_(PFX, X)
#define ITERATOR CVX_(SNAME, _iter)
#define ITER_TAG (TAG * CVX_ITER_TAG_MULT)

struct SNAME
{
    cvx_container super;
    size_t capacity;
    size_t pointer;
    size_t index;
    size_t count;
#ifdef CIRCULAR
    size_t head;
#endif
    T *buffer;
};

#ifdef CIRCULAR
struct SNAME FUNC(_init)(T *buffer, size_t capacity, size_t count, size_t head)
#else
struct SNAME FUNC(_init)(T *buffer, size_t capacity, size_t count)
#endif
{
    struct SNAME result = {
        .buffer = buffer,
        .capacity = capacity,
        .count = count,
        .index = 0,
#ifdef CIRCULAR
        .head = head,
        .pointer = head,
#else
        .pointer = 0,
#endif
        .super = { .flag = CVX_FLAG_OK, .tag = ITER_TAG },
    };
#ifdef SPARSE
    while (result.pointer < result.capacity && result.buffer[result.pointer] == SPARSE)
        result.pointer++;
#endif
    return result;
}

bool FUNC(_at_end)(struct SNAME *self)
{
    return self->index == self->count;
}

bool FUNC(_at_start)(struct SNAME *self)
{
    return self->index == 0;
}

void FUNC(_next)(struct SNAME *self)
{
    if (FUNC(_at_end)(self))
        return;
#ifdef CIRCULAR
    self->pointer = (self->pointer + 1) % self->capacity;
#elif defined(SPARSE)
    self->pointer++;
    while (self->pointer < self->capacity && self->buffer[self->pointer] == SPARSE)
        self->pointer++;
#else
    self->pointer++;
#endif
    self->index++;
}

size_t FUNC(_index)(struct SNAME *self)
{
    return self->index;
}

T FUNC(_value)(struct SNAME *self)
{
    return self->buffer[self->pointer];
}

#ifdef CIRCULAR
#undef CIRCULAR
#endif
#include "cvx/undef.h"
