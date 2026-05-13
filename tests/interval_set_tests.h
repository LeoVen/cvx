#ifndef INTERVAL_SET_TESTS_H
#define INTERVAL_SET_TESTS_H

#include "cvx/flags.h"
#include "tests/alloc.h"
#include "tests/cvxtest.h"
#include "tests/cvxtestutils.h"
#include "tests/implementations.h"

/* Helper: stack-allocate [1,5) [10,15) [20,25) */
static void is_int_fill3(struct iset_int *col)
{
    is_int_add(col, 1, 5);
    is_int_add(col, 10, 15);
    is_int_add(col, 20, 25);
}

/* ---- init ---- */

static void test_is_int_init(struct cvxtest *t)
{
    struct iset_int s;
    is_int_init(&s, is_int_vtabv_comp_only);
    CVXCHECK(t, s.super.tag == 55);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, s.count == 0);
    CVXCHECK(t, s.capacity == 0);
    CVXCHECK(t, s.buffer == NULL);
    CVXCHECK(t, s.vtabv == is_int_vtabv_comp_only);

    struct iset_int s_null;
    is_int_init(&s_null, NULL);
    CVXCHECK(t, s_null.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, s_null.super.tag == 0);

    struct iset_int_vtabv vtabv = { 0 };
    struct iset_int s_nocomp;
    is_int_init(&s_nocomp, &vtabv);
    CVXCHECK(t, s_nocomp.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, s_nocomp.super.tag == 0);
}

/* ---- clone ---- */

static void test_is_int_clone(struct cvxtest *t)
{
    struct iset_int orig;
    is_int_init(&orig, is_int_vtabv_comp_only);

    struct iset_int clone_empty;
    is_int_clone(&orig, &clone_empty);
    CVXCHECK(t, clone_empty.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(&clone_empty) == 0);
    is_int_drop(&clone_empty);

    is_int_add(&orig, 1, 5);
    is_int_add(&orig, 10, 15);

    struct iset_int clone_vals;
    is_int_clone(&orig, &clone_vals);
    CVXCHECK(t, clone_vals.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(&clone_vals) == 2);
    CVXCHECK(t, is_int_contains(&clone_vals, 3) == true);
    CVXCHECK(t, is_int_contains(&clone_vals, 12) == true);
    CVXCHECK(t, is_int_contains(&clone_vals, 7) == false);

    is_int_add(&orig, 6, 9);
    CVXCHECK(t, is_int_count(&clone_vals) == 2);

    CVX_TEST_COUNTER_CLONE_RESET();
    struct iset_int clone_vtab;
    struct iset_int orig_full;
    is_int_init(&orig_full, is_int_vtabv_full);
    is_int_add(&orig_full, 1, 5);
    is_int_add(&orig_full, 10, 15);
    is_int_clone(&orig_full, &clone_vtab);
    CVX_TEST_COUNTER_CLONE(t, 4);
    CVXCHECK(t, is_int_count(&clone_vtab) == 2);
    is_int_drop(&orig_full);
    is_int_drop(&clone_vtab);

    struct iset_int orig_alloc;
    is_int_init(&orig_alloc, is_int_vtabv_comp_only);
    is_int_add(&orig_alloc, 1, 5);
    CVX_MALLOC_FAIL_NEXT();
    struct iset_int clone_alloc;
    is_int_clone(&orig_alloc, &clone_alloc);
    CVXCHECK(t, clone_alloc.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, orig_alloc.super.flag == CVX_FLAG_ALLOC);
    CVX_MALLOC_RESET();
    is_int_drop(&orig_alloc);

    MAKE_INVALID_CONTAINER(col);
    struct iset_int dummy = { 0 };
    is_int__proxy_clone(col, (cvx_container *)&dummy);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);

    is_int_drop(&orig);
    is_int_drop(&clone_vals);
}

/* ---- drop ---- */

static void test_is_int_drop(struct cvxtest *t)
{
    struct iset_int s;
    is_int_init(&s, is_int_vtabv_comp_only);
    is_int_drop(&s);
    CVXCHECK(t, s.buffer == NULL);
    CVXCHECK(t, s.count == 0);

    CVX_TEST_COUNTER_DROP_RESET();
    struct iset_int s_vtab;
    is_int_init(&s_vtab, is_int_vtabv_full);
    is_int_add(&s_vtab, 1, 5);
    is_int_add(&s_vtab, 10, 15);
    is_int_drop(&s_vtab);
    CVX_TEST_COUNTER_DROP(t, 4);

    is_int_drop(NULL);

    MAKE_INVALID_CONTAINER(col);
    is_int__proxy_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- count ---- */

static void test_is_int_count(struct cvxtest *t)
{
    struct iset_int s;
    is_int_init(&s, is_int_vtabv_comp_only);
    CVXCHECK(t, is_int_count(&s) == 0);
    is_int_add(&s, 1, 5);
    CVXCHECK(t, is_int_count(&s) == 1);
    is_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    size_t res = is_int__proxy_count(col);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- empty ---- */

static void test_is_int_empty(struct cvxtest *t)
{
    struct iset_int s;
    is_int_init(&s, is_int_vtabv_comp_only);
    CVXCHECK(t, is_int_empty(&s) == true);
    is_int_add(&s, 1, 3);
    CVXCHECK(t, is_int_empty(&s) == false);
    is_int_drop(&s);
    is_int_init(&s, is_int_vtabv_comp_only);
    CVXCHECK(t, is_int_empty(&s) == true);
    is_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    bool res = is_int__proxy_empty(col);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- add ---- */

static void test_is_int_add(struct cvxtest *t)
{
    struct iset_int s;
    is_int_init(&s, is_int_vtabv_comp_only);

    is_int_add(&s, 3, 7);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(&s) == 1);
    CVXCHECK(t, is_int_contains(&s, 3) == true);
    CVXCHECK(t, is_int_contains(&s, 6) == true);
    CVXCHECK(t, is_int_contains(&s, 7) == false);

    is_int_add(&s, 10, 14);
    CVXCHECK(t, is_int_count(&s) == 2);

    is_int_add(&s, 3, 8);
    CVXCHECK(t, is_int_count(&s) == 2);
    CVXCHECK(t, s.buffer[0].lo == 3 && s.buffer[0].hi == 8);

    is_int_drop(&s);
    is_int_init(&s, is_int_vtabv_comp_only);

    is_int_add(&s, 10, 12);
    is_int_add(&s, 12, 15);
    CVXCHECK(t, is_int_count(&s) == 1);
    CVXCHECK(t, is_int_contains(&s, 14) == true);
    CVXCHECK(t, is_int_contains(&s, 15) == false);

    is_int_drop(&s);
    is_int_init(&s, is_int_vtabv_comp_only);

    is_int_add(&s, 1, 4);
    is_int_add(&s, 5, 8);
    is_int_add(&s, 9, 12);
    CVXCHECK(t, is_int_count(&s) == 3);
    is_int_add(&s, 3, 10);
    CVXCHECK(t, is_int_count(&s) == 1);

    is_int_add(&s, 5, 5);
    CVXCHECK(t, s.super.flag == CVX_FLAG_INVALID);

    is_int_add(&s, 9, 3);
    CVXCHECK(t, s.super.flag == CVX_FLAG_INVALID);

    is_int_drop(&s);

    struct iset_int s_novtab;
    is_int_init(&s_novtab, NULL);
    is_int_add(&s_novtab, 1, 5);
    CVXCHECK(t, s_novtab.super.flag == CVX_FLAG_VTAB);

    MAKE_INVALID_CONTAINER(col);
    is_int__proxy_add(col, 1, 5);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);

    CVX_MALLOC_FAIL_NEXT();
    struct iset_int s_alloc;
    is_int_init(&s_alloc, is_int_vtabv_comp_only);
    is_int_add(&s_alloc, 1, 5);
    CVXCHECK(t, s_alloc.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, is_int_count(&s_alloc) == 0);
    CVX_MALLOC_RESET();
    is_int_drop(&s_alloc);

    struct iset_int s_realloc;
    is_int_init(&s_realloc, is_int_vtabv_comp_only);
    for (int i = 0; i < 16; i++)
        is_int_add(&s_realloc, i * 10, i * 10 + 5);
    CVXCHECK(t, is_int_count(&s_realloc) == 16);
    CVX_MALLOC_FAIL_NEXT();
    is_int_add(&s_realloc, 200, 205);
    CVXCHECK(t, s_realloc.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, is_int_count(&s_realloc) == 16);
    CVX_MALLOC_RESET();
    is_int_drop(&s_realloc);

    CVX_TEST_COUNTER_DROP_RESET();
    struct iset_int s_merge;
    is_int_init(&s_merge, is_int_vtabv_full);
    is_int_add(&s_merge, 1, 5);
    is_int_add(&s_merge, 10, 15);
    CVX_TEST_COUNTER_DROP_RESET();
    is_int_add(&s_merge, 3, 12);
    CVX_TEST_COUNTER_DROP(t, 2);
    CVXCHECK(t, is_int_count(&s_merge) == 1);
    is_int_drop(&s_merge);
}

/* ---- remove ---- */

static void test_is_int_remove(struct cvxtest *t)
{
    struct iset_int s;
    is_int_init(&s, is_int_vtabv_comp_only);

    is_int_add(&s, 3, 7);
    is_int_remove(&s, 3, 7);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(&s) == 0);

    is_int_add(&s, 1, 10);
    is_int_remove(&s, 3, 7);
    CVXCHECK(t, is_int_count(&s) == 2);
    CVXCHECK(t, is_int_contains(&s, 2) == true);
    CVXCHECK(t, is_int_contains(&s, 3) == false);
    CVXCHECK(t, is_int_contains(&s, 7) == true);

    is_int_drop(&s);
    is_int_init(&s, is_int_vtabv_comp_only);

    is_int_add(&s, 1, 10);
    is_int_remove(&s, 1, 5);
    CVXCHECK(t, is_int_count(&s) == 1);
    CVXCHECK(t, is_int_contains(&s, 1) == false);
    CVXCHECK(t, is_int_contains(&s, 5) == true);

    is_int_drop(&s);
    is_int_init(&s, is_int_vtabv_comp_only);

    is_int_add(&s, 1, 10);
    is_int_remove(&s, 6, 10);
    CVXCHECK(t, is_int_count(&s) == 1);
    CVXCHECK(t, is_int_contains(&s, 5) == true);
    CVXCHECK(t, is_int_contains(&s, 6) == false);

    is_int_drop(&s);
    is_int_init(&s, is_int_vtabv_comp_only);

    is_int_add(&s, 1, 5);
    is_int_add(&s, 8, 12);
    is_int_add(&s, 15, 20);
    is_int_remove(&s, 3, 17);
    CVXCHECK(t, is_int_count(&s) == 2);
    CVXCHECK(t, is_int_contains(&s, 2) == true);
    CVXCHECK(t, is_int_contains(&s, 3) == false);
    CVXCHECK(t, is_int_contains(&s, 17) == true);

    is_int_add(&s, 10, 14);
    is_int_remove(&s, 12, 12);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);

    is_int_add(&s, 1, 3);
    is_int_add(&s, 7, 10);
    is_int_remove(&s, 3, 7);
    CVXCHECK(t, is_int_count(&s) >= 2);

    is_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    is_int__proxy_remove(col, 1, 5);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);

    CVX_TEST_COUNTER_DROP_RESET();
    struct iset_int s_vtab;
    is_int_init(&s_vtab, is_int_vtabv_full);
    is_int_add(&s_vtab, 1, 10);
    CVX_TEST_COUNTER_DROP_RESET();
    is_int_remove(&s_vtab, 3, 7);
    CVX_TEST_COUNTER_DROP(t, 2);
    CVXCHECK(t, is_int_count(&s_vtab) == 2);
    is_int_drop(&s_vtab);
}

/* ---- contains ---- */

static void test_is_int_contains(struct cvxtest *t)
{
    struct iset_int s;
    is_int_init(&s, is_int_vtabv_comp_only);

    CVXCHECK(t, is_int_contains(&s, 0) == false);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);

    is_int_add(&s, 5, 10);
    CVXCHECK(t, is_int_contains(&s, 5) == true);
    CVXCHECK(t, is_int_contains(&s, 9) == true);
    CVXCHECK(t, is_int_contains(&s, 10) == false);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);

    is_int_add(&s, 20, 25);
    CVXCHECK(t, is_int_contains(&s, 15) == false);

    is_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    bool res = is_int__proxy_contains(col, 3);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- contains_interval ---- */

static void test_is_int_contains_interval(struct cvxtest *t)
{
    struct iset_int s;
    is_int_init(&s, is_int_vtabv_comp_only);

    is_int_add(&s, 1, 10);
    CVXCHECK(t, is_int_contains_interval(&s, 3, 7) == true);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_contains_interval(&s, 5, 5) == false);

    is_int_add(&s, 20, 25);
    CVXCHECK(t, is_int_contains_interval(&s, 3, 22) == false);

    is_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    bool res = is_int__proxy_contains_interval(col, 1, 5);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- overlaps ---- */

static void test_is_int_overlaps(struct cvxtest *t)
{
    struct iset_int s;
    is_int_init(&s, is_int_vtabv_comp_only);

    CVXCHECK(t, is_int_overlaps(&s, 1, 5) == false);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);

    is_int_add(&s, 1, 5);
    CVXCHECK(t, is_int_overlaps(&s, 3, 8) == true);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_overlaps(&s, 6, 10) == false);
    CVXCHECK(t, is_int_overlaps(&s, 5, 8) == false);

    is_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    bool res = is_int__proxy_overlaps(col, 1, 5);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_init_start ---- */

static void test_is_int_iter_init_start(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);

    struct iset_int_iter it = is_int_iter_init_start(&col);
    CVXCHECK(t, it.super.tag == (size_t)IS_ITER_TAG);
    CVXCHECK(t, it.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 0);
    CVXCHECK(t, it.target == &col);

    is_int_drop(&col);
    is_int_init(&col, is_int_vtabv_comp_only);
    struct iset_int_iter it_empty = is_int_iter_init_start(&col);
    CVXCHECK(t, it_empty.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it_empty.index == 0);
    is_int_drop(&col);
}

/* ---- iter_init_end ---- */

static void test_is_int_iter_init_end(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);

    struct iset_int_iter it = is_int_iter_init_end(&col);
    CVXCHECK(t, it.super.tag == (size_t)IS_ITER_TAG);
    CVXCHECK(t, it.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 3);

    is_int_drop(&col);
}

/* ---- iter_start / iter_end ---- */

static void test_is_int_iter_start(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);

    struct iset_int_iter *iter = is_int_iter_start(&col);
    CVXCHECK(t, iter != NULL);
    if (iter)
    {
        CVXCHECK(t, iter->super.tag == (size_t)IS_ITER_TAG);
        CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
        CVXCHECK(t, iter->index == 0);
        is_int_iter_drop(iter);
    }

    MAKE_INVALID_CONTAINER(bad);
    cvx_container *res = is_int__proxy_iter_start(bad);
    CVXCHECK(t, res == NULL);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);

    CVX_MALLOC_FAIL_NEXT();
    struct iset_int_iter *iter_fail = is_int_iter_start(&col);
    CVXCHECK(t, iter_fail == NULL);
    CVX_MALLOC_RESET();

    is_int_drop(&col);
}

/* ---- iter_end ---- */

static void test_is_int_iter_end(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);

    struct iset_int_iter *iter = is_int_iter_end(&col);
    CVXCHECK(t, iter != NULL);
    if (iter)
    {
        CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
        CVXCHECK(t, iter->index == 3);
        is_int_iter_drop(iter);
    }

    MAKE_INVALID_CONTAINER(bad);
    cvx_container *res = is_int__proxy_iter_end(bad);
    CVXCHECK(t, res == NULL);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);

    CVX_MALLOC_FAIL_NEXT();
    struct iset_int_iter *iter_fail = is_int_iter_end(&col);
    CVXCHECK(t, iter_fail == NULL);
    CVX_MALLOC_RESET();

    is_int_drop(&col);
}

/* ---- iter_drop ---- */

static void test_is_int_iter_drop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    is_int__proxy_iter_drop(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_at_start ---- */

static void test_is_int_iter_at_start(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_start(&col);

    CVXCHECK(t, is_int_iter_at_start(iter) == true);
    is_int_iter_next(iter);
    CVXCHECK(t, is_int_iter_at_start(iter) == false);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    bool res = is_int__proxy_iter_at_start(bad);
    CVXCHECK(t, res == false);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_at_end ---- */

static void test_is_int_iter_at_end(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_end(&col);

    CVXCHECK(t, is_int_iter_at_end(iter) == true);
    is_int_iter_prev(iter);
    CVXCHECK(t, is_int_iter_at_end(iter) == false);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    bool res = is_int__proxy_iter_at_end(bad);
    CVXCHECK(t, res == false);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_count ---- */

static void test_is_int_iter_count(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_start(&col);

    CVXCHECK(t, is_int_iter_count(iter) == 3);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    size_t res = is_int__proxy_iter_count(bad);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_to_start ---- */

static void test_is_int_iter_to_start(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_start(&col);

    is_int_iter_next(iter);
    is_int_iter_next(iter);
    is_int_iter_to_start(iter);
    CVXCHECK(t, is_int_iter_at_start(iter) == true);
    CVXCHECK(t, iter->index == 0);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    is_int__proxy_iter_to_start(bad);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_to_end ---- */

static void test_is_int_iter_to_end(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_start(&col);

    is_int_iter_to_end(iter);
    CVXCHECK(t, is_int_iter_at_end(iter) == true);
    CVXCHECK(t, iter->index == 3);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    is_int__proxy_iter_to_end(bad);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_next ---- */

static void test_is_int_iter_next(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_start(&col);

    CVXCHECK(t, is_int_iter_value_lo(iter) == 1);
    CVXCHECK(t, is_int_iter_value_hi(iter) == 5);
    is_int_iter_next(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 10);
    is_int_iter_next(iter);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 20);
    is_int_iter_next(iter);
    CVXCHECK(t, is_int_iter_at_end(iter) == true);
    is_int_iter_next(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    is_int__proxy_iter_next(bad);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_prev ---- */

static void test_is_int_iter_prev(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_end(&col);

    is_int_iter_prev(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 20);
    is_int_iter_prev(iter);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 10);
    is_int_iter_prev(iter);
    CVXCHECK(t, is_int_iter_at_start(iter) == true);
    is_int_iter_prev(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    is_int__proxy_iter_prev(bad);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_forward ---- */

static void test_is_int_iter_forward(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_start(&col);

    is_int_iter_forward(iter, 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 20);

    is_int_iter_to_start(iter);
    is_int_iter_forward(iter, 100);
    CVXCHECK(t, is_int_iter_at_end(iter) == true);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    is_int__proxy_iter_forward(bad, 1);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_backward ---- */

static void test_is_int_iter_backward(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_end(&col);

    is_int_iter_backward(iter, 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 10);

    is_int_iter_to_end(iter);
    is_int_iter_backward(iter, 100);
    CVXCHECK(t, is_int_iter_at_start(iter) == true);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    is_int__proxy_iter_backward(bad, 1);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_entry ---- */

static void test_is_int_iter_entry(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_start(&col);

    struct iset_int_entry e = is_int_iter_entry(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, e.lo == 1 && e.hi == 5);

    is_int_iter_to_end(iter);
    is_int_iter_entry(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    CVXCHECK(t, is_int_iter_value_lo(iter) == 0);

    is_int_iter_to_start(iter);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 1);
    CVXCHECK(t, is_int_iter_value_hi(iter) == 5);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    struct iset_int_entry res = is_int__proxy_iter_entry(bad);
    CVXCHECK(t, res.lo == 0 && res.hi == 0);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_value_lo / iter_value_hi ---- */

static void test_is_int_iter_value_lo_hi(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_start(&col);

    CVXCHECK(t, is_int_iter_value_lo(iter) == 1);
    CVXCHECK(t, is_int_iter_value_hi(iter) == 5);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad_lo);
    int rlo = is_int__proxy_iter_value_lo(bad_lo);
    CVXCHECK(t, rlo == 0);
    CVXCHECK(t, bad_lo->flag == CVX_FLAG_WRONG_TAG);

    MAKE_INVALID_CONTAINER(bad_hi);
    int rhi = is_int__proxy_iter_value_hi(bad_hi);
    CVXCHECK(t, rhi == 0);
    CVXCHECK(t, bad_hi->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_index ---- */

static void test_is_int_iter_index(struct cvxtest *t)
{
    struct iset_int col;
    is_int_init(&col, is_int_vtabv_comp_only);
    is_int_fill3(&col);
    struct iset_int_iter *iter = is_int_iter_start(&col);

    CVXCHECK(t, is_int_iter_index(iter) == 0);
    is_int_iter_next(iter);
    CVXCHECK(t, is_int_iter_index(iter) == 1);
    is_int_iter_next(iter);
    CVXCHECK(t, is_int_iter_index(iter) == 2);

    is_int_iter_drop(iter);
    is_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    size_t res = is_int__proxy_iter_index(bad);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static struct cvxresult run_interval_set_tests(void)
{
    struct cvxtest t = { 0 };

    printf("interval_set\n");

    CVXRUN(&t, test_is_int_init);
    CVXRUN(&t, test_is_int_clone);
    CVXRUN(&t, test_is_int_drop);
    CVXRUN(&t, test_is_int_count);
    CVXRUN(&t, test_is_int_empty);
    CVXRUN(&t, test_is_int_add);
    CVXRUN(&t, test_is_int_remove);
    CVXRUN(&t, test_is_int_contains);
    CVXRUN(&t, test_is_int_contains_interval);
    CVXRUN(&t, test_is_int_overlaps);
    CVXRUN(&t, test_is_int_iter_init_start);
    CVXRUN(&t, test_is_int_iter_init_end);
    CVXRUN(&t, test_is_int_iter_start);
    CVXRUN(&t, test_is_int_iter_end);
    CVXRUN(&t, test_is_int_iter_drop);
    CVXRUN(&t, test_is_int_iter_at_start);
    CVXRUN(&t, test_is_int_iter_at_end);
    CVXRUN(&t, test_is_int_iter_count);
    CVXRUN(&t, test_is_int_iter_to_start);
    CVXRUN(&t, test_is_int_iter_to_end);
    CVXRUN(&t, test_is_int_iter_next);
    CVXRUN(&t, test_is_int_iter_prev);
    CVXRUN(&t, test_is_int_iter_forward);
    CVXRUN(&t, test_is_int_iter_backward);
    CVXRUN(&t, test_is_int_iter_entry);
    CVXRUN(&t, test_is_int_iter_value_lo_hi);
    CVXRUN(&t, test_is_int_iter_index);

    return CVXSUMMARY(&t);
}

#endif /* INTERVAL_SET_TESTS_H */
