/**
 * @file array_iterator.h
 * @author Leonardo Vencovsky
 * @brief An iterator that works on contiguous arrays.
 * @version 0.0.1
 *
 * The implementation of the array iterator can be tuned depending on the underlying data
 * structure. By default, this is an iterator that starts at the index 0 of an array and goes up to
 * `count`. By specifying extra options, you can change the behaviour of the iterator. Only one of
 * them can be defined.
 *
 * ## Options
 *
 * - `CIRCULAR`: if defined, the pointer wraps around the array's capacity. The `_start` and
 * `_end` functions now require an extra parameter to specify at which index the iterator starts or
 * ends.
 * - `SPARSE(item)`: a function-like macro. If defined, the array contains "holes". This macro
 * needs to return a boolean value that checks if the passed item is a hole or not.
 *
 * ## Required Macros
 *
 * - `V`: Type name of the array (e.g. `V *buffer`).
 * - `SNAME`: Prefix of all declared structs (e.g. `struct SNAME`, `struct SNAME_vtabv`, etc.).
 * - `PFX`: Prefix of all functions, including implementation detail ones.
 * - `TAG`: A unique integer tag that identifies this data structure.
 *
 * ## Internally used macros
 * - `IT_` prefix macros: used by other data structures. Should not be used directly.
 * - `EMBEDDED`: used by other data structures to automatically implement an iterator. When this
 * is enabled, it changes the input macros and prefixes them all with "IT_".
 *
 * ## Usage
 *
 * ```c
 * for (struct darray_iter it = da_iter_start(&darr); !da_iter_at_end(&it); da_iter_next(&it))
 * {
 *     printf("%s\t%s\n", da_iter_value(&it), da_iter_value(&it));
 * }
 * ```
 */

#include "cvx/fallback.h"

// clang-format off
#ifndef EMBEDDED
#ifndef V
#error "cvx/array_iterator.h requires V to be defined (the element type, e.g. #define V int)"
#endif
#ifndef SNAME
#error "cvx/array_iterator.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_cbuf)"
#endif
#ifndef PFX
#error "cvx/array_iterator.h requires PFX to be defined (the function prefix, e.g. #define PFX cb)"
#endif
#ifndef TAG
#error "cvx/array_iterator.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
#endif
#else // EMBEDDED
#ifndef IT_V
#error "cvx/array_iterator.h requires IT_V to be defined (the element type, e.g. #define IT_V int)"
#endif
#ifndef IT_SNAME
#error "cvx/array_iterator.h requires IT_SNAME to be defined (the struct name, e.g. #define IT_SNAME my_cbuf)"
#endif
#ifndef IT_PFX
#error "cvx/array_iterator.h requires IT_PFX to be defined (the function prefix, e.g. #define IT_PFX cb)"
#endif
#ifndef IT_TAG
#error "cvx/array_iterator.h requires IT_TAG to be defined (a unique integer tag, e.g. #define IT_TAG 1)"
#endif
#endif // EMBEDDED
// Options
#if defined(SPARSE) && defined(CIRCULAR)
#error "cvx/array_iterator.h can only define one macro option at a time"
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
