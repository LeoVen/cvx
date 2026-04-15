#ifndef DLINKED_LIST_GUARD_TESTS_H
#define DLINKED_LIST_GUARD_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

static void test_dll_int_guard_clone(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *result = dll_int__proxy_clone(col);
    CVXCHECK(t, result == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_drop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_clear(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_clear(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_count(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = dll_int__proxy_count(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_empty(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = dll_int__proxy_empty(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = dll_int__proxy_front(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = dll_int__proxy_back(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_get(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = dll_int__proxy_get(col, 0);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_push_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_push_front(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_push_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_push_back(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_push_at(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_push_at(col, 1, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_pop_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_pop_front(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_pop_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_pop_back(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_pop_at(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_pop_at(col, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_replace_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_replace_front(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_replace_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_replace_back(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_wrong_tag(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    col->super.tag = 0;
    dll_int__proxy_push_back((cvx_container *)col, 1);

    CVXCHECK(t, col->super.flag == CVX_FLAG_WRONG_TAG);

    col->super.tag = 88;
    dll_int_drop(col);
}

/* ---- iter wrong tag guards ---- */

static void test_dll_int_iter_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *iter = dll_int__proxy_iter_start(col);

    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *iter = dll_int__proxy_iter_end(col);

    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_drop_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_at_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = dll_int__proxy_iter_at_start(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_at_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = dll_int__proxy_iter_at_end(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_count_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = dll_int__proxy_iter_count(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_to_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_to_start(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_to_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_to_end(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_next_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_next(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_prev_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_prev(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_forward_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_forward(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_backward_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_backward(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_value_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int val = dll_int__proxy_iter_value(col);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_index_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = dll_int__proxy_iter_index(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_init_start / iter_init_end wrong tag ---- */

static void test_dll_int_guard_iter_init_start(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    struct dlinked_int_iter it = dll_int__proxy_iter_init_start(col);
    CVXCHECK(t, it.super.flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_guard_iter_init_end(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    struct dlinked_int_iter it = dll_int__proxy_iter_init_end(col);
    CVXCHECK(t, it.super.flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static int run_dlinked_list_guard_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dlinked_list (guards)\n");

    CVXRUN(&t, test_dll_int_guard_clone);
    CVXRUN(&t, test_dll_int_guard_drop);
    CVXRUN(&t, test_dll_int_guard_clear);
    CVXRUN(&t, test_dll_int_guard_count);
    CVXRUN(&t, test_dll_int_guard_empty);
    CVXRUN(&t, test_dll_int_guard_front);
    CVXRUN(&t, test_dll_int_guard_back);
    CVXRUN(&t, test_dll_int_guard_get);
    CVXRUN(&t, test_dll_int_guard_push_front);
    CVXRUN(&t, test_dll_int_guard_push_back);
    CVXRUN(&t, test_dll_int_guard_push_at);
    CVXRUN(&t, test_dll_int_guard_pop_front);
    CVXRUN(&t, test_dll_int_guard_pop_back);
    CVXRUN(&t, test_dll_int_guard_pop_at);
    CVXRUN(&t, test_dll_int_guard_replace_front);
    CVXRUN(&t, test_dll_int_guard_replace_back);

    CVXRUN(&t, test_dll_int_guard_iter_init_start);
    CVXRUN(&t, test_dll_int_guard_iter_init_end);
    CVXRUN(&t, test_dll_int_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_start_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_end_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_drop_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_at_start_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_at_end_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_count_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_to_start_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_to_end_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_next_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_prev_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_forward_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_backward_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_value_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_index_wrong_tag);

    return CVXSUMMARY(&t);
}

#endif /* DLINKED_LIST_GUARD_TESTS_H */
