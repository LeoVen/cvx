#ifndef DYNAMIC_ARRAY_GUARD_TESTS_H
#define DYNAMIC_ARRAY_GUARD_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

static void test_da_int_guard_clone(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *result = da_int__proxy_clone(col);
    CVXCHECK(t, result == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_drop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_int__proxy_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_clear(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_int__proxy_clear(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_count(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = da_int__proxy_count(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_capacity(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = da_int__proxy_capacity(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_empty(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = da_int__proxy_empty(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_full(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = da_int__proxy_full(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = da_int__proxy_front(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = da_int__proxy_back(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_get(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = da_int__proxy_get(col, 0);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_push_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_int__proxy_push_front(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_push_at(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_int__proxy_push_at(col, 1, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_push_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_int__proxy_push_back(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_pop_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_int__proxy_pop_front(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_pop_at(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_int__proxy_pop_at(col, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_pop_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_int__proxy_pop_back(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_replace_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_int__proxy_replace_front(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_guard_replace_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_int__proxy_replace_back(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_wrong_tag(struct cvxtest *t)
{
    struct dynamic_array_int *col = da_int_new_with(NULL, 4);
    col->super.tag = 0; /* corrupt the tag */
    da_int__proxy_push_back((cvx_container *)col, 1);

    CVXCHECK(t, col->super.flag == CVX_FLAG_WRONG_TAG);

    /* restore tag so drop works */
    col->super.tag = 99;
    da_int_drop(col);
}

/* ---- iterator wrong tag guards ---- */

static void test_da_int_iter_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    cvx_container *iter = da_int__proxy_iter_start(invalid);
    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    cvx_container *iter = da_int__proxy_iter_end(invalid);
    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_drop_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int__proxy_iter_drop(invalid);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_at_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bool result = da_int__proxy_iter_at_start(invalid);
    CVXCHECK(t, result == false);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_at_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    bool result = da_int__proxy_iter_at_end(invalid);
    CVXCHECK(t, result == false);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_count_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    size_t count = da_int__proxy_iter_count(invalid);
    CVXCHECK(t, count == 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_next_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int__proxy_iter_next(invalid);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_prev_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int__proxy_iter_prev(invalid);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_forward_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int__proxy_iter_forward(invalid, 1);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_backward_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int__proxy_iter_backward(invalid, 1);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_go_to_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    da_int__proxy_iter_go_to(invalid, 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_value_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    int val = da_int__proxy_iter_value(invalid);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_int_iter_index_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(invalid);
    size_t idx = da_int__proxy_iter_index(invalid);
    CVXCHECK(t, idx == 0);
    CVXCHECK(t, invalid->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static struct cvxresult run_dynamic_array_guard_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dynamic_array (guards)\n");

    CVXRUN(&t, test_da_int_guard_clone);
    CVXRUN(&t, test_da_int_guard_drop);
    CVXRUN(&t, test_da_int_guard_clear);
    CVXRUN(&t, test_da_int_guard_count);
    CVXRUN(&t, test_da_int_guard_capacity);
    CVXRUN(&t, test_da_int_guard_empty);
    CVXRUN(&t, test_da_int_guard_full);
    CVXRUN(&t, test_da_int_guard_front);
    CVXRUN(&t, test_da_int_guard_back);
    CVXRUN(&t, test_da_int_guard_get);
    CVXRUN(&t, test_da_int_guard_push_front);
    CVXRUN(&t, test_da_int_guard_push_at);
    CVXRUN(&t, test_da_int_guard_push_back);
    CVXRUN(&t, test_da_int_guard_pop_front);
    CVXRUN(&t, test_da_int_guard_pop_at);
    CVXRUN(&t, test_da_int_guard_pop_back);
    CVXRUN(&t, test_da_int_guard_replace_front);
    CVXRUN(&t, test_da_int_guard_replace_back);
    CVXRUN(&t, test_da_int_wrong_tag);

    CVXRUN(&t, test_da_int_iter_start_wrong_tag);
    CVXRUN(&t, test_da_int_iter_end_wrong_tag);
    CVXRUN(&t, test_da_int_iter_drop_wrong_tag);
    CVXRUN(&t, test_da_int_iter_at_start_wrong_tag);
    CVXRUN(&t, test_da_int_iter_at_end_wrong_tag);
    CVXRUN(&t, test_da_int_iter_count_wrong_tag);
    CVXRUN(&t, test_da_int_iter_next_wrong_tag);
    CVXRUN(&t, test_da_int_iter_prev_wrong_tag);
    CVXRUN(&t, test_da_int_iter_forward_wrong_tag);
    CVXRUN(&t, test_da_int_iter_backward_wrong_tag);
    CVXRUN(&t, test_da_int_iter_go_to_wrong_tag);
    CVXRUN(&t, test_da_int_iter_value_wrong_tag);
    CVXRUN(&t, test_da_int_iter_index_wrong_tag);

    return CVXSUMMARY(&t);
}

#endif /* DYNAMIC_ARRAY_GUARD_TESTS_H */
