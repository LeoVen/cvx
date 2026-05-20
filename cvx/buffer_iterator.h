/// buffer_iterator.h
///
/// Status
///
///   [x] concept
///   [x] v1
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
/// User defined macro options
///
///   - SPARSE(item): a function-like macro that receives and item and checks if this is just a
///   "hole" or an actual item
///   - CIRCULAR: if enabled, the pointer wraps around the buffer's capacity
///
/// Other macros
///
///   - EMBEDDED: Used by other data structures to automatically implement an iterator. When this is
///   enabled, it changes the input macros and prefixes them all with "IT_".
///

#include "cvx/fallback.h"

// clang-format off
#ifndef EMBEDDED
#ifndef V
#error "cvx/buffer_iterator.h requires V to be defined (the element type, e.g. #define V int)"
#endif
#ifndef SNAME
#error "cvx/buffer_iterator.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_cbuf)"
#endif
#ifndef PFX
#error "cvx/buffer_iterator.h requires PFX to be defined (the function prefix, e.g. #define PFX cb)"
#endif
#ifndef TAG
#error "cvx/buffer_iterator.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
#endif
#else // EMBEDDED
#ifndef IT_V
#error "cvx/buffer_iterator.h requires IT_V to be defined (the element type, e.g. #define IT_V int)"
#endif
#ifndef IT_SNAME
#error "cvx/buffer_iterator.h requires IT_SNAME to be defined (the struct name, e.g. #define IT_SNAME my_cbuf)"
#endif
#ifndef IT_PFX
#error "cvx/buffer_iterator.h requires IT_PFX to be defined (the function prefix, e.g. #define IT_PFX cb)"
#endif
#ifndef IT_TAG
#error "cvx/buffer_iterator.h requires IT_TAG to be defined (a unique integer tag, e.g. #define IT_TAG 1)"
#endif
#endif // EMBEDDED
// Options
#if defined(SPARSE) && defined(CIRCULAR)
#error "cvx/buffer_iterator.h can only define one macro option at a time"
#endif
// clang-format on

#ifndef EMBEDDED
#define BI_V V
#define BI_SNAME SNAME
#define BI_PFX PFX
#define BI_TAG TAG
#define BI_FN_START _start
#define BI_FN_END _end
#else
#define BI_V IT_V
#define BI_SNAME IT_SNAME
#define BI_PFX IT_PFX
#define BI_TAG IT_TAG
#define BI_FN_START __start
#define BI_FN_END __end
#endif

#include "cvx/core.h"

#define BI_FUNC(X) CVX_(BI_PFX, X)

struct BI_SNAME
{
    cvx_container super;
    size_t capacity;
    size_t pointer;
    size_t index;
    size_t count;
#ifdef CIRCULAR
    size_t head;
#endif
    V *buffer;
};

#ifdef CIRCULAR
struct BI_SNAME BI_FUNC(BI_FN_START)(V *buffer, size_t capacity, size_t count, size_t head);
#else
struct BI_SNAME BI_FUNC(BI_FN_START)(V *buffer, size_t capacity, size_t count);
#endif
#ifdef CIRCULAR
struct BI_SNAME BI_FUNC(BI_FN_END)(V *buffer, size_t capacity, size_t count, size_t tail);
#else
struct BI_SNAME BI_FUNC(BI_FN_END)(V *buffer, size_t capacity, size_t count);
#endif
bool BI_FUNC(_at_end)(struct BI_SNAME *self);
bool BI_FUNC(_at_start)(struct BI_SNAME *self);
void BI_FUNC(_to_end)(struct BI_SNAME *self);
void BI_FUNC(_to_start)(struct BI_SNAME *self);
void BI_FUNC(_next)(struct BI_SNAME *self);
void BI_FUNC(_prev)(struct BI_SNAME *self);
#ifndef SPARSE
void BI_FUNC(_go_to)(struct BI_SNAME *self);
#endif
void BI_FUNC(_forward)(struct BI_SNAME *self, size_t steps);
void BI_FUNC(_backward)(struct BI_SNAME *self, size_t steps);
size_t BI_FUNC(_index)(struct BI_SNAME *self);
V BI_FUNC(_value)(struct BI_SNAME *self);

#ifdef CIRCULAR
struct BI_SNAME BI_FUNC(BI_FN_START)(V *buffer, size_t capacity, size_t count, size_t head)
#else
struct BI_SNAME BI_FUNC(BI_FN_START)(V *buffer, size_t capacity, size_t count)
#endif
{
    struct BI_SNAME result = {
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
        .super = { .flag = CVX_FLAG_OK, .tag = BI_TAG },
    };
#ifdef SPARSE
    while (result.pointer < result.capacity && SPARSE(result.buffer[result.pointer]))
        result.pointer++;
#endif
    return result;
}

#ifdef CIRCULAR
struct BI_SNAME BI_FUNC(BI_FN_END)(V *buffer, size_t capacity, size_t count, size_t tail)
#else
struct BI_SNAME BI_FUNC(BI_FN_END)(V *buffer, size_t capacity, size_t count)
#endif
{
    struct BI_SNAME result = {
        .buffer = buffer,
        .capacity = capacity,
        .count = count,
        .index = count,
#ifdef CIRCULAR
        .head = tail,
        .pointer = tail,
#elif defined(SPARSE)
        .pointer = capacity - 1,
#else
        .pointer = count - 1,
#endif
        .super = { .flag = CVX_FLAG_OK, .tag = BI_TAG },
    };
#ifdef SPARSE
    while (result.pointer > 0 && SPARSE(result.buffer[result.pointer]))
        result.pointer--;
#endif
    return result;
}

bool BI_FUNC(_at_end)(struct BI_SNAME *self)
{
    return self->index == self->count;
}

bool BI_FUNC(_at_start)(struct BI_SNAME *self)
{
    return self->index == 0;
}

void BI_FUNC(_to_end)(struct BI_SNAME *self)
{
    // TODO:
    (void)self;
}

void BI_FUNC(_to_start)(struct BI_SNAME *self)
{
    // TODO:
    (void)self;
}

void BI_FUNC(_next)(struct BI_SNAME *self)
{
    if (BI_FUNC(_at_end)(self))
    {
        self->super.flag = CVX_FLAG_RANGE;
        return;
    }
#ifdef CIRCULAR
    self->pointer = (self->pointer + 1) % self->capacity;
#elif defined(SPARSE)
    self->pointer++;
    while (self->pointer < self->capacity && SPARSE(self->buffer[self->pointer]))
        self->pointer++;
#else
    self->pointer++;
#endif
    self->index++;
}

void BI_FUNC(_prev)(struct BI_SNAME *self)
{
    if (BI_FUNC(_at_start)(self))
    {
        self->super.flag = CVX_FLAG_RANGE;
        return;
    }
#ifdef CIRCULAR
    self->pointer = (self->pointer == 0 ? self->capacity - 1 : self->pointer - 1);
#elif defined(SPARSE)
    while (self->pointer > 0)
    {
        self->pointer--;
        if (!(SPARSE(self->buffer[self->pointer])))
            break;
    }
#else
    self->pointer--;
#endif
    self->index--;
}

#ifndef SPARSE
void BI_FUNC(_go_to)(struct BI_SNAME *self)
{
    // TODO:
    (void)self;
}
#endif

void BI_FUNC(_forward)(struct BI_SNAME *self, size_t steps)
{
    // TODO:
    (void)self;
    (void)steps;
}

void BI_FUNC(_backward)(struct BI_SNAME *self, size_t steps)
{
    // TODO:
    (void)self;
    (void)steps;
}

size_t BI_FUNC(_index)(struct BI_SNAME *self)
{
    return self->index;
}

V BI_FUNC(_value)(struct BI_SNAME *self)
{
    return self->buffer[self->pointer];
}

#undef CIRCULAR
#undef SPARSE
#undef BI_V
#undef BI_SNAME
#undef BI_PFX
#undef BI_TAG
#undef BI_FUNC

#ifndef EMBEDDED
#include "cvx/undef.h"
#endif
#undef EMBEDDED
