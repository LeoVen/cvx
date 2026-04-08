#ifndef CVX_ALLOC_TEST_H
#define CVX_ALLOC_TEST_H

#include <stdlib.h>

// Counts successful allocations since last reset.
int cvx_malloc_fail_counter = 0;
// Fail once counter reaches this value. -1 = disabled.
int cvx_malloc_fail_after = -1;

static inline void *cvx_malloc(size_t size)
{
    if (cvx_malloc_fail_after >= 0 && cvx_malloc_fail_counter >= cvx_malloc_fail_after)
        return NULL;

    cvx_malloc_fail_counter++;

    return malloc(size);
}

static inline void *cvx_realloc(void *ptr, size_t size)
{
    if (cvx_malloc_fail_after >= 0 && cvx_malloc_fail_counter >= cvx_malloc_fail_after)
        return NULL;

    cvx_malloc_fail_counter++;

    return realloc(ptr, size);
}

// NOTE: calloc is intentionally replaced with malloc (no zero-init) to catch
// any code that relies on calloc for initialization instead of setting fields
// explicitly.
// clang-format off
#define malloc(size)           cvx_malloc(size)
#define calloc(amount, size)   cvx_malloc((amount) * (size))
#define realloc(ptr, size)     cvx_realloc(ptr, size)
// clang-format on

// Fail on the very next allocation.
#define CVX_MALLOC_FAIL_NEXT() \
    do \
    { \
        cvx_malloc_fail_after = 0; \
        cvx_malloc_fail_counter = 0; \
    } while (0)

// Let n allocations succeed, then fail on the (n+1)-th.
#define CVX_MALLOC_FAIL_AFTER(n) \
    do \
    { \
        cvx_malloc_fail_after = (n); \
        cvx_malloc_fail_counter = 0; \
    } while (0)

// Disable failure injection and reset counter.
#define CVX_MALLOC_RESET() \
    do \
    { \
        cvx_malloc_fail_after = -1; \
        cvx_malloc_fail_counter = 0; \
    } while (0)

// Number of successful allocations since last reset.
#define CVX_MALLOC_COUNT() cvx_malloc_fail_counter

#endif /* CVX_ALLOC_TEST_H */
