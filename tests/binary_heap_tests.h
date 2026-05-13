#ifndef BINARY_HEAP_TESTS_H
#define BINARY_HEAP_TESTS_H

#include "cvx/flags.h"
#include "tests/alloc.h"
#include "tests/cvxtest.h"
#include "tests/cvxtestutils.h"
#include "tests/implementations.h"

static void bh_fill3(struct bheap_int *h)
{
    bh_int_push(h, 10);
    bh_int_push(h, 20);
    bh_int_push(h, 30);
}

static void test_bh_int_init(struct cvxtest *t)
{
    struct bheap_int h;
    // Null vtabv: flag VTAB, tag stays 0
    bh_int_init(&h, NULL, CVX_MAX_HEAP, 0);
    CVXCHECK(t, h.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, h.super.tag == 0);
    // No comp: flag VTAB
    struct bheap_int_vtabv no_comp = { 0 };
    bh_int_init(&h, &no_comp, CVX_MAX_HEAP, 0);
    CVXCHECK(t, h.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, h.super.tag == 0);
    // Normal
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, h.super.tag == 33);
    CVXCHECK(t, h.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, h.vtabv == bh_int_vtabv_comp_only);
    CVXCHECK(t, h.HO == CVX_MAX_HEAP);
    CVXCHECK(t, h.count == 0);
    CVXCHECK(t, h.capacity == 0);
    CVXCHECK(t, h.buffer == NULL);
    bh_int_drop(&h);
}

static void test_bh_int_clone(struct cvxtest *t)
{
    struct bheap_int orig, clone;
    // Clone empty heap
    bh_int_init(&orig, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_clone(&orig, &clone);
    CVXCHECK(t, clone.super.tag == 33);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, clone.count == 0);
    CVXCHECK(t, clone.vtabv == bh_int_vtabv_comp_only);
    CVXCHECK(t, clone.HO == CVX_MAX_HEAP);
    bh_int_drop(&orig);
    bh_int_drop(&clone);
    // Clone with elements — independent
    bh_int_init(&orig, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(&orig, 3);
    bh_int_push(&orig, 1);
    bh_int_push(&orig, 7);
    bh_int_clone(&orig, &clone);
    CVXCHECK(t, clone.count == 3);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, bh_int_peek(&clone) == 7);
    bh_int_pop(&orig);
    CVXCHECK(t, clone.count == 3);
    bh_int_drop(&orig);
    bh_int_drop(&clone);
    // vtabv->clone called during clone
    CVX_TEST_COUNTER_CLONE_RESET();
    bh_int_init(&orig, bh_int_vtabv_full, CVX_MAX_HEAP, 0);
    bh_int_push(&orig, 10);
    bh_int_push(&orig, 20);
    bh_int_push(&orig, 30);
    bh_int_clone(&orig, &clone);
    CVX_TEST_COUNTER_CLONE(t, 3);
    CVXCHECK(t, clone.count == 3);
    bh_int_drop(&orig);
    bh_int_drop(&clone);
    // Null vtabv->clone: no crash
    bh_int_init(&orig, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(&orig, 7);
    bh_int_push(&orig, 8);
    bh_int_push(&orig, 9);
    bh_int_clone(&orig, &clone);
    CVXCHECK(t, clone.count == 3);
    bh_int_drop(&orig);
    bh_int_drop(&clone);
    // Alloc failure: buffer malloc fails
    bh_int_init(&orig, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(&orig, 10);
    bh_int_push(&orig, 20);
    CVX_MALLOC_FAIL_NEXT();
    bh_int_clone(&orig, &clone);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, orig.super.flag == CVX_FLAG_ALLOC);
    CVX_MALLOC_RESET();
    bh_int_drop(&orig);
    bh_int_drop(&clone);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    struct bheap_int dummy;
    bh_int_init(&dummy, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int__proxy_clone(col, (cvx_container *)&dummy);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
    bh_int_drop(&dummy);
}

static void test_bh_int_drop(struct cvxtest *t)
{
    struct bheap_int h;
    // Drop empty
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_drop(&h);
    // vtabv->drop called for each element
    CVX_TEST_COUNTER_DROP_RESET();
    bh_int_init(&h, bh_int_vtabv_full, CVX_MAX_HEAP, 0);
    bh_int_push(&h, 1);
    bh_int_push(&h, 2);
    bh_int_push(&h, 3);
    bh_int_drop(&h);
    CVX_TEST_COUNTER_DROP(t, 3);
    // Null vtabv->drop: no crash
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(&h, 1);
    bh_int_push(&h, 2);
    bh_int_push(&h, 3);
    bh_int_drop(&h);
    // Null pointer: no crash
    bh_int_drop(NULL);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_flag(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_flag(&h) == CVX_FLAG_OK);
    bh_int_pop(&h);
    CVXCHECK(t, bh_int_flag(&h) == CVX_FLAG_EMPTY);
    bh_int_drop(&h);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    enum cvx_flags result = bh_int__proxy_flag(col);
    CVXCHECK(t, result == CVX_FLAG_WRONG_TAG);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_heap_order(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_heap_order(&h) == CVX_MAX_HEAP);
    bh_int_drop(&h);
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MIN_HEAP, 0);
    CVXCHECK(t, bh_int_heap_order(&h) == CVX_MIN_HEAP);
    bh_int_drop(&h);
}

static void test_bh_int_count(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_count(&h) == 0);
    bh_int_push(&h, 1);
    CVXCHECK(t, bh_int_count(&h) == 1);
    bh_int_push(&h, 2);
    CVXCHECK(t, bh_int_count(&h) == 2);
    bh_int_pop(&h);
    CVXCHECK(t, bh_int_count(&h) == 1);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_count(col) == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_capacity(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_capacity(&h) == 0);
    bh_int_push(&h, 1);
    CVXCHECK(t, bh_int_capacity(&h) >= 1);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_capacity(col) == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_empty(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_empty(&h) == true);
    bh_int_push(&h, 1);
    CVXCHECK(t, bh_int_empty(&h) == false);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_empty(col) == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_full(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 1);
    CVXCHECK(t, bh_int_full(&h) == false);
    bh_int_push(&h, 42);
    CVXCHECK(t, bh_int_full(&h) == true);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_full(col) == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_push(struct cvxtest *t)
{
    struct bheap_int h;
    // Max-heap order
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    int values[] = { 3, 1, 4, 1, 5, 9, 2, 6 };
    int desc[] = { 9, 6, 5, 4, 3, 2, 1, 1 };
    size_t n = sizeof(values) / sizeof(values[0]);
    for (size_t i = 0; i < n; i++)
        bh_int_push(&h, values[i]);
    CVXCHECK(t, bh_int_count(&h) == n);
    CVXCHECK(t, h.super.flag == CVX_FLAG_OK);
    for (size_t i = 0; i < n; i++)
        CVXCHECK(t, bh_int_pop(&h) == desc[i]);
    bh_int_drop(&h);
    // Min-heap order
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MIN_HEAP, 0);
    int asc[] = { 1, 1, 2, 3, 4, 5, 6, 9 };
    for (size_t i = 0; i < n; i++)
        bh_int_push(&h, values[i]);
    for (size_t i = 0; i < n; i++)
        CVXCHECK(t, bh_int_pop(&h) == asc[i]);
    bh_int_drop(&h);
    // Buffer grows
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    for (int i = 0; i < 100; i++)
        bh_int_push(&h, i);
    CVXCHECK(t, bh_int_count(&h) == 100);
    bh_int_drop(&h);
    // Alloc failure: initial buffer
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVX_MALLOC_FAIL_NEXT();
    bh_int_push(&h, 42);
    CVXCHECK(t, h.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, bh_int_count(&h) == 0);
    CVX_MALLOC_RESET();
    bh_int_drop(&h);
    // Alloc failure: realloc (capacity=2, push third)
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 2);
    bh_int_push(&h, 10);
    bh_int_push(&h, 20);
    CVX_MALLOC_FAIL_NEXT();
    bh_int_push(&h, 30);
    CVXCHECK(t, h.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, bh_int_count(&h) == 2);
    CVXCHECK(t, bh_int_capacity(&h) == 2);
    CVX_MALLOC_RESET();
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_push(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
    // Wrong tag on real heap
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    h.super.tag = 0;
    bh_int__proxy_push((cvx_container *)&h, 1);
    CVXCHECK(t, h.super.flag == CVX_FLAG_WRONG_TAG);
    h.super.tag = 33;
    bh_int_drop(&h);
}

static void test_bh_int_pop(struct cvxtest *t)
{
    struct bheap_int h;
    // Empty pop
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_pop(&h) == 0);
    CVXCHECK(t, h.super.flag == CVX_FLAG_EMPTY);
    bh_int_drop(&h);
    // Single element
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(&h, 42);
    CVXCHECK(t, bh_int_pop(&h) == 42);
    CVXCHECK(t, h.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, bh_int_empty(&h) == true);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_pop(col) == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_peek(struct cvxtest *t)
{
    struct bheap_int h;
    // Empty peek
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_peek(&h) == 0);
    CVXCHECK(t, h.super.flag == CVX_FLAG_EMPTY);
    bh_int_drop(&h);
    // Peek does not remove
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(&h, 10);
    bh_int_push(&h, 5);
    bh_int_push(&h, 20);
    CVXCHECK(t, bh_int_peek(&h) == 20);
    CVXCHECK(t, h.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, bh_int_count(&h) == 3);
    bh_int_drop(&h);
    // Min-heap peek
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MIN_HEAP, 0);
    bh_int_push(&h, 10);
    bh_int_push(&h, 5);
    bh_int_push(&h, 20);
    CVXCHECK(t, bh_int_peek(&h) == 5);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_peek(col) == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_init_start(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    struct bheap_int_iter iter = bh_int_iter_init_start(&h);
    CVXCHECK(t, iter.super.tag == BH_ITER_TAG);
    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter.index == 0);
    bh_int_drop(&h);
}

static void test_bh_int_iter_init_end(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter iter = bh_int_iter_init_end(&h);
    CVXCHECK(t, iter.super.tag == BH_ITER_TAG);
    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter.index == 3);
    bh_int_drop(&h);
}

static void test_bh_int_iter_start(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    struct bheap_int_iter *iter = bh_int_iter_start(&h);
    CVXCHECK(t, iter != NULL);
    if (iter)
    {
        CVXCHECK(t, iter->super.tag == BH_ITER_TAG);
        CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
        CVXCHECK(t, bh_int_iter_index(iter) == 0);
        bh_int_iter_drop(iter);
    }
    // Alloc failure
    CVX_MALLOC_FAIL_NEXT();
    iter = bh_int_iter_start(&h);
    CVXCHECK(t, iter == NULL);
    CVX_MALLOC_RESET();
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_iter_start(col) == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_end(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_end(&h);
    CVXCHECK(t, iter != NULL);
    if (iter)
    {
        CVXCHECK(t, iter->super.tag == BH_ITER_TAG);
        CVXCHECK(t, bh_int_iter_index(iter) == 3);
        bh_int_iter_drop(iter);
    }
    // Alloc failure
    CVX_MALLOC_FAIL_NEXT();
    iter = bh_int_iter_end(&h);
    CVXCHECK(t, iter == NULL);
    CVX_MALLOC_RESET();
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_iter_end(col) == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_drop(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    struct bheap_int_iter *iter = bh_int_iter_start(&h);
    CVXCHECK(t, iter != NULL);
    if (iter)
        bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_iter_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_at_start(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_start(&h);
    CVXCHECK(t, bh_int_iter_at_start(iter) == true);
    bh_int_iter_next(iter);
    CVXCHECK(t, bh_int_iter_at_start(iter) == false);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_iter_at_start(col) == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_at_end(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_end(&h);
    CVXCHECK(t, bh_int_iter_at_end(iter) == true);
    bh_int_iter_prev(iter);
    CVXCHECK(t, bh_int_iter_at_end(iter) == false);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_iter_at_end(col) == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_count(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    for (int i = 0; i < 5; i++)
        bh_int_push(&h, i);
    struct bheap_int_iter *iter = bh_int_iter_start(&h);
    CVXCHECK(t, bh_int_iter_count(iter) == 5);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_iter_count(col) == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_to_start(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_start(&h);
    bh_int_iter_next(iter);
    bh_int_iter_next(iter);
    bh_int_iter_to_start(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_iter_to_start(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_to_end(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_start(&h);
    bh_int_iter_to_end(iter);
    CVXCHECK(t, bh_int_iter_at_end(iter) == true);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_iter_to_end(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_next(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_start(&h);
    CVXCHECK(t, bh_int_iter_index(iter) == 0);
    bh_int_iter_next(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 1);
    bh_int_iter_next(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 2);
    // next at end: RANGE
    bh_int_iter_to_end(iter);
    bh_int_iter_next(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_iter_next(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_prev(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_end(&h);
    CVXCHECK(t, bh_int_iter_index(iter) == 3);
    bh_int_iter_prev(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 2);
    bh_int_iter_prev(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 1);
    // prev at start: RANGE
    bh_int_iter_to_start(iter);
    bh_int_iter_prev(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_iter_prev(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_forward(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_start(&h);
    bh_int_iter_forward(iter, 2);
    CVXCHECK(t, bh_int_iter_index(iter) == 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    // Clamps to end
    bh_int_iter_forward(iter, 100);
    CVXCHECK(t, bh_int_iter_index(iter) == 3);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_iter_forward(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_backward(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_end(&h);
    bh_int_iter_backward(iter, 2);
    CVXCHECK(t, bh_int_iter_index(iter) == 1);
    // Clamps to start
    bh_int_iter_backward(iter, 100);
    CVXCHECK(t, bh_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_iter_backward(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_go_to(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_start(&h);
    bh_int_iter_go_to(iter, 2);
    CVXCHECK(t, bh_int_iter_index(iter) == 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    // Go to end position
    bh_int_iter_go_to(iter, 3);
    CVXCHECK(t, bh_int_iter_at_end(iter) == true);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    // Out of range
    bh_int_iter_go_to(iter, 4);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_iter_go_to(col, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_value(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_start(&h);
    // Index 0 is max in a max-heap
    CVXCHECK(t, bh_int_iter_value(iter) == 30);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    // Traverse and sum all values
    int sum = 0;
    while (!bh_int_iter_at_end(iter))
    {
        sum += bh_int_iter_value(iter);
        bh_int_iter_next(iter);
    }
    CVXCHECK(t, sum == 60);
    // At end: RANGE
    int val = bh_int_iter_value(iter);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_iter_value(col) == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_index(struct cvxtest *t)
{
    struct bheap_int h;
    bh_int_init(&h, bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_fill3(&h);
    struct bheap_int_iter *iter = bh_int_iter_start(&h);
    CVXCHECK(t, bh_int_iter_index(iter) == 0);
    bh_int_iter_next(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 1);
    bh_int_iter_next(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 2);
    bh_int_iter_drop(iter);
    bh_int_drop(&h);
    // Guard
    MAKE_INVALID_CONTAINER(col);
    CVXCHECK(t, bh_int__proxy_iter_index(col) == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static struct cvxresult run_binary_heap_tests(void)
{
    struct cvxtest t = { 0 };

    printf("binary_heap\n");

    CVXRUN(&t, test_bh_int_init);
    CVXRUN(&t, test_bh_int_clone);
    CVXRUN(&t, test_bh_int_drop);
    CVXRUN(&t, test_bh_int_flag);
    CVXRUN(&t, test_bh_int_heap_order);
    CVXRUN(&t, test_bh_int_count);
    CVXRUN(&t, test_bh_int_capacity);
    CVXRUN(&t, test_bh_int_empty);
    CVXRUN(&t, test_bh_int_full);
    CVXRUN(&t, test_bh_int_push);
    CVXRUN(&t, test_bh_int_pop);
    CVXRUN(&t, test_bh_int_peek);
    CVXRUN(&t, test_bh_int_iter_init_start);
    CVXRUN(&t, test_bh_int_iter_init_end);
    CVXRUN(&t, test_bh_int_iter_start);
    CVXRUN(&t, test_bh_int_iter_end);
    CVXRUN(&t, test_bh_int_iter_drop);
    CVXRUN(&t, test_bh_int_iter_at_start);
    CVXRUN(&t, test_bh_int_iter_at_end);
    CVXRUN(&t, test_bh_int_iter_count);
    CVXRUN(&t, test_bh_int_iter_to_start);
    CVXRUN(&t, test_bh_int_iter_to_end);
    CVXRUN(&t, test_bh_int_iter_next);
    CVXRUN(&t, test_bh_int_iter_prev);
    CVXRUN(&t, test_bh_int_iter_forward);
    CVXRUN(&t, test_bh_int_iter_backward);
    CVXRUN(&t, test_bh_int_iter_go_to);
    CVXRUN(&t, test_bh_int_iter_value);
    CVXRUN(&t, test_bh_int_iter_index);

    return CVXSUMMARY(&t);
}

#endif /* BINARY_HEAP_TESTS_H */
