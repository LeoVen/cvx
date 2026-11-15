#ifndef CVXT_ALLOC_H
#define CVXT_ALLOC_H

#include <stddef.h>

extern int cvxt_alloc_fail_counter;
extern int cvxt_alloc_fail_after;

void cvxt_malloc_set(int fail_counter, int fail_after);
void *cvxt_malloc(size_t size);
void *cvxt_realloc(void *ptr, size_t size);

// clang-format off
// Fail on the very next allocation.
#define CVXT_MALLOC_FAIL_NEXT() do { cvxt_malloc_set(0, 0); } while (0)
// Let n allocations succeed, then fail on the (n+1)-th.
#define CVXT_MALLOC_FAIL_AFTER(n) do { cvxt_malloc_set(0, (n)); } while (0)
// Disable failure injection and reset counter.
#define CVXT_MALLOC_RESET() do { cvxt_malloc_set(0, -1); } while (0)
// Number of successful allocations since last reset.
#define CVXT_MALLOC_COUNT() cvxt_alloc_fail_counter
// clang-format on

#endif /* CVXT_ALLOC_H */
