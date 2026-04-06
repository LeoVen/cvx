#ifndef DYNAMIC_ARRAY_ITER_TESTS_H
#define DYNAMIC_ARRAY_ITER_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

static void da_int_iter_fill3(cvx_container *col)
{
    da_int_push_back(col, 10);
    da_int_push_back(col, 20);
    da_int_push_back(col, 30);
}

/* ---- da_int_iter_init_start / da_int_iter_init_end ---- */

static void test_da_int_iter_init_start(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    struct dynamic_array_int_iter iter = da_int_iter_init_start(col);
    CVXCHECK(t, iter.super.tag == DA_ITER_TAG);
    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter.index == 0);
    da_int_drop(col);
}

static void test_da_int_iter_init_end(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    struct dynamic_array_int_iter iter = da_int_iter_init_end(col);
    CVXCHECK(t, iter.super.tag == DA_ITER_TAG);
    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter.index == 3);
    da_int_drop(col);
}

static void test_da_int_iter_init_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    struct dynamic_array_int_iter iter = da_int_iter_init_start(invalid);
    CVXCHECK(t, iter.super.flag == CVX_FLAG_WRONG_TAG);
}

/* ---- da_int_iter_start / da_int_iter_end ---- */

static void test_da_int_iter_start(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    cvx_container *iter = da_int_iter_start(col);
    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        da_int_drop(col);
        return;
    }
    CVXCHECK(t, iter->tag == DA_ITER_TAG);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, da_int_iter_index(iter) == 0);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_end(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_end(col);
    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        da_int_drop(col);
        return;
    }
    CVXCHECK(t, iter->tag == DA_ITER_TAG);
    CVXCHECK(t, da_int_iter_index(iter) == 3);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    cvx_container *iter = da_int_iter_start(invalid);
    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    cvx_container *iter = da_int_iter_end(invalid);
    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- da_int_iter_drop ---- */

static void test_da_int_iter_drop(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    cvx_container *iter = da_int_iter_start(col);
    CVXCHECK(t, iter != NULL);
    if (iter)
        da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_drop_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int_iter_drop(invalid);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- da_int_iter_at_start / da_int_iter_at_end ---- */

static void test_da_int_iter_at_start_true(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    cvx_container *iter = da_int_iter_start(col);
    CVXCHECK(t, da_int_iter_at_start(iter) == true);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_at_start_false(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    da_int_iter_next(iter);
    CVXCHECK(t, da_int_iter_at_start(iter) == false);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_at_end_true(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_end(col);
    CVXCHECK(t, da_int_iter_at_end(iter) == true);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_at_end_false(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    CVXCHECK(t, da_int_iter_at_end(iter) == false);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_at_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bool result = da_int_iter_at_start(invalid);
    CVXCHECK(t, result == false);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_at_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bool result = da_int_iter_at_end(invalid);
    CVXCHECK(t, result == false);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- da_int_iter_count ---- */

static void test_da_int_iter_count(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    for (int i = 0; i < 5; i++)
        da_int_push_back(col, i);
    cvx_container *iter = da_int_iter_start(col);
    CVXCHECK(t, da_int_iter_count(iter) == 5);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_count_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    size_t count = da_int_iter_count(invalid);
    CVXCHECK(t, count == 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- da_int_iter_to_start / da_int_iter_to_end ---- */

static void test_da_int_iter_to_start(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    da_int_iter_next(iter);
    da_int_iter_next(iter);
    da_int_iter_to_start(iter);
    CVXCHECK(t, da_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_to_end(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    da_int_iter_to_end(iter);
    CVXCHECK(t, da_int_iter_at_end(iter) == true);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

/* ---- da_int_iter_next ---- */

static void test_da_int_iter_next(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    CVXCHECK(t, da_int_iter_index(iter) == 0);
    da_int_iter_next(iter);
    CVXCHECK(t, da_int_iter_index(iter) == 1);
    da_int_iter_next(iter);
    CVXCHECK(t, da_int_iter_index(iter) == 2);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_next_at_end(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_end(col);
    da_int_iter_next(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_RANGE);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_next_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int_iter_next(invalid);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- da_int_iter_prev ---- */

static void test_da_int_iter_prev(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_end(col);
    CVXCHECK(t, da_int_iter_index(iter) == 3);
    da_int_iter_prev(iter);
    CVXCHECK(t, da_int_iter_index(iter) == 2);
    da_int_iter_prev(iter);
    CVXCHECK(t, da_int_iter_index(iter) == 1);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_prev_at_start(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    cvx_container *iter = da_int_iter_start(col);
    da_int_iter_prev(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_RANGE);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_prev_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int_iter_prev(invalid);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- da_int_iter_forward ---- */

static void test_da_int_iter_forward(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    da_int_iter_forward(iter, 2);
    CVXCHECK(t, da_int_iter_index(iter) == 2);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_forward_clamp(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    da_int_iter_forward(iter, 100);
    CVXCHECK(t, da_int_iter_index(iter) == 3);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_forward_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int_iter_forward(invalid, 1);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- da_int_iter_backward ---- */

static void test_da_int_iter_backward(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_end(col);
    da_int_iter_backward(iter, 2);
    CVXCHECK(t, da_int_iter_index(iter) == 1);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_backward_clamp(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_end(col);
    da_int_iter_backward(iter, 100);
    CVXCHECK(t, da_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_backward_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int_iter_backward(invalid, 1);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- da_int_iter_go_to ---- */

static void test_da_int_iter_go_to(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    da_int_iter_go_to(iter, 2);
    CVXCHECK(t, da_int_iter_index(iter) == 2);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_go_to_end(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    da_int_iter_go_to(iter, 3);
    CVXCHECK(t, da_int_iter_at_end(iter) == true);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_go_to_out_of_range(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    da_int_iter_go_to(iter, 4);
    CVXCHECK(t, iter->flag == CVX_FLAG_RANGE);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_go_to_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int_iter_go_to(invalid, 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- da_int_iter_value ---- */

static void test_da_int_iter_value(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    CVXCHECK(t, da_int_iter_value(iter) == 10);
    da_int_iter_next(iter);
    CVXCHECK(t, da_int_iter_value(iter) == 20);
    da_int_iter_next(iter);
    CVXCHECK(t, da_int_iter_value(iter) == 30);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_value_at_end(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_end(col);
    int val = da_int_iter_value(iter);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_RANGE);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_value_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    int val = da_int_iter_value(invalid);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- da_int_iter_index ---- */

static void test_da_int_iter_index(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    da_int_iter_fill3(col);
    cvx_container *iter = da_int_iter_start(col);
    CVXCHECK(t, da_int_iter_index(iter) == 0);
    da_int_iter_next(iter);
    CVXCHECK(t, da_int_iter_index(iter) == 1);
    da_int_iter_next(iter);
    CVXCHECK(t, da_int_iter_index(iter) == 2);
    da_int_iter_drop(iter);
    da_int_drop(col);
}

static void test_da_int_iter_index_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    size_t idx = da_int_iter_index(invalid);
    CVXCHECK(t, idx == 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static int run_dynamic_array_iter_tests(void)
{
    struct cvxtest t = { 0 };
    printf("dynamic_array (iterator)\n");

    CVXRUN(&t, test_da_int_iter_init_start);
    CVXRUN(&t, test_da_int_iter_init_end);
    CVXRUN(&t, test_da_int_iter_init_start_wrong_tag);

    CVXRUN(&t, test_da_int_iter_start);
    CVXRUN(&t, test_da_int_iter_end);
    CVXRUN(&t, test_da_int_iter_start_wrong_tag);
    CVXRUN(&t, test_da_int_iter_end_wrong_tag);

    CVXRUN(&t, test_da_int_iter_drop);
    CVXRUN(&t, test_da_int_iter_drop_wrong_tag);

    CVXRUN(&t, test_da_int_iter_at_start_true);
    CVXRUN(&t, test_da_int_iter_at_start_false);
    CVXRUN(&t, test_da_int_iter_at_end_true);
    CVXRUN(&t, test_da_int_iter_at_end_false);
    CVXRUN(&t, test_da_int_iter_at_start_wrong_tag);
    CVXRUN(&t, test_da_int_iter_at_end_wrong_tag);

    CVXRUN(&t, test_da_int_iter_count);
    CVXRUN(&t, test_da_int_iter_count_wrong_tag);

    CVXRUN(&t, test_da_int_iter_to_start);
    CVXRUN(&t, test_da_int_iter_to_end);

    CVXRUN(&t, test_da_int_iter_next);
    CVXRUN(&t, test_da_int_iter_next_at_end);
    CVXRUN(&t, test_da_int_iter_next_wrong_tag);

    CVXRUN(&t, test_da_int_iter_prev);
    CVXRUN(&t, test_da_int_iter_prev_at_start);
    CVXRUN(&t, test_da_int_iter_prev_wrong_tag);

    CVXRUN(&t, test_da_int_iter_forward);
    CVXRUN(&t, test_da_int_iter_forward_clamp);
    CVXRUN(&t, test_da_int_iter_forward_wrong_tag);

    CVXRUN(&t, test_da_int_iter_backward);
    CVXRUN(&t, test_da_int_iter_backward_clamp);
    CVXRUN(&t, test_da_int_iter_backward_wrong_tag);

    CVXRUN(&t, test_da_int_iter_go_to);
    CVXRUN(&t, test_da_int_iter_go_to_end);
    CVXRUN(&t, test_da_int_iter_go_to_out_of_range);
    CVXRUN(&t, test_da_int_iter_go_to_wrong_tag);

    CVXRUN(&t, test_da_int_iter_value);
    CVXRUN(&t, test_da_int_iter_value_at_end);
    CVXRUN(&t, test_da_int_iter_value_wrong_tag);

    CVXRUN(&t, test_da_int_iter_index);
    CVXRUN(&t, test_da_int_iter_index_wrong_tag);

    return CVXSUMMARY(&t);
}

#endif /* DYNAMIC_ARRAY_ITER_TESTS_H */
