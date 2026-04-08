#ifndef DYNAMIC_ARRAY_ALLOC_TESTS_H
#define DYNAMIC_ARRAY_ALLOC_TESTS_H

// alloc.h must be included before implementations.h so that the #define
// malloc / calloc / realloc macros are in effect when dynamic_array.h is compiled.
#include "tests/alloc.h"

#include "tests/cvxtest.h"
#include "tests/implementations.h"

/* ---- new ---- */

// da_int_new() performs one allocation (the struct). Failing it returns NULL.
static void test_da_int_alloc_new(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    cvx_container *d = da_int_new();
    CVXCHECK(t, d == NULL);
    CVX_MALLOC_RESET();
}

/* ---- new_with ---- */

// da_int_new_with() performs two allocations: the struct, then the buffer.
// Failing the struct returns NULL.
static void test_da_int_alloc_new_with_struct_fails(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    cvx_container *d = da_int_new_with(NULL, 4);
    CVXCHECK(t, d == NULL);
    CVX_MALLOC_RESET();
}

// Failing the buffer must free the struct and return NULL (no leak).
static void test_da_int_alloc_new_with_buffer_fails(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_AFTER(1);
    cvx_container *d = da_int_new_with(NULL, 4);
    CVXCHECK(t, d == NULL);
    CVX_MALLOC_RESET();
}

/* ---- push: initial buffer allocation ---- */

// The first push on an empty da_int_new() container triggers the initial
// buffer malloc. Failing it must set CVX_FLAG_ALLOC; count stays 0.
static void test_da_int_alloc_push_back_initial_buffer(struct cvxtest *t)
{
    cvx_container *d = da_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    CVX_MALLOC_FAIL_NEXT();
    da_int_push_back(d, 42);
    CVXCHECK(t, d->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, da_int_count(d) == 0);

    CVX_MALLOC_RESET();
    da_int_drop(d);
}

static void test_da_int_alloc_push_front_initial_buffer(struct cvxtest *t)
{
    cvx_container *d = da_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    CVX_MALLOC_FAIL_NEXT();
    da_int_push_front(d, 42);
    CVXCHECK(t, d->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, da_int_count(d) == 0);

    CVX_MALLOC_RESET();
    da_int_drop(d);
}

static void test_da_int_alloc_push_at_initial_buffer(struct cvxtest *t)
{
    cvx_container *d = da_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    CVX_MALLOC_FAIL_NEXT();
    da_int_push_at(d, 42, 0);
    CVXCHECK(t, d->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, da_int_count(d) == 0);

    CVX_MALLOC_RESET();
    da_int_drop(d);
}

/* ---- push: realloc (buffer growth) ---- */

// Use new_with(capacity=2) so the realloc path is reached after exactly 2
// pushes without any additional mallocs to count.
// On realloc failure the existing elements and capacity must be preserved.
static void test_da_int_alloc_push_back_realloc(struct cvxtest *t)
{
    cvx_container *d = da_int_new_with(NULL, 2);
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    da_int_push_back(d, 10);
    da_int_push_back(d, 20);
    CVXCHECK(t, da_int_count(d) == 2);

    CVX_MALLOC_FAIL_NEXT();
    da_int_push_back(d, 30);
    CVXCHECK(t, d->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, da_int_count(d) == 2);
    CVXCHECK(t, da_int_capacity(d) == 2); // capacity must not be corrupted
    CVXCHECK(t, da_int_front(d) == 10);
    CVXCHECK(t, da_int_back(d) == 20);

    CVX_MALLOC_RESET();
    da_int_drop(d);
}

static void test_da_int_alloc_push_front_realloc(struct cvxtest *t)
{
    cvx_container *d = da_int_new_with(NULL, 2);
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    da_int_push_back(d, 10);
    da_int_push_back(d, 20);

    CVX_MALLOC_FAIL_NEXT();
    da_int_push_front(d, 30);
    CVXCHECK(t, d->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, da_int_count(d) == 2);
    CVXCHECK(t, da_int_capacity(d) == 2);
    CVXCHECK(t, da_int_front(d) == 10);
    CVXCHECK(t, da_int_back(d) == 20);

    CVX_MALLOC_RESET();
    da_int_drop(d);
}

static void test_da_int_alloc_push_at_realloc(struct cvxtest *t)
{
    cvx_container *d = da_int_new_with(NULL, 2);
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    da_int_push_back(d, 10);
    da_int_push_back(d, 20);

    CVX_MALLOC_FAIL_NEXT();
    da_int_push_at(d, 30, 1);
    CVXCHECK(t, d->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, da_int_count(d) == 2);
    CVXCHECK(t, da_int_capacity(d) == 2);
    CVXCHECK(t, da_int_front(d) == 10);
    CVXCHECK(t, da_int_back(d) == 20);

    CVX_MALLOC_RESET();
    da_int_drop(d);
}

/* ---- clone ---- */

// clone() allocates: (1) new struct via _new(), (2) buffer if count > 0.
// Failing the struct returns NULL.
static void test_da_int_alloc_clone_struct_fails(struct cvxtest *t)
{
    cvx_container *d = da_int_new_with(NULL, 4);
    CVXCHECK(t, d != NULL);
    if (!d)
        return;
    da_int_push_back(d, 10);
    da_int_push_back(d, 20);

    CVX_MALLOC_FAIL_NEXT();
    cvx_container *copy = da_int_clone(d);
    CVXCHECK(t, copy == NULL);

    CVX_MALLOC_RESET();
    da_int_drop(d);
}

// Failing the buffer allocation inside clone must free the cloned struct and
// return NULL.
static void test_da_int_alloc_clone_buffer_fails(struct cvxtest *t)
{
    cvx_container *d = da_int_new_with(NULL, 4);
    CVXCHECK(t, d != NULL);
    if (!d)
        return;
    da_int_push_back(d, 10);
    da_int_push_back(d, 20);

    // Allow 1 alloc (the clone struct), fail the buffer malloc.
    CVX_MALLOC_FAIL_AFTER(1);
    cvx_container *copy = da_int_clone(d);
    CVXCHECK(t, copy == NULL);

    CVX_MALLOC_RESET();
    da_int_drop(d);
}

// A successful clone must be independent of the original.
static void test_da_int_alloc_clone_success(struct cvxtest *t)
{
    cvx_container *d = da_int_new_with(NULL, 4);
    CVXCHECK(t, d != NULL);
    if (!d)
        return;
    da_int_push_back(d, 10);
    da_int_push_back(d, 20);

    cvx_container *copy = da_int_clone(d);
    CVXCHECK(t, copy != NULL);
    if (!copy)
    {
        da_int_drop(d);
        return;
    }

    da_int_push_back(d, 30);
    CVXCHECK(t, da_int_count(copy) == 2);
    CVXCHECK(t, da_int_front(copy) == 10);
    CVXCHECK(t, da_int_back(copy) == 20);

    da_int_drop(copy);
    da_int_drop(d);
}

/* ---- init_with (stack-allocated) ---- */

// da_int_init_with() allocates only the buffer (struct is on the stack).
// Failing that calloc must return a struct with CVX_FLAG_ALLOC and NULL buffer.
static void test_da_int_alloc_init_with_buffer_fails(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    struct dynamic_array_int da = da_int_init_with(NULL, 4);
    CVXCHECK(t, da.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, da.buffer == NULL);
    CVX_MALLOC_RESET();
    // Stack-allocated; buffer is NULL so nothing to free.
}

/* ---- copy (stack-allocated) ---- */

// da_int_copy() allocates only the buffer (struct is on the stack).
// Failing that malloc must return a struct with CVX_FLAG_ALLOC and NULL buffer.
static void test_da_int_alloc_copy_buffer_fails(struct cvxtest *t)
{
    cvx_container *src = da_int_new_with(NULL, 4);
    CVXCHECK(t, src != NULL);
    if (!src)
        return;

    da_int_push_back(src, 10);
    da_int_push_back(src, 20);

    struct dynamic_array_int *self = (struct dynamic_array_int *)src;

    CVX_MALLOC_FAIL_NEXT();
    struct dynamic_array_int copy = da_int_copy(self);
    CVXCHECK(t, copy.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, copy.buffer == NULL);

    CVX_MALLOC_RESET();
    da_int_drop(src);
}

/* ---- iter_start / iter_end ---- */

static void test_da_int_alloc_iter_start_fails(struct cvxtest *t)
{
    cvx_container *d = da_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    CVX_MALLOC_FAIL_NEXT();
    cvx_container *it = da_int_iter_start(d);
    CVXCHECK(t, it == NULL);

    CVX_MALLOC_RESET();
    da_int_drop(d);
}

static void test_da_int_alloc_iter_end_fails(struct cvxtest *t)
{
    cvx_container *d = da_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    CVX_MALLOC_FAIL_NEXT();
    cvx_container *it = da_int_iter_end(d);
    CVXCHECK(t, it == NULL);

    CVX_MALLOC_RESET();
    da_int_drop(d);
}

/* ---- runner ---- */

static int run_dynamic_array_alloc_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dynamic_array (alloc errors)\n");

    CVXRUN(&t, test_da_int_alloc_new);

    CVXRUN(&t, test_da_int_alloc_new_with_struct_fails);
    CVXRUN(&t, test_da_int_alloc_new_with_buffer_fails);

    CVXRUN(&t, test_da_int_alloc_push_back_initial_buffer);
    CVXRUN(&t, test_da_int_alloc_push_front_initial_buffer);
    CVXRUN(&t, test_da_int_alloc_push_at_initial_buffer);

    CVXRUN(&t, test_da_int_alloc_push_back_realloc);
    CVXRUN(&t, test_da_int_alloc_push_front_realloc);
    CVXRUN(&t, test_da_int_alloc_push_at_realloc);

    CVXRUN(&t, test_da_int_alloc_init_with_buffer_fails);
    CVXRUN(&t, test_da_int_alloc_copy_buffer_fails);

    CVXRUN(&t, test_da_int_alloc_clone_struct_fails);
    CVXRUN(&t, test_da_int_alloc_clone_buffer_fails);
    CVXRUN(&t, test_da_int_alloc_clone_success);

    CVXRUN(&t, test_da_int_alloc_iter_start_fails);
    CVXRUN(&t, test_da_int_alloc_iter_end_fails);

    return CVXSUMMARY(&t);
}

#endif /* DYNAMIC_ARRAY_ALLOC_TESTS_H */
