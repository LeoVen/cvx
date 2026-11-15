#include "cvxt_alloc.h"
#include <stdio.h>
#include <stdlib.h>

int cvxt_alloc_fail_counter = 0;
int cvxt_alloc_fail_after = -1;

void cvxt_malloc_set(int fail_counter, int fail_after)
{
    cvxt_alloc_fail_counter = fail_counter;
    cvxt_alloc_fail_after = fail_after;
}

void *cvxt_malloc(size_t size)
{
    if (cvxt_alloc_fail_after >= 0 && cvxt_alloc_fail_counter >= cvxt_alloc_fail_after)
        return NULL;

    cvxt_alloc_fail_counter++;

    return malloc(size);
}

void *cvxt_realloc(void *ptr, size_t size)
{
    if (cvxt_alloc_fail_after >= 0 && cvxt_alloc_fail_counter >= cvxt_alloc_fail_after)
        return NULL;

    cvxt_alloc_fail_counter++;

    return realloc(ptr, size);
}
