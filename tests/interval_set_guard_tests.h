#ifndef INTERVAL_SET_GUARD_TESTS_H
#define INTERVAL_SET_GUARD_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

/* Container guards */

static void test_is_int_guard_clone(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *res = is_int__proxy_clone(col);
    CVXCHECK(t, res == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_drop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    is_int__proxy_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_clear(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    is_int__proxy_clear(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_count(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t res = is_int__proxy_count(col);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_empty(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool res = is_int__proxy_empty(col);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_add(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    is_int__proxy_add(col, 1, 5);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_remove(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    is_int__proxy_remove(col, 1, 5);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_contains(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool res = is_int__proxy_contains(col, 3);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_contains_interval(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool res = is_int__proxy_contains_interval(col, 1, 5);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_overlaps(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool res = is_int__proxy_overlaps(col, 1, 5);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* Iterator guards */

static void test_is_int_guard_iter_start(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *iter = is_int__proxy_iter_start(col);
    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_end(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *iter = is_int__proxy_iter_end(col);
    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_drop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    is_int__proxy_iter_drop(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_at_start(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    bool res = is_int__proxy_iter_at_start(iter);
    CVXCHECK(t, res == false);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_at_end(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    bool res = is_int__proxy_iter_at_end(iter);
    CVXCHECK(t, res == false);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_count(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    size_t res = is_int__proxy_iter_count(iter);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_to_start(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    is_int__proxy_iter_to_start(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_to_end(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    is_int__proxy_iter_to_end(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_next(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    is_int__proxy_iter_next(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_prev(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    is_int__proxy_iter_prev(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_forward(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    is_int__proxy_iter_forward(iter, 1);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_backward(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    is_int__proxy_iter_backward(iter, 1);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_entry(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    struct iset_int_entry res = is_int__proxy_iter_entry(iter);
    CVXCHECK(t, res.lo == 0 && res.hi == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_value_lo(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    int res = is_int__proxy_iter_value_lo(iter);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_value_hi(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    int res = is_int__proxy_iter_value_hi(iter);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_is_int_guard_iter_index(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    size_t res = is_int__proxy_iter_index(iter);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static struct cvxresult run_interval_set_guard_tests(void)
{
    struct cvxtest t = { 0 };

    printf("interval_set (guards)\n");

    CVXRUN(&t, test_is_int_guard_clone);
    CVXRUN(&t, test_is_int_guard_drop);
    CVXRUN(&t, test_is_int_guard_clear);
    CVXRUN(&t, test_is_int_guard_count);
    CVXRUN(&t, test_is_int_guard_empty);
    CVXRUN(&t, test_is_int_guard_add);
    CVXRUN(&t, test_is_int_guard_remove);
    CVXRUN(&t, test_is_int_guard_contains);
    CVXRUN(&t, test_is_int_guard_contains_interval);
    CVXRUN(&t, test_is_int_guard_overlaps);

    CVXRUN(&t, test_is_int_guard_iter_start);
    CVXRUN(&t, test_is_int_guard_iter_end);
    CVXRUN(&t, test_is_int_guard_iter_drop);
    CVXRUN(&t, test_is_int_guard_iter_at_start);
    CVXRUN(&t, test_is_int_guard_iter_at_end);
    CVXRUN(&t, test_is_int_guard_iter_count);
    CVXRUN(&t, test_is_int_guard_iter_to_start);
    CVXRUN(&t, test_is_int_guard_iter_to_end);
    CVXRUN(&t, test_is_int_guard_iter_next);
    CVXRUN(&t, test_is_int_guard_iter_prev);
    CVXRUN(&t, test_is_int_guard_iter_forward);
    CVXRUN(&t, test_is_int_guard_iter_backward);
    CVXRUN(&t, test_is_int_guard_iter_entry);
    CVXRUN(&t, test_is_int_guard_iter_value_lo);
    CVXRUN(&t, test_is_int_guard_iter_value_hi);
    CVXRUN(&t, test_is_int_guard_iter_index);

    return CVXSUMMARY(&t);
}

#endif /* INTERVAL_SET_GUARD_TESTS_H */
