/// buffer_iterator.h
///
/// Status
///
///   [ ] concept
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
/// Parameters:
/// - NULLCHECKS: if enabled, items are checked against NULL and skipped if so
/// - CIRCULAR: if enabled, the pointer wraps around the buffer's capacity

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

struct ITERATOR
{
    cvx_container super;
    size_t capacity;
    size_t pointer;
    size_t index;
    T *buffer;
};

/**
 * Create a new Buffer Iterator
 *
 * @param buffer A pointer to the start of the buffer
 * @param capacity Total buffer capacity
 * @param index Where to start iteration
 *
 * @return A stack allocated buffer iterator
 */
struct ITERATOR FUNC(_init_iter)(T *buffer, size_t capacity, size_t index)
{
    struct ITERATOR result = { 0 };
    result.buffer = buffer;
    result.capacity = capacity;
    result.pointer = index;
    return (struct ITERATOR){
        .buffer = buffer,
        .capacity = capacity,
        .index = 0,
        .super = { .flag = CVX_FLAG_OK, .tag = ITER_TAG },
    };
}
