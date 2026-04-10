#ifndef INTERVAL_SET_ALLOC_TESTS_H
#define INTERVAL_SET_ALLOC_TESTS_H

// alloc.h must be included before implementations.h.
#include "tests/alloc.h"

#include "tests/cvxtest.h"
#include "tests/implementations.h"

/* ---- new ---- */

static void test_is_int_alloc_new(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    cvx_container *col = is_int_new();
    CVXCHECK(t, col == NULL);
    CVX_MALLOC_RESET();
}

/* ---- new_with ---- */

static void test_is_int_alloc_new_with(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);
    CVXCHECK(t, col == NULL);
    CVX_MALLOC_RESET();
}

/* ---- add: buffer allocation fails on first add ---- */

static void test_is_int_alloc_add_buffer_fails(struct cvxtest *t)
{
    // 1 alloc for the struct succeeds, then the buffer malloc fails.
    CVX_MALLOC_FAIL_AFTER(1);
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);
    CVXCHECK(t, col != NULL);
    if (!col)
    {
        CVX_MALLOC_RESET();
        return;
    }

    is_int_add(col, 1, 5);
    CVXCHECK(t, col->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, is_int_count(col) == 0);

    CVX_MALLOC_RESET();
    is_int_drop(col);
}

/* ---- add: buffer reallocation fails ---- */

static void test_is_int_alloc_add_realloc_fails(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    // Fill 16 slots (the initial capacity).
    for (int i = 0; i < 16; i++)
        is_int_add(col, i * 10, i * 10 + 5);

    CVXCHECK(t, is_int_count(col) == 16);

    // The next add triggers a realloc; make it fail.
    CVX_MALLOC_FAIL_NEXT();
    is_int_add(col, 200, 205);
    CVXCHECK(t, col->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, is_int_count(col) == 16);

    CVX_MALLOC_RESET();
    is_int_drop(col);
}

/* ---- clone: struct allocation fails ---- */

static void test_is_int_alloc_clone_struct_fails(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);
    is_int_add(col, 1, 5);

    CVX_MALLOC_FAIL_NEXT();
    cvx_container *clone = is_int_clone(col);
    CVXCHECK(t, clone == NULL);

    CVX_MALLOC_RESET();
    is_int_drop(col);
}

/* ---- clone: buffer allocation fails ---- */

static void test_is_int_alloc_clone_buffer_fails(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);
    is_int_add(col, 1, 5);

    // 1 alloc for the clone struct succeeds, then buffer malloc fails.
    CVX_MALLOC_FAIL_AFTER(1);
    cvx_container *clone = is_int_clone(col);
    CVXCHECK(t, clone == NULL);

    CVX_MALLOC_RESET();
    is_int_drop(col);
}

/* ---- iter_start allocation fails ---- */

static void test_is_int_alloc_iter_start_fails(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    CVX_MALLOC_FAIL_NEXT();
    cvx_container *iter = is_int_iter_start(col);
    CVXCHECK(t, iter == NULL);

    CVX_MALLOC_RESET();
    is_int_drop(col);
}

/* ---- iter_end allocation fails ---- */

static void test_is_int_alloc_iter_end_fails(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    CVX_MALLOC_FAIL_NEXT();
    cvx_container *iter = is_int_iter_end(col);
    CVXCHECK(t, iter == NULL);

    CVX_MALLOC_RESET();
    is_int_drop(col);
}

/* ---- copy: buffer allocation fails ---- */

static void test_is_int_alloc_copy_buffer_fails(struct cvxtest *t)
{
    struct iset_int orig = is_int_init(is_int_vtabv_comp_only);
    is_int_add(cvx_col(orig), 1, 5);

    CVX_MALLOC_FAIL_NEXT();
    struct iset_int copy = is_int_copy(&orig);

    CVXCHECK(t, ((cvx_container *)&copy)->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, copy.count == 0);
    CVXCHECK(t, copy.buffer == NULL);

    CVX_MALLOC_RESET();
    is_int_clear(cvx_col(orig));
}

/* ---- runner ---- */

static int run_interval_set_alloc_tests(void)
{
    struct cvxtest t = { 0 };

    printf("interval_set (alloc errors)\n");

    CVXRUN(&t, test_is_int_alloc_copy_buffer_fails);

    CVXRUN(&t, test_is_int_alloc_new);
    CVXRUN(&t, test_is_int_alloc_new_with);

    CVXRUN(&t, test_is_int_alloc_clone_struct_fails);
    CVXRUN(&t, test_is_int_alloc_clone_buffer_fails);

    CVXRUN(&t, test_is_int_alloc_add_buffer_fails);
    CVXRUN(&t, test_is_int_alloc_add_realloc_fails);

    CVXRUN(&t, test_is_int_alloc_iter_start_fails);
    CVXRUN(&t, test_is_int_alloc_iter_end_fails);

    return CVXSUMMARY(&t);
}

#endif /* INTERVAL_SET_ALLOC_TESTS_H */
