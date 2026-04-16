#ifndef BINARY_HEAP_GUARD_TESTS_H
#define BINARY_HEAP_GUARD_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

/* ---- container proxy guards ---- */

static void test_bh_int_guard_clone(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *result = bh_int__proxy_clone(col);
    CVXCHECK(t, result == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_guard_drop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_guard_clear(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_clear(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_guard_flag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    enum cvx_flags result = bh_int__proxy_flag(col);
    CVXCHECK(t, result == CVX_FLAG_WRONG_TAG);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_guard_count(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = bh_int__proxy_count(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_guard_capacity(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = bh_int__proxy_capacity(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_guard_empty(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = bh_int__proxy_empty(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_guard_full(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = bh_int__proxy_full(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_guard_push(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bh_int__proxy_push(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_guard_pop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = bh_int__proxy_pop(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_guard_peek(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = bh_int__proxy_peek(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_wrong_tag(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    h->super.tag = 0; /* corrupt the tag */
    bh_int__proxy_push((cvx_container *)h, 1);

    CVXCHECK(t, h->super.flag == CVX_FLAG_WRONG_TAG);

    /* restore tag so drop works */
    h->super.tag = 33;
    bh_int_drop(h);
}

/* ---- iterator proxy guards ---- */

static void test_bh_int_iter_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    cvx_container *iter = bh_int__proxy_iter_start(invalid);
    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    cvx_container *iter = bh_int__proxy_iter_end(invalid);
    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_drop_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bh_int__proxy_iter_drop(invalid);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_at_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bool result = bh_int__proxy_iter_at_start(invalid);
    CVXCHECK(t, result == false);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_at_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bool result = bh_int__proxy_iter_at_end(invalid);
    CVXCHECK(t, result == false);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_count_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    size_t count = bh_int__proxy_iter_count(invalid);
    CVXCHECK(t, count == 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_to_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bh_int__proxy_iter_to_start(invalid);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_to_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bh_int__proxy_iter_to_end(invalid);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_next_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bh_int__proxy_iter_next(invalid);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_prev_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bh_int__proxy_iter_prev(invalid);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_forward_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bh_int__proxy_iter_forward(invalid, 1);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_backward_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bh_int__proxy_iter_backward(invalid, 1);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_go_to_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bh_int__proxy_iter_go_to(invalid, 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_value_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    int val = bh_int__proxy_iter_value(invalid);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_bh_int_iter_index_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    size_t idx = bh_int__proxy_iter_index(invalid);
    CVXCHECK(t, idx == 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static struct cvxresult run_binary_heap_guard_tests(void)
{
    struct cvxtest t = { 0 };

    printf("binary_heap (guards)\n");

    CVXRUN(&t, test_bh_int_guard_clone);
    CVXRUN(&t, test_bh_int_guard_drop);
    CVXRUN(&t, test_bh_int_guard_clear);
    CVXRUN(&t, test_bh_int_guard_flag);
    CVXRUN(&t, test_bh_int_guard_count);
    CVXRUN(&t, test_bh_int_guard_capacity);
    CVXRUN(&t, test_bh_int_guard_empty);
    CVXRUN(&t, test_bh_int_guard_full);
    CVXRUN(&t, test_bh_int_guard_push);
    CVXRUN(&t, test_bh_int_guard_pop);
    CVXRUN(&t, test_bh_int_guard_peek);
    CVXRUN(&t, test_bh_int_wrong_tag);

    CVXRUN(&t, test_bh_int_iter_start_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_end_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_drop_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_at_start_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_at_end_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_count_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_to_start_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_to_end_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_next_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_prev_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_forward_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_backward_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_go_to_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_value_wrong_tag);
    CVXRUN(&t, test_bh_int_iter_index_wrong_tag);

    return CVXSUMMARY(&t);
}

#endif /* BINARY_HEAP_GUARD_TESTS_H */
