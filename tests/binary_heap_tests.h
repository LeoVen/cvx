#ifndef BINARY_HEAP_TESTS_H
#define BINARY_HEAP_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"
#include "implementations.h"

/* ---- init ---- */

static void test_bh_int_init_null_vtabv(struct cvxtest *t)
{
    struct bheap_int h = bh_int_init(NULL);
    CVXCHECK(t, h.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, h.super.tag == 0);
}

static void test_bh_int_init_no_comp(struct cvxtest *t)
{
    struct bheap_int_vtabv vtabv = { 0 };
    struct bheap_int h = bh_int_init(&vtabv);
    CVXCHECK(t, h.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, h.super.tag == 0);
}

static void test_bh_int_init(struct cvxtest *t)
{
    struct bheap_int h = bh_int_init(bh_int_vtabv_comp_only);
    CVXCHECK(t, h.super.tag == 33);
    CVXCHECK(t, h.vtabv == bh_int_vtabv_comp_only);
    CVXCHECK(t, h.HO == CVX_MAX_HEAP);
    CVXCHECK(t, h.count == 0);
    CVXCHECK(t, h.capacity == 0);
    CVXCHECK(t, h.buffer == NULL);
}

/* ---- init_with ---- */

static void test_bh_int_init_with_null_vtabv(struct cvxtest *t)
{
    struct bheap_int h = bh_int_init_with(NULL, CVX_MAX_HEAP, 8);
    CVXCHECK(t, h.super.flag == CVX_FLAG_VTAB);
}

static void test_bh_int_init_with(struct cvxtest *t)
{
    struct bheap_int h = bh_int_init_with(bh_int_vtabv_comp_only, CVX_MIN_HEAP, 8);
    CVXCHECK(t, h.super.tag == 33);
    CVXCHECK(t, h.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, h.HO == CVX_MIN_HEAP);
    CVXCHECK(t, h.capacity == 8);
    CVXCHECK(t, h.buffer != NULL);
    CVXCHECK(t, h.count == 0);
    free(h.buffer);
}

static void test_bh_int_init_with_zero_capacity(struct cvxtest *t)
{
    struct bheap_int h = bh_int_init_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, h.super.tag == 33);
    CVXCHECK(t, h.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, h.capacity == 0);
    CVXCHECK(t, h.buffer == NULL);
}

/* ---- copy ---- */

static void test_bh_int_copy_empty(struct cvxtest *t)
{
    struct bheap_int *src = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);

    struct bheap_int copy = bh_int_copy(src);
    CVXCHECK(t, copy.super.tag == 33);
    CVXCHECK(t, copy.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, copy.count == 0);
    CVXCHECK(t, copy.vtabv == bh_int_vtabv_comp_only);
    CVXCHECK(t, copy.HO == CVX_MAX_HEAP);

    bh_int_drop(src);
}

static void test_bh_int_copy_independent(struct cvxtest *t)
{
    struct bheap_int *src = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(src, 10);
    bh_int_push(src, 20);
    bh_int_push(src, 5);

    struct bheap_int copy = bh_int_copy(src);
    CVXCHECK(t, copy.count == 3);
    CVXCHECK(t, copy.super.flag == CVX_FLAG_OK);

    /* Modifying src does not affect copy */
    bh_int_push(src, 100);
    CVXCHECK(t, copy.count == 3);

    /* Copy preserves heap invariant: peek returns max */
    CVXCHECK(t, bh_int_peek(&copy) == 20);

    free(copy.buffer);
    bh_int_drop(src);
}

/* ---- new ---- */

static void test_bh_int_new(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new();
    CVXCHECK(t, h != NULL);
    if (!h)
        return;

    CVXCHECK(t, h->super.tag == 33);
    CVXCHECK(t, h->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, h->count == 0);
    CVXCHECK(t, h->capacity == 0);
    CVXCHECK(t, h->buffer == NULL);
    CVXCHECK(t, h->vtabv == NULL);
    CVXCHECK(t, h->HO == CVX_MAX_HEAP);

    bh_int_drop(h);
}

/* ---- new_with ---- */

static void test_bh_int_new_with(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MIN_HEAP, 4);
    CVXCHECK(t, h != NULL);
    if (!h)
        return;

    CVXCHECK(t, h->super.tag == 33);
    CVXCHECK(t, h->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, h->vtabv == bh_int_vtabv_comp_only);
    CVXCHECK(t, h->HO == CVX_MIN_HEAP);
    CVXCHECK(t, h->capacity == 4);
    CVXCHECK(t, h->count == 0);
    CVXCHECK(t, h->buffer != NULL);

    bh_int_drop(h);
}

static void test_bh_int_new_with_zero_capacity(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, h != NULL);
    if (!h)
        return;

    CVXCHECK(t, h->capacity == 0);
    CVXCHECK(t, h->buffer == NULL);

    bh_int_drop(h);
}

/* ---- clone ---- */

static void test_bh_int_clone_empty(struct cvxtest *t)
{
    struct bheap_int *src = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);

    struct bheap_int *clone = bh_int_clone(src);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        bh_int_drop(src);
        return;
    }

    CVXCHECK(t, clone->super.tag == 33);
    CVXCHECK(t, clone->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, clone->count == 0);
    CVXCHECK(t, clone->vtabv == bh_int_vtabv_comp_only);
    CVXCHECK(t, clone->HO == CVX_MAX_HEAP);

    bh_int_drop(clone);
    bh_int_drop(src);
}

static void test_bh_int_clone_independent(struct cvxtest *t)
{
    struct bheap_int *src = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(src, 3);
    bh_int_push(src, 1);
    bh_int_push(src, 7);

    struct bheap_int *clone = bh_int_clone(src);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        bh_int_drop(src);
        return;
    }

    CVXCHECK(t, clone->count == 3);
    CVXCHECK(t, bh_int_peek(clone) == 7);

    /* Modifying src does not affect clone */
    bh_int_pop(src);
    CVXCHECK(t, clone->count == 3);

    bh_int_drop(clone);
    bh_int_drop(src);
}

/* ---- clear ---- */

static void test_bh_int_clear(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(h, 5);
    bh_int_push(h, 3);
    bh_int_push(h, 8);

    bh_int_clear(h);

    CVXCHECK(t, h->count == 0);
    CVXCHECK(t, h->capacity == 0);
    CVXCHECK(t, h->buffer == NULL);
    CVXCHECK(t, h->super.flag == CVX_FLAG_OK);

    bh_int_drop(h);
}

static void test_bh_int_clear_reusable(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(h, 10);
    bh_int_clear(h);

    bh_int_push(h, 42);
    CVXCHECK(t, bh_int_count(h) == 1);
    CVXCHECK(t, bh_int_peek(h) == 42);

    bh_int_drop(h);
}

/* ---- flag ---- */

static void test_bh_int_flag(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_flag(h) == CVX_FLAG_OK);

    bh_int_pop(h); /* empty pop sets flag to EMPTY */
    CVXCHECK(t, bh_int_flag(h) == CVX_FLAG_EMPTY);

    bh_int_drop(h);
}

/* ---- heap_order ---- */

static void test_bh_int_heap_order_max(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_heap_order(h) == CVX_MAX_HEAP);
    bh_int_drop(h);
}

static void test_bh_int_heap_order_min(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MIN_HEAP, 0);
    CVXCHECK(t, bh_int_heap_order(h) == CVX_MIN_HEAP);
    bh_int_drop(h);
}

/* ---- count ---- */

static void test_bh_int_count(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_count(h) == 0);

    bh_int_push(h, 1);
    CVXCHECK(t, bh_int_count(h) == 1);

    bh_int_push(h, 2);
    CVXCHECK(t, bh_int_count(h) == 2);

    bh_int_pop(h);
    CVXCHECK(t, bh_int_count(h) == 1);

    bh_int_drop(h);
}

/* ---- capacity ---- */

static void test_bh_int_capacity_grows(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_capacity(h) == 0);

    bh_int_push(h, 1); /* triggers first allocation */
    CVXCHECK(t, bh_int_capacity(h) >= 1);

    bh_int_drop(h);
}

/* ---- empty ---- */

static void test_bh_int_empty_true(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, bh_int_empty(h) == true);
    bh_int_drop(h);
}

static void test_bh_int_empty_false(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(h, 1);
    CVXCHECK(t, bh_int_empty(h) == false);
    bh_int_drop(h);
}

/* ---- full ---- */

static void test_bh_int_full_true(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 1);
    bh_int_push(h, 42);
    CVXCHECK(t, bh_int_full(h) == true);
    bh_int_drop(h);
}

static void test_bh_int_full_false(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 4);
    CVXCHECK(t, bh_int_full(h) == false);
    bh_int_drop(h);
}

/* ---- push ---- */

static void test_bh_int_push_max_heap_order(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    int values[] = { 3, 1, 4, 1, 5, 9, 2, 6 };
    int sorted_desc[] = { 9, 6, 5, 4, 3, 2, 1, 1 };
    size_t n = sizeof(values) / sizeof(values[0]);

    for (size_t i = 0; i < n; i++)
        bh_int_push(h, values[i]);

    CVXCHECK(t, bh_int_count(h) == n);
    CVXCHECK(t, h->super.flag == CVX_FLAG_OK);

    for (size_t i = 0; i < n; i++)
        CVXCHECK(t, bh_int_pop(h) == sorted_desc[i]);

    bh_int_drop(h);
}

static void test_bh_int_push_min_heap_order(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MIN_HEAP, 0);
    int values[] = { 3, 1, 4, 1, 5, 9, 2, 6 };
    int sorted_asc[] = { 1, 1, 2, 3, 4, 5, 6, 9 };
    size_t n = sizeof(values) / sizeof(values[0]);

    for (size_t i = 0; i < n; i++)
        bh_int_push(h, values[i]);

    for (size_t i = 0; i < n; i++)
        CVXCHECK(t, bh_int_pop(h) == sorted_asc[i]);

    bh_int_drop(h);
}

static void test_bh_int_push_grows_buffer(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);

    for (int i = 0; i < 100; i++)
        bh_int_push(h, i);

    CVXCHECK(t, bh_int_count(h) == 100);
    CVXCHECK(t, h->super.flag == CVX_FLAG_OK);

    bh_int_drop(h);
}

/* ---- pop ---- */

static void test_bh_int_pop_empty(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);

    int result = bh_int_pop(h);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, h->super.flag == CVX_FLAG_EMPTY);

    bh_int_drop(h);
}

static void test_bh_int_pop_single(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(h, 42);

    int result = bh_int_pop(h);
    CVXCHECK(t, result == 42);
    CVXCHECK(t, h->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, bh_int_empty(h) == true);

    bh_int_drop(h);
}

/* ---- peek ---- */

static void test_bh_int_peek_empty(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);

    int result = bh_int_peek(h);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, h->super.flag == CVX_FLAG_EMPTY);

    bh_int_drop(h);
}

static void test_bh_int_peek_does_not_remove(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(h, 10);
    bh_int_push(h, 5);
    bh_int_push(h, 20);

    int top = bh_int_peek(h);
    CVXCHECK(t, top == 20);
    CVXCHECK(t, h->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, bh_int_count(h) == 3); /* count unchanged */

    bh_int_drop(h);
}

static void test_bh_int_peek_min_heap(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MIN_HEAP, 0);
    bh_int_push(h, 10);
    bh_int_push(h, 5);
    bh_int_push(h, 20);

    CVXCHECK(t, bh_int_peek(h) == 5);

    bh_int_drop(h);
}

/* ---- runner ---- */

static struct cvxresult run_binary_heap_tests(void)
{
    struct cvxtest t = { 0 };

    printf("binary_heap\n");

    CVXRUN(&t, test_bh_int_init_null_vtabv);
    CVXRUN(&t, test_bh_int_init_no_comp);
    CVXRUN(&t, test_bh_int_init);

    CVXRUN(&t, test_bh_int_init_with_null_vtabv);
    CVXRUN(&t, test_bh_int_init_with);
    CVXRUN(&t, test_bh_int_init_with_zero_capacity);

    CVXRUN(&t, test_bh_int_copy_empty);
    CVXRUN(&t, test_bh_int_copy_independent);

    CVXRUN(&t, test_bh_int_new);

    CVXRUN(&t, test_bh_int_new_with);
    CVXRUN(&t, test_bh_int_new_with_zero_capacity);

    CVXRUN(&t, test_bh_int_clone_empty);
    CVXRUN(&t, test_bh_int_clone_independent);

    CVXRUN(&t, test_bh_int_clear);
    CVXRUN(&t, test_bh_int_clear_reusable);

    CVXRUN(&t, test_bh_int_flag);

    CVXRUN(&t, test_bh_int_heap_order_max);
    CVXRUN(&t, test_bh_int_heap_order_min);

    CVXRUN(&t, test_bh_int_count);

    CVXRUN(&t, test_bh_int_capacity_grows);

    CVXRUN(&t, test_bh_int_empty_true);
    CVXRUN(&t, test_bh_int_empty_false);

    CVXRUN(&t, test_bh_int_full_true);
    CVXRUN(&t, test_bh_int_full_false);

    CVXRUN(&t, test_bh_int_push_max_heap_order);
    CVXRUN(&t, test_bh_int_push_min_heap_order);
    CVXRUN(&t, test_bh_int_push_grows_buffer);

    CVXRUN(&t, test_bh_int_pop_empty);
    CVXRUN(&t, test_bh_int_pop_single);

    CVXRUN(&t, test_bh_int_peek_empty);
    CVXRUN(&t, test_bh_int_peek_does_not_remove);
    CVXRUN(&t, test_bh_int_peek_min_heap);

    return CVXSUMMARY(&t);
}

#endif /* BINARY_HEAP_TESTS_H */
