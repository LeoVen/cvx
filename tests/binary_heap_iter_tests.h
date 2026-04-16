#ifndef BINARY_HEAP_ITER_TESTS_H
#define BINARY_HEAP_ITER_TESTS_H

#include "cvxtest.h"
#include "implementations.h"

static void bh_int_iter_fill3(struct bheap_int *h)
{
    bh_int_push(h, 10);
    bh_int_push(h, 20);
    bh_int_push(h, 30);
}

/* ---- bh_int_iter_init_start / bh_int_iter_init_end ---- */

static void test_bh_int_iter_init_start(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    struct bheap_int_iter iter = bh_int_iter_init_start(h);
    CVXCHECK(t, iter.super.tag == BH_ITER_TAG);
    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter.index == 0);
    bh_int_drop(h);
}

static void test_bh_int_iter_init_end(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter iter = bh_int_iter_init_end(h);
    CVXCHECK(t, iter.super.tag == BH_ITER_TAG);
    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter.index == 3);
    bh_int_drop(h);
}

/* ---- bh_int_iter_start / bh_int_iter_end ---- */

static void test_bh_int_iter_start(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        bh_int_drop(h);
        return;
    }
    CVXCHECK(t, iter->super.tag == BH_ITER_TAG);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, bh_int_iter_index(iter) == 0);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_end(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_end(h);
    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        bh_int_drop(h);
        return;
    }
    CVXCHECK(t, iter->super.tag == BH_ITER_TAG);
    CVXCHECK(t, bh_int_iter_index(iter) == 3);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- bh_int_iter_drop ---- */

static void test_bh_int_iter_drop(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    CVXCHECK(t, iter != NULL);
    if (iter)
        bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- bh_int_iter_at_start / bh_int_iter_at_end ---- */

static void test_bh_int_iter_at_start_true(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    CVXCHECK(t, bh_int_iter_at_start(iter) == true);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_at_start_false(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    bh_int_iter_next(iter);
    CVXCHECK(t, bh_int_iter_at_start(iter) == false);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_at_end_true(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_end(h);
    CVXCHECK(t, bh_int_iter_at_end(iter) == true);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_at_end_false(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    CVXCHECK(t, bh_int_iter_at_end(iter) == false);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- bh_int_iter_count ---- */

static void test_bh_int_iter_count(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    for (int i = 0; i < 5; i++)
        bh_int_push(h, i);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    CVXCHECK(t, bh_int_iter_count(iter) == 5);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- bh_int_iter_to_start / bh_int_iter_to_end ---- */

static void test_bh_int_iter_to_start(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    bh_int_iter_next(iter);
    bh_int_iter_next(iter);
    bh_int_iter_to_start(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_to_end(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    bh_int_iter_to_end(iter);
    CVXCHECK(t, bh_int_iter_at_end(iter) == true);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- bh_int_iter_next ---- */

static void test_bh_int_iter_next(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    CVXCHECK(t, bh_int_iter_index(iter) == 0);
    bh_int_iter_next(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 1);
    bh_int_iter_next(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 2);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_next_at_end(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_end(h);
    bh_int_iter_next(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- bh_int_iter_prev ---- */

static void test_bh_int_iter_prev(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_end(h);
    CVXCHECK(t, bh_int_iter_index(iter) == 3);
    bh_int_iter_prev(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 2);
    bh_int_iter_prev(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 1);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_prev_at_start(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    bh_int_iter_prev(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- bh_int_iter_forward ---- */

static void test_bh_int_iter_forward(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    bh_int_iter_forward(iter, 2);
    CVXCHECK(t, bh_int_iter_index(iter) == 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_forward_clamp(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    bh_int_iter_forward(iter, 100);
    CVXCHECK(t, bh_int_iter_index(iter) == 3);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- bh_int_iter_backward ---- */

static void test_bh_int_iter_backward(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_end(h);
    bh_int_iter_backward(iter, 2);
    CVXCHECK(t, bh_int_iter_index(iter) == 1);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_backward_clamp(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_end(h);
    bh_int_iter_backward(iter, 100);
    CVXCHECK(t, bh_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- bh_int_iter_go_to ---- */

static void test_bh_int_iter_go_to(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    bh_int_iter_go_to(iter, 2);
    CVXCHECK(t, bh_int_iter_index(iter) == 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_go_to_end(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    bh_int_iter_go_to(iter, 3);
    CVXCHECK(t, bh_int_iter_at_end(iter) == true);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_go_to_out_of_range(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    bh_int_iter_go_to(iter, 4);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- bh_int_iter_value ---- */

static void test_bh_int_iter_value(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);

    /* Heap internal buffer is not in sorted order; just verify we can read
     * all 3 elements and collect the expected set {10, 20, 30}. */
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    int sum = 0;
    while (!bh_int_iter_at_end(iter))
    {
        sum += bh_int_iter_value(iter);
        bh_int_iter_next(iter);
    }
    CVXCHECK(t, sum == 60);

    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_value_root_is_max(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);

    /* index 0 is always the max in a max-heap */
    CVXCHECK(t, bh_int_iter_value(iter) == 30);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);

    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

static void test_bh_int_iter_value_at_end(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_end(h);
    int val = bh_int_iter_value(iter);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- bh_int_iter_index ---- */

static void test_bh_int_iter_index(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_iter_fill3(h);
    struct bheap_int_iter *iter = bh_int_iter_start(h);
    CVXCHECK(t, bh_int_iter_index(iter) == 0);
    bh_int_iter_next(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 1);
    bh_int_iter_next(iter);
    CVXCHECK(t, bh_int_iter_index(iter) == 2);
    bh_int_iter_drop(iter);
    bh_int_drop(h);
}

/* ---- runner ---- */

static struct cvxresult run_binary_heap_iter_tests(void)
{
    struct cvxtest t = { 0 };
    printf("binary_heap (iterator)\n");

    CVXRUN(&t, test_bh_int_iter_init_start);
    CVXRUN(&t, test_bh_int_iter_init_end);

    CVXRUN(&t, test_bh_int_iter_start);
    CVXRUN(&t, test_bh_int_iter_end);

    CVXRUN(&t, test_bh_int_iter_drop);

    CVXRUN(&t, test_bh_int_iter_at_start_true);
    CVXRUN(&t, test_bh_int_iter_at_start_false);

    CVXRUN(&t, test_bh_int_iter_at_end_true);
    CVXRUN(&t, test_bh_int_iter_at_end_false);

    CVXRUN(&t, test_bh_int_iter_count);

    CVXRUN(&t, test_bh_int_iter_to_start);
    CVXRUN(&t, test_bh_int_iter_to_end);

    CVXRUN(&t, test_bh_int_iter_next);
    CVXRUN(&t, test_bh_int_iter_next_at_end);

    CVXRUN(&t, test_bh_int_iter_prev);
    CVXRUN(&t, test_bh_int_iter_prev_at_start);

    CVXRUN(&t, test_bh_int_iter_forward);
    CVXRUN(&t, test_bh_int_iter_forward_clamp);

    CVXRUN(&t, test_bh_int_iter_backward);
    CVXRUN(&t, test_bh_int_iter_backward_clamp);

    CVXRUN(&t, test_bh_int_iter_go_to);
    CVXRUN(&t, test_bh_int_iter_go_to_end);
    CVXRUN(&t, test_bh_int_iter_go_to_out_of_range);

    CVXRUN(&t, test_bh_int_iter_value);
    CVXRUN(&t, test_bh_int_iter_value_root_is_max);
    CVXRUN(&t, test_bh_int_iter_value_at_end);

    CVXRUN(&t, test_bh_int_iter_index);

    return CVXSUMMARY(&t);
}

#endif /* BINARY_HEAP_ITER_TESTS_H */
