#ifndef INTERVAL_MAP_TESTS_H
#define INTERVAL_MAP_TESTS_H

#include "cvx/flags.h"
#include "tests/alloc.h"
#include "tests/cvxtest.h"
#include "tests/cvxtestutils.h"
#include "tests/implementations.h"

/* Helper: stack-allocate [1,5)→10  [10,15)→20  [20,25)→30 */
static void im_int_fill3(struct imap_int_int *col)
{
    im_int_add(col, 1, 5, 10);
    im_int_add(col, 10, 15, 20);
    im_int_add(col, 20, 25, 30);
}

/* ---- init ---- */

static void test_im_int_init(struct cvxtest *t)
{
    struct imap_int_int s;
    im_int_init(&s, im_int_vtabk, NULL);
    CVXCHECK(t, s.super.tag == 44);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, s.count == 0);
    CVXCHECK(t, s.capacity == 0);
    CVXCHECK(t, s.buffer == NULL);
    CVXCHECK(t, s.vtabk == im_int_vtabk);
    CVXCHECK(t, s.vtabv == NULL);

    struct imap_int_int s_null;
    im_int_init(&s_null, NULL, NULL);
    CVXCHECK(t, s_null.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, s_null.super.tag == 0);

    struct imap_int_int_vtabk vtabk = { 0 };
    struct imap_int_int s_nocomp;
    im_int_init(&s_nocomp, &vtabk, NULL);
    CVXCHECK(t, s_nocomp.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, s_nocomp.super.tag == 0);
}

/* ---- clone ---- */

static void test_im_int_clone(struct cvxtest *t)
{
    struct imap_int_int orig;
    im_int_init(&orig, im_int_vtabk, NULL);

    struct imap_int_int clone_empty;
    im_int_clone(&orig, &clone_empty);
    CVXCHECK(t, clone_empty.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(&clone_empty) == 0);
    im_int_drop(&clone_empty);

    im_int_add(&orig, 1, 5, 10);
    im_int_add(&orig, 10, 15, 20);

    struct imap_int_int clone_vals;
    im_int_clone(&orig, &clone_vals);
    CVXCHECK(t, clone_vals.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(&clone_vals) == 2);
    CVXCHECK(t, im_int_get(&clone_vals, 3) == 10);
    CVXCHECK(t, im_int_get(&clone_vals, 12) == 20);
    CVXCHECK(t, im_int_contains_key(&clone_vals, 7) == false);

    im_int_add(&orig, 6, 9, 99);
    CVXCHECK(t, im_int_count(&clone_vals) == 2);

    CVX_TEST_COUNTER_CLONE_RESET();
    struct imap_int_int clone_vtab;
    struct imap_int_int orig_full;
    im_int_init(&orig_full, im_int_vtabk_full, im_int_vtabv_full);
    im_int_add(&orig_full, 1, 5, 10);
    im_int_add(&orig_full, 10, 15, 20);
    CVX_TEST_COUNTER_CLONE_RESET();
    im_int_clone(&orig_full, &clone_vtab);
    CVX_TEST_COUNTER_CLONE(t, 6);
    CVXCHECK(t, im_int_count(&clone_vtab) == 2);
    im_int_drop(&orig_full);
    im_int_drop(&clone_vtab);

    struct imap_int_int orig_alloc;
    im_int_init(&orig_alloc, im_int_vtabk, NULL);
    im_int_add(&orig_alloc, 1, 5, 10);
    CVX_MALLOC_FAIL_NEXT();
    struct imap_int_int clone_alloc;
    im_int_clone(&orig_alloc, &clone_alloc);
    CVXCHECK(t, clone_alloc.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, orig_alloc.super.flag == CVX_FLAG_ALLOC);
    CVX_MALLOC_RESET();
    im_int_drop(&orig_alloc);

    MAKE_INVALID_CONTAINER(col);
    struct imap_int_int dummy = { 0 };
    im_int__proxy_clone(col, (cvx_container *)&dummy);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);

    im_int_drop(&orig);
    im_int_drop(&clone_vals);
}

/* ---- drop ---- */

static void test_im_int_drop(struct cvxtest *t)
{
    struct imap_int_int s;
    im_int_init(&s, im_int_vtabk, NULL);
    im_int_drop(&s);
    CVXCHECK(t, s.buffer == NULL);
    CVXCHECK(t, s.count == 0);

    CVX_TEST_COUNTER_DROP_RESET();
    struct imap_int_int s_vtab;
    im_int_init(&s_vtab, im_int_vtabk_full, im_int_vtabv_full);
    im_int_add(&s_vtab, 1, 5, 10);
    im_int_add(&s_vtab, 10, 15, 20);
    CVX_TEST_COUNTER_DROP_RESET();
    im_int_drop(&s_vtab);
    CVX_TEST_COUNTER_DROP(t, 6);

    im_int_drop(NULL);

    MAKE_INVALID_CONTAINER(col);
    im_int__proxy_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- count ---- */

static void test_im_int_count(struct cvxtest *t)
{
    struct imap_int_int s;
    im_int_init(&s, im_int_vtabk, NULL);
    CVXCHECK(t, im_int_count(&s) == 0);
    im_int_add(&s, 1, 5, 10);
    CVXCHECK(t, im_int_count(&s) == 1);
    im_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    size_t res = im_int__proxy_count(col);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- empty ---- */

static void test_im_int_empty(struct cvxtest *t)
{
    struct imap_int_int s;
    im_int_init(&s, im_int_vtabk, NULL);
    CVXCHECK(t, im_int_empty(&s) == true);
    im_int_add(&s, 1, 3, 99);
    CVXCHECK(t, im_int_empty(&s) == false);
    im_int_drop(&s);
    im_int_init(&s, im_int_vtabk, NULL);
    CVXCHECK(t, im_int_empty(&s) == true);
    im_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    bool res = im_int__proxy_empty(col);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- add ---- */

static void test_im_int_add(struct cvxtest *t)
{
    struct imap_int_int s;
    im_int_init(&s, im_int_vtabk, NULL);

    im_int_add(&s, 3, 7, 42);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(&s) == 1);
    CVXCHECK(t, im_int_get(&s, 3) == 42);
    CVXCHECK(t, im_int_get(&s, 6) == 42);
    CVXCHECK(t, im_int_contains_key(&s, 7) == false);

    im_int_add(&s, 10, 15, 20);
    CVXCHECK(t, im_int_count(&s) == 2);

    im_int_add(&s, 3, 7, 99);
    CVXCHECK(t, im_int_count(&s) == 2);
    CVXCHECK(t, im_int_get(&s, 5) == 99);

    im_int_drop(&s);
    im_int_init(&s, im_int_vtabk, NULL);

    im_int_add(&s, 1, 10, 10);
    im_int_add(&s, 5, 15, 20);
    CVXCHECK(t, im_int_count(&s) == 2);
    CVXCHECK(t, im_int_get(&s, 2) == 10);
    CVXCHECK(t, im_int_get(&s, 5) == 20);

    im_int_drop(&s);
    im_int_init(&s, im_int_vtabk, NULL);

    im_int_add(&s, 1, 20, 10);
    im_int_add(&s, 5, 15, 20);
    CVXCHECK(t, im_int_count(&s) == 3);
    CVXCHECK(t, im_int_get(&s, 2) == 10);
    CVXCHECK(t, im_int_get(&s, 7) == 20);
    CVXCHECK(t, im_int_get(&s, 17) == 10);

    im_int_drop(&s);
    im_int_init(&s, im_int_vtabk, im_int_vtabv_with_comp);

    im_int_add(&s, 1, 5, 10);
    im_int_add(&s, 5, 10, 10);
    CVXCHECK(t, im_int_count(&s) == 1);
    CVXCHECK(t, im_int_get(&s, 1) == 10);
    CVXCHECK(t, im_int_get(&s, 9) == 10);

    im_int_add(&s, 10, 15, 20);
    CVXCHECK(t, im_int_count(&s) == 2);
    CVXCHECK(t, im_int_get(&s, 12) == 20);

    im_int_drop(&s);
    im_int_init(&s, im_int_vtabk, NULL);

    im_int_add(&s, 5, 5, 1);
    CVXCHECK(t, s.super.flag == CVX_FLAG_INVALID);
    im_int_add(&s, 9, 3, 1);
    CVXCHECK(t, s.super.flag == CVX_FLAG_INVALID);

    im_int_drop(&s);

    struct imap_int_int s_novtab;
    im_int_init(&s_novtab, NULL, NULL);
    im_int_add(&s_novtab, 1, 5, 10);
    CVXCHECK(t, s_novtab.super.flag == CVX_FLAG_VTAB);

    MAKE_INVALID_CONTAINER(col);
    im_int__proxy_add(col, 1, 5, 10);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);

    CVX_MALLOC_FAIL_NEXT();
    struct imap_int_int s_alloc;
    im_int_init(&s_alloc, im_int_vtabk, NULL);
    im_int_add(&s_alloc, 1, 5, 10);
    CVXCHECK(t, s_alloc.super.flag == CVX_FLAG_ALLOC);
    CVX_MALLOC_RESET();
    im_int_drop(&s_alloc);

    struct imap_int_int s_realloc;
    im_int_init(&s_realloc, im_int_vtabk, NULL);
    for (int i = 0; i < 15; i++)
        im_int_add(&s_realloc, i * 10, i * 10 + 5, i);
    CVXCHECK(t, im_int_count(&s_realloc) == 15);
    CVX_MALLOC_FAIL_NEXT();
    im_int_add(&s_realloc, 200, 205, 99);
    CVXCHECK(t, s_realloc.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, im_int_count(&s_realloc) == 15);
    CVX_MALLOC_RESET();
    im_int_drop(&s_realloc);

    CVX_TEST_COUNTER_DROP_RESET();
    struct imap_int_int s_drop;
    im_int_init(&s_drop, im_int_vtabk_full, im_int_vtabv_full);
    im_int_add(&s_drop, 1, 10, 10);
    CVX_TEST_COUNTER_DROP_RESET();
    im_int_add(&s_drop, 1, 10, 20);
    CVX_TEST_COUNTER_DROP(t, 3);
    CVXCHECK(t, im_int_get(&s_drop, 5) == 20);
    im_int_drop(&s_drop);
}

/* ---- remove ---- */

static void test_im_int_remove(struct cvxtest *t)
{
    struct imap_int_int s;
    im_int_init(&s, im_int_vtabk, NULL);

    im_int_add(&s, 3, 7, 10);
    im_int_remove(&s, 3, 7);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(&s) == 0);

    im_int_add(&s, 1, 10, 42);
    im_int_remove(&s, 3, 7);
    CVXCHECK(t, im_int_count(&s) == 2);
    CVXCHECK(t, im_int_get(&s, 1) == 42);
    CVXCHECK(t, im_int_contains_key(&s, 3) == false);
    CVXCHECK(t, im_int_get(&s, 7) == 42);

    im_int_drop(&s);
    im_int_init(&s, im_int_vtabk, NULL);

    im_int_add(&s, 1, 10, 42);
    im_int_remove(&s, 1, 5);
    CVXCHECK(t, im_int_count(&s) == 1);
    CVXCHECK(t, im_int_contains_key(&s, 1) == false);
    CVXCHECK(t, im_int_get(&s, 5) == 42);

    im_int_drop(&s);
    im_int_init(&s, im_int_vtabk, NULL);

    im_int_add(&s, 1, 5, 10);
    im_int_add(&s, 8, 12, 20);
    im_int_add(&s, 15, 20, 30);
    im_int_remove(&s, 3, 17);
    CVXCHECK(t, im_int_count(&s) == 2);
    CVXCHECK(t, im_int_get(&s, 1) == 10);
    CVXCHECK(t, im_int_contains_key(&s, 3) == false);
    CVXCHECK(t, im_int_get(&s, 17) == 30);

    im_int_add(&s, 1, 5, 1);
    im_int_remove(&s, 3, 3);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);

    im_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    im_int__proxy_remove(col, 1, 5);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);

    CVX_TEST_COUNTER_DROP_RESET();
    CVX_TEST_COUNTER_CLONE_RESET();
    struct imap_int_int s_vtab;
    im_int_init(&s_vtab, im_int_vtabk_full, im_int_vtabv_full);
    im_int_add(&s_vtab, 1, 10, 42);
    CVX_TEST_COUNTER_DROP_RESET();
    CVX_TEST_COUNTER_CLONE_RESET();
    im_int_remove(&s_vtab, 3, 7);
    CVX_TEST_COUNTER_CLONE(t, 6);
    CVX_TEST_COUNTER_DROP(t, 3);
    CVXCHECK(t, im_int_count(&s_vtab) == 2);
    im_int_drop(&s_vtab);
}

/* ---- get ---- */

static void test_im_int_get(struct cvxtest *t)
{
    struct imap_int_int s;
    im_int_init(&s, im_int_vtabk, NULL);

    im_int_get(&s, 5);
    CVXCHECK(t, s.super.flag == CVX_FLAG_NOT_FOUND);

    im_int_add(&s, 5, 10, 42);
    int val = im_int_get(&s, 7);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, val == 42);

    val = im_int_get(&s, 5);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, val == 42);

    im_int_get(&s, 10);
    CVXCHECK(t, s.super.flag == CVX_FLAG_NOT_FOUND);

    im_int_add(&s, 15, 20, 99);
    im_int_get(&s, 11);
    CVXCHECK(t, s.super.flag == CVX_FLAG_NOT_FOUND);

    im_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    int res = im_int__proxy_get(col, 3);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- contains_key ---- */

static void test_im_int_contains_key(struct cvxtest *t)
{
    struct imap_int_int s;
    im_int_init(&s, im_int_vtabk, NULL);
    im_int_add(&s, 5, 10, 42);

    CVXCHECK(t, im_int_contains_key(&s, 7) == true);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_contains_key(&s, 10) == false);
    CVXCHECK(t, im_int_contains_key(&s, 3) == false);

    im_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    bool res = im_int__proxy_contains_key(col, 3);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- contains_interval ---- */

static void test_im_int_contains_interval(struct cvxtest *t)
{
    struct imap_int_int s;
    im_int_init(&s, im_int_vtabk, NULL);
    im_int_add(&s, 1, 10, 42);

    CVXCHECK(t, im_int_contains_interval(&s, 3, 7) == true);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_contains_interval(&s, 5, 5) == false);

    im_int_add(&s, 15, 20, 99);
    CVXCHECK(t, im_int_contains_interval(&s, 3, 16) == false);

    im_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    bool res = im_int__proxy_contains_interval(col, 1, 5);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- overlaps ---- */

static void test_im_int_overlaps(struct cvxtest *t)
{
    struct imap_int_int s;
    im_int_init(&s, im_int_vtabk, NULL);

    CVXCHECK(t, im_int_overlaps(&s, 1, 5) == false);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);

    im_int_add(&s, 1, 5, 42);
    CVXCHECK(t, im_int_overlaps(&s, 3, 8) == true);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_overlaps(&s, 6, 10) == false);
    CVXCHECK(t, im_int_overlaps(&s, 5, 8) == false);

    im_int_drop(&s);

    MAKE_INVALID_CONTAINER(col);
    bool res = im_int__proxy_overlaps(col, 1, 5);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_init_start ---- */

static void test_im_int_iter_init_start(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);

    struct imap_int_int_iter it = im_int_iter_init_start(&col);
    CVXCHECK(t, it.super.tag == (size_t)IM_ITER_TAG);
    CVXCHECK(t, it.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 0);
    CVXCHECK(t, it.target == &col);

    im_int_drop(&col);
    im_int_init(&col, im_int_vtabk, NULL);
    struct imap_int_int_iter it_empty = im_int_iter_init_start(&col);
    CVXCHECK(t, it_empty.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it_empty.index == 0);
    im_int_drop(&col);
}

/* ---- iter_init_end ---- */

static void test_im_int_iter_init_end(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);

    struct imap_int_int_iter it = im_int_iter_init_end(&col);
    CVXCHECK(t, it.super.tag == (size_t)IM_ITER_TAG);
    CVXCHECK(t, it.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 3);

    im_int_drop(&col);
}

/* ---- iter_start / iter_end ---- */

static void test_im_int_iter_start(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);

    struct imap_int_int_iter *iter = im_int_iter_start(&col);
    CVXCHECK(t, iter != NULL);
    if (iter)
    {
        CVXCHECK(t, iter->super.tag == (size_t)IM_ITER_TAG);
        CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
        CVXCHECK(t, iter->index == 0);
        im_int_iter_drop(iter);
    }

    MAKE_INVALID_CONTAINER(bad);
    cvx_container *res = im_int__proxy_iter_start(bad);
    CVXCHECK(t, res == NULL);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);

    CVX_MALLOC_FAIL_NEXT();
    struct imap_int_int_iter *iter_fail = im_int_iter_start(&col);
    CVXCHECK(t, iter_fail == NULL);
    CVX_MALLOC_RESET();

    im_int_drop(&col);
}

/* ---- iter_end ---- */

static void test_im_int_iter_end(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);

    struct imap_int_int_iter *iter = im_int_iter_end(&col);
    CVXCHECK(t, iter != NULL);
    if (iter)
    {
        CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
        CVXCHECK(t, iter->index == 3);
        im_int_iter_drop(iter);
    }

    MAKE_INVALID_CONTAINER(bad);
    cvx_container *res = im_int__proxy_iter_end(bad);
    CVXCHECK(t, res == NULL);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);

    CVX_MALLOC_FAIL_NEXT();
    struct imap_int_int_iter *iter_fail = im_int_iter_end(&col);
    CVXCHECK(t, iter_fail == NULL);
    CVX_MALLOC_RESET();

    im_int_drop(&col);
}

/* ---- iter_drop ---- */

static void test_im_int_iter_drop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    im_int__proxy_iter_drop(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_at_start ---- */

static void test_im_int_iter_at_start(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_start(&col);

    CVXCHECK(t, im_int_iter_at_start(iter) == true);
    im_int_iter_next(iter);
    CVXCHECK(t, im_int_iter_at_start(iter) == false);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    bool res = im_int__proxy_iter_at_start(bad);
    CVXCHECK(t, res == false);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_at_end ---- */

static void test_im_int_iter_at_end(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_end(&col);

    CVXCHECK(t, im_int_iter_at_end(iter) == true);
    im_int_iter_prev(iter);
    CVXCHECK(t, im_int_iter_at_end(iter) == false);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    bool res = im_int__proxy_iter_at_end(bad);
    CVXCHECK(t, res == false);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_count ---- */

static void test_im_int_iter_count(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_start(&col);

    CVXCHECK(t, im_int_iter_count(iter) == 3);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    size_t res = im_int__proxy_iter_count(bad);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_to_start ---- */

static void test_im_int_iter_to_start(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_start(&col);

    im_int_iter_next(iter);
    im_int_iter_next(iter);
    im_int_iter_to_start(iter);
    CVXCHECK(t, im_int_iter_at_start(iter) == true);
    CVXCHECK(t, iter->index == 0);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    im_int__proxy_iter_to_start(bad);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_to_end ---- */

static void test_im_int_iter_to_end(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_start(&col);

    im_int_iter_to_end(iter);
    CVXCHECK(t, im_int_iter_at_end(iter) == true);
    CVXCHECK(t, iter->index == 3);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    im_int__proxy_iter_to_end(bad);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_next ---- */

static void test_im_int_iter_next(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_start(&col);

    CVXCHECK(t, im_int_iter_value_lo(iter) == 1);
    CVXCHECK(t, im_int_iter_value_hi(iter) == 5);
    CVXCHECK(t, im_int_iter_value_val(iter) == 10);
    im_int_iter_next(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_iter_value_lo(iter) == 10);
    CVXCHECK(t, im_int_iter_value_val(iter) == 20);
    im_int_iter_next(iter);
    CVXCHECK(t, im_int_iter_value_lo(iter) == 20);
    CVXCHECK(t, im_int_iter_value_val(iter) == 30);
    im_int_iter_next(iter);
    CVXCHECK(t, im_int_iter_at_end(iter) == true);
    im_int_iter_next(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    im_int__proxy_iter_next(bad);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_prev ---- */

static void test_im_int_iter_prev(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_end(&col);

    im_int_iter_prev(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_iter_value_lo(iter) == 20);
    CVXCHECK(t, im_int_iter_value_val(iter) == 30);
    im_int_iter_prev(iter);
    CVXCHECK(t, im_int_iter_value_lo(iter) == 10);
    im_int_iter_prev(iter);
    CVXCHECK(t, im_int_iter_at_start(iter) == true);
    im_int_iter_prev(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    im_int__proxy_iter_prev(bad);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_forward ---- */

static void test_im_int_iter_forward(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_start(&col);

    im_int_iter_forward(iter, 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_iter_value_lo(iter) == 20);

    im_int_iter_to_start(iter);
    im_int_iter_forward(iter, 100);
    CVXCHECK(t, im_int_iter_at_end(iter) == true);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    im_int__proxy_iter_forward(bad, 1);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_backward ---- */

static void test_im_int_iter_backward(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_end(&col);

    im_int_iter_backward(iter, 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_iter_value_lo(iter) == 10);

    im_int_iter_to_end(iter);
    im_int_iter_backward(iter, 100);
    CVXCHECK(t, im_int_iter_at_start(iter) == true);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    im_int__proxy_iter_backward(bad, 1);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_entry ---- */

static void test_im_int_iter_entry(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_start(&col);

    struct imap_int_int_entry e = im_int_iter_entry(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, e.lo == 1 && e.hi == 5 && e.val == 10);

    im_int_iter_to_end(iter);
    im_int_iter_entry(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    CVXCHECK(t, im_int_iter_value_lo(iter) == 0);

    im_int_iter_to_start(iter);
    CVXCHECK(t, im_int_iter_value_lo(iter) == 1);
    CVXCHECK(t, im_int_iter_value_hi(iter) == 5);
    CVXCHECK(t, im_int_iter_value_val(iter) == 10);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    struct imap_int_int_entry res = im_int__proxy_iter_entry(bad);
    CVXCHECK(t, res.lo == 0 && res.hi == 0 && res.val == 0);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_value_lo / iter_value_hi / iter_value_val ---- */

static void test_im_int_iter_value_lo_hi_val(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_start(&col);

    CVXCHECK(t, im_int_iter_value_lo(iter) == 1);
    CVXCHECK(t, im_int_iter_value_hi(iter) == 5);
    CVXCHECK(t, im_int_iter_value_val(iter) == 10);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad_lo);
    int rlo = im_int__proxy_iter_value_lo(bad_lo);
    CVXCHECK(t, rlo == 0);
    CVXCHECK(t, bad_lo->flag == CVX_FLAG_WRONG_TAG);

    MAKE_INVALID_CONTAINER(bad_hi);
    int rhi = im_int__proxy_iter_value_hi(bad_hi);
    CVXCHECK(t, rhi == 0);
    CVXCHECK(t, bad_hi->flag == CVX_FLAG_WRONG_TAG);

    MAKE_INVALID_CONTAINER(bad_val);
    int rval = im_int__proxy_iter_value_val(bad_val);
    CVXCHECK(t, rval == 0);
    CVXCHECK(t, bad_val->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_index ---- */

static void test_im_int_iter_index(struct cvxtest *t)
{
    struct imap_int_int col;
    im_int_init(&col, im_int_vtabk, NULL);
    im_int_fill3(&col);
    struct imap_int_int_iter *iter = im_int_iter_start(&col);

    CVXCHECK(t, im_int_iter_index(iter) == 0);
    im_int_iter_next(iter);
    CVXCHECK(t, im_int_iter_index(iter) == 1);
    im_int_iter_next(iter);
    CVXCHECK(t, im_int_iter_index(iter) == 2);

    im_int_iter_drop(iter);
    im_int_drop(&col);

    MAKE_INVALID_CONTAINER(bad);
    size_t res = im_int__proxy_iter_index(bad);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, bad->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static struct cvxresult run_interval_map_tests(void)
{
    struct cvxtest t = { 0 };

    printf("interval_map\n");

    CVXRUN(&t, test_im_int_init);
    CVXRUN(&t, test_im_int_clone);
    CVXRUN(&t, test_im_int_drop);
    CVXRUN(&t, test_im_int_count);
    CVXRUN(&t, test_im_int_empty);
    CVXRUN(&t, test_im_int_add);
    CVXRUN(&t, test_im_int_remove);
    CVXRUN(&t, test_im_int_get);
    CVXRUN(&t, test_im_int_contains_key);
    CVXRUN(&t, test_im_int_contains_interval);
    CVXRUN(&t, test_im_int_overlaps);
    CVXRUN(&t, test_im_int_iter_init_start);
    CVXRUN(&t, test_im_int_iter_init_end);
    CVXRUN(&t, test_im_int_iter_start);
    CVXRUN(&t, test_im_int_iter_end);
    CVXRUN(&t, test_im_int_iter_drop);
    CVXRUN(&t, test_im_int_iter_at_start);
    CVXRUN(&t, test_im_int_iter_at_end);
    CVXRUN(&t, test_im_int_iter_count);
    CVXRUN(&t, test_im_int_iter_to_start);
    CVXRUN(&t, test_im_int_iter_to_end);
    CVXRUN(&t, test_im_int_iter_next);
    CVXRUN(&t, test_im_int_iter_prev);
    CVXRUN(&t, test_im_int_iter_forward);
    CVXRUN(&t, test_im_int_iter_backward);
    CVXRUN(&t, test_im_int_iter_entry);
    CVXRUN(&t, test_im_int_iter_value_lo_hi_val);
    CVXRUN(&t, test_im_int_iter_index);

    return CVXSUMMARY(&t);
}

#endif /* INTERVAL_MAP_TESTS_H */
