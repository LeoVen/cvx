#ifndef INTERVAL_MAP_ALLOC_TESTS_H
#define INTERVAL_MAP_ALLOC_TESTS_H

// alloc.h must be included before implementations.h.
#include "tests/alloc.h"

#include "tests/cvxtest.h"
#include "tests/implementations.h"

/* ---- new ---- */

static void test_im_int_alloc_new(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    cvx_container *col = im_int_new();
    CVXCHECK(t, col == NULL);
    CVX_MALLOC_RESET();
}

/* ---- new_with ---- */

static void test_im_int_alloc_new_with(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);
    CVXCHECK(t, col == NULL);
    CVX_MALLOC_RESET();
}

/* ---- put: buffer malloc fails on first put ---- */

static void test_im_int_alloc_add_buffer_fails(struct cvxtest *t)
{
    // 1 alloc for the struct succeeds, then the buffer malloc fails.
    CVX_MALLOC_FAIL_AFTER(1);
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);
    CVXCHECK(t, col != NULL);
    if (!col)
    {
        CVX_MALLOC_RESET();
        return;
    }

    im_int_add(col, 1, 5, 10);
    CVXCHECK(t, col->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, im_int_count(col) == 0);

    CVX_MALLOC_RESET();
    im_int_drop(col);
}

/* ---- put: buffer reallocation fails ---- */

static void test_im_int_alloc_add_realloc_fails(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    // Fill 15 disjoint slots (capacity starts at 16 after first put).
    // After 15 puts: count=15, capacity=16.
    // Next put needs capacity >= 17 → triggers realloc.
    for (int i = 0; i < 15; i++)
        im_int_add(col, i * 10, i * 10 + 5, i);

    CVXCHECK(t, im_int_count(col) == 15);

    CVX_MALLOC_FAIL_NEXT();
    im_int_add(col, 200, 205, 99);
    CVXCHECK(t, col->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, im_int_count(col) == 15);

    CVX_MALLOC_RESET();
    im_int_drop(col);
}

/* ---- clone: struct allocation fails ---- */

static void test_im_int_alloc_clone_struct_fails(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);
    im_int_add(col, 1, 5, 10);

    CVX_MALLOC_FAIL_NEXT();
    cvx_container *clone = im_int_clone(col);
    CVXCHECK(t, clone == NULL);

    CVX_MALLOC_RESET();
    im_int_drop(col);
}

/* ---- clone: buffer allocation fails ---- */

static void test_im_int_alloc_clone_buffer_fails(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);
    im_int_add(col, 1, 5, 10);

    // 1 alloc for the clone struct succeeds, then buffer malloc fails.
    CVX_MALLOC_FAIL_AFTER(1);
    cvx_container *clone = im_int_clone(col);
    CVXCHECK(t, clone == NULL);

    CVX_MALLOC_RESET();
    im_int_drop(col);
}

/* ---- iter_start allocation fails ---- */

static void test_im_int_alloc_iter_start_fails(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    CVX_MALLOC_FAIL_NEXT();
    cvx_container *iter = im_int_iter_start(col);
    CVXCHECK(t, iter == NULL);

    CVX_MALLOC_RESET();
    im_int_drop(col);
}

/* ---- iter_end allocation fails ---- */

static void test_im_int_alloc_iter_end_fails(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    CVX_MALLOC_FAIL_NEXT();
    cvx_container *iter = im_int_iter_end(col);
    CVXCHECK(t, iter == NULL);

    CVX_MALLOC_RESET();
    im_int_drop(col);
}

/* ---- copy: buffer allocation fails ---- */

static void test_im_int_alloc_copy_buffer_fails(struct cvxtest *t)
{
    struct imap_int_int orig = im_int_init(im_int_vtabk, NULL);
    im_int_add(cvx_col(orig), 1, 5, 10);

    CVX_MALLOC_FAIL_NEXT();
    struct imap_int_int copy = im_int_copy(&orig);

    CVXCHECK(t, ((cvx_container *)&copy)->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, copy.count == 0);
    CVXCHECK(t, copy.buffer == NULL);

    CVX_MALLOC_RESET();
    im_int_clear(cvx_col(orig));
}

/* ---- runner ---- */

static int run_interval_map_alloc_tests(void)
{
    struct cvxtest t = { 0 };

    printf("interval_map (alloc errors)\n");

    CVXRUN(&t, test_im_int_alloc_copy_buffer_fails);

    CVXRUN(&t, test_im_int_alloc_new);
    CVXRUN(&t, test_im_int_alloc_new_with);

    CVXRUN(&t, test_im_int_alloc_clone_struct_fails);
    CVXRUN(&t, test_im_int_alloc_clone_buffer_fails);

    CVXRUN(&t, test_im_int_alloc_add_buffer_fails);
    CVXRUN(&t, test_im_int_alloc_add_realloc_fails);

    CVXRUN(&t, test_im_int_alloc_iter_start_fails);
    CVXRUN(&t, test_im_int_alloc_iter_end_fails);

    return CVXSUMMARY(&t);
}

#endif /* INTERVAL_MAP_ALLOC_TESTS_H */
