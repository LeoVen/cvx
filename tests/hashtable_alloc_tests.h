#ifndef HASHTABLE_ALLOC_TESTS_H
#define HASHTABLE_ALLOC_TESTS_H

// alloc.h must be included before implementations.h.
#include "tests/alloc.h"

#include "tests/cvxtest.h"
#include "tests/implementations.h"

/* ---- new ---- */

static void test_ht_int_alloc_new(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    struct ht_int_int *col = ht_int_new();
    CVXCHECK(t, col == NULL);
    CVX_MALLOC_RESET();
}

/* ---- new_with ---- */

static void test_ht_int_alloc_new_with(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    CVXCHECK(t, col == NULL);
    CVX_MALLOC_RESET();
}

/* ---- insert: buffer malloc fails on first insert ---- */

static void test_ht_int_alloc_insert_buffer_fails(struct cvxtest *t)
{
    // 1 alloc for the struct succeeds, then the buffer malloc fails.
    CVX_MALLOC_FAIL_AFTER(1);
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    CVXCHECK(t, col != NULL);
    if (!col)
    {
        CVX_MALLOC_RESET();
        return;
    }

    bool ok = ht_int_insert(col, 1, 10);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, col->super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, ht_int_count(col) == 0);

    CVX_MALLOC_RESET();
    ht_int_drop(col);
}

/* ---- insert: resize malloc fails ---- */

static void test_ht_int_alloc_insert_resize_fails(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    // First prime capacity is 53.  Load threshold is 53 * 0.7 = 37.1, so a
    // resize is triggered when count >= 38 at the start of an insert.
    // Insert 38 entries (count becomes 38 after each); the 39th insert will
    // see count=38 >= 37.1 and trigger a resize.
    for (int i = 0; i < 38; i++)
    {
        bool ok = ht_int_insert(col, i, i);
        CVXCHECK(t, ok == true);
    }

    CVXCHECK(t, ht_int_count(col) == 38);

    // Make the next malloc (which will be the resize buffer) fail.
    CVX_MALLOC_FAIL_NEXT();
    bool ok = ht_int_insert(col, 1000, 999);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, col->super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, ht_int_count(col) == 38); // unchanged

    CVX_MALLOC_RESET();
    ht_int_drop(col);
}

/* ---- clone: struct allocation fails ---- */

static void test_ht_int_alloc_clone_struct_fails(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    ht_int_insert(col, 1, 10);

    CVX_MALLOC_FAIL_NEXT();
    struct ht_int_int *clone = ht_int_clone(col);
    CVXCHECK(t, clone == NULL);

    CVX_MALLOC_RESET();
    ht_int_drop(col);
}

/* ---- clone: buffer allocation fails ---- */

static void test_ht_int_alloc_clone_buffer_fails(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    ht_int_insert(col, 1, 10);

    // 1 alloc for the clone struct succeeds, then buffer malloc fails.
    CVX_MALLOC_FAIL_AFTER(1);
    struct ht_int_int *clone = ht_int_clone(col);
    CVXCHECK(t, clone == NULL);

    CVX_MALLOC_RESET();
    ht_int_drop(col);
}

/* ---- copy: buffer allocation fails ---- */

static void test_ht_int_alloc_copy_buffer_fails(struct cvxtest *t)
{
    struct ht_int_int orig = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&orig, 1, 10);

    CVX_MALLOC_FAIL_NEXT();
    struct ht_int_int copy = ht_int_copy(&orig);

    CVXCHECK(t, copy.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, copy.count == 0);
    CVXCHECK(t, copy.buffer == NULL);

    CVX_MALLOC_RESET();
    ht_int_clear(&orig);
}

/* ---- init_with: buffer allocation fails ---- */

static void test_ht_int_alloc_init_with_fails(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    struct ht_int_int s = ht_int_init_with(ht_int_vtabk, NULL, 10);

    CVXCHECK(t, s.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, s.buffer == NULL);
    CVXCHECK(t, s.capacity == 0);

    CVX_MALLOC_RESET();
}

/* ---- iter_start allocation fails ---- */

static void test_ht_int_alloc_iter_start_fails(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);

    CVX_MALLOC_FAIL_NEXT();
    struct ht_int_int_iter *iter = ht_int_iter_start(col);
    CVXCHECK(t, iter == NULL);

    CVX_MALLOC_RESET();
    ht_int_drop(col);
}

/* ---- iter_end allocation fails ---- */

static void test_ht_int_alloc_iter_end_fails(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);

    CVX_MALLOC_FAIL_NEXT();
    struct ht_int_int_iter *iter = ht_int_iter_end(col);
    CVXCHECK(t, iter == NULL);

    CVX_MALLOC_RESET();
    ht_int_drop(col);
}

/* ---- runner ---- */

static struct cvxresult run_hashtable_alloc_tests(void)
{
    struct cvxtest t = { 0 };

    printf("hashtable (alloc errors)\n");

    CVXRUN(&t, test_ht_int_alloc_copy_buffer_fails);
    CVXRUN(&t, test_ht_int_alloc_init_with_fails);

    CVXRUN(&t, test_ht_int_alloc_new);
    CVXRUN(&t, test_ht_int_alloc_new_with);

    CVXRUN(&t, test_ht_int_alloc_clone_struct_fails);
    CVXRUN(&t, test_ht_int_alloc_clone_buffer_fails);

    CVXRUN(&t, test_ht_int_alloc_insert_buffer_fails);
    CVXRUN(&t, test_ht_int_alloc_insert_resize_fails);

    CVXRUN(&t, test_ht_int_alloc_iter_start_fails);
    CVXRUN(&t, test_ht_int_alloc_iter_end_fails);

    return CVXSUMMARY(&t);
}

#endif /* HASHTABLE_ALLOC_TESTS_H */
