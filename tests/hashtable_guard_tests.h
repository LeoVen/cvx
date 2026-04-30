#ifndef HASHTABLE_GUARD_TESTS_H
#define HASHTABLE_GUARD_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

/* Container guards */

static void test_ht_int_guard_clone(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *res = ht_int__proxy_clone(col);
    CVXCHECK(t, res == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_drop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    ht_int__proxy_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_clear(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    ht_int__proxy_clear(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_count(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t res = ht_int__proxy_count(col);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_capacity(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t res = ht_int__proxy_capacity(col);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_empty(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool res = ht_int__proxy_empty(col);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_insert(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool res = ht_int__proxy_insert(col, 1, 10);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_update(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool res = ht_int__proxy_update(col, 1, 10, NULL);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_remove(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool res = ht_int__proxy_remove(col, 1, NULL);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_get(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int res = ht_int__proxy_get(col, 1);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_contains(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool res = ht_int__proxy_contains(col, 1);
    CVXCHECK(t, res == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* Iterator guards */

static void test_ht_int_guard_iter_start(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *res = ht_int__proxy_iter_start(col);
    CVXCHECK(t, res == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_end(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *res = ht_int__proxy_iter_end(col);
    CVXCHECK(t, res == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_drop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    ht_int__proxy_iter_drop(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_at_start(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    bool res = ht_int__proxy_iter_at_start(iter);
    CVXCHECK(t, res == false);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_at_end(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    bool res = ht_int__proxy_iter_at_end(iter);
    CVXCHECK(t, res == false);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_count(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    size_t res = ht_int__proxy_iter_count(iter);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_to_start(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    ht_int__proxy_iter_to_start(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_to_end(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    ht_int__proxy_iter_to_end(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_next(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    ht_int__proxy_iter_next(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_prev(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    ht_int__proxy_iter_prev(iter);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_forward(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    ht_int__proxy_iter_forward(iter, 1);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_backward(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    ht_int__proxy_iter_backward(iter, 1);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_key(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    int res = ht_int__proxy_iter_key(iter);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_value(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    int res = ht_int__proxy_iter_value(iter);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_guard_iter_index(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(iter);
    size_t res = ht_int__proxy_iter_index(iter);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static struct cvxresult run_hashtable_guard_tests(void)
{
    struct cvxtest t = { 0 };

    printf("hashtable (guards)\n");

    CVXRUN(&t, test_ht_int_guard_clone);
    CVXRUN(&t, test_ht_int_guard_drop);
    CVXRUN(&t, test_ht_int_guard_clear);
    CVXRUN(&t, test_ht_int_guard_count);
    CVXRUN(&t, test_ht_int_guard_capacity);
    CVXRUN(&t, test_ht_int_guard_empty);
    CVXRUN(&t, test_ht_int_guard_insert);
    CVXRUN(&t, test_ht_int_guard_update);
    CVXRUN(&t, test_ht_int_guard_remove);
    CVXRUN(&t, test_ht_int_guard_get);
    CVXRUN(&t, test_ht_int_guard_contains);

    CVXRUN(&t, test_ht_int_guard_iter_start);
    CVXRUN(&t, test_ht_int_guard_iter_end);
    CVXRUN(&t, test_ht_int_guard_iter_drop);
    CVXRUN(&t, test_ht_int_guard_iter_at_start);
    CVXRUN(&t, test_ht_int_guard_iter_at_end);
    CVXRUN(&t, test_ht_int_guard_iter_count);
    CVXRUN(&t, test_ht_int_guard_iter_to_start);
    CVXRUN(&t, test_ht_int_guard_iter_to_end);
    CVXRUN(&t, test_ht_int_guard_iter_next);
    CVXRUN(&t, test_ht_int_guard_iter_prev);
    CVXRUN(&t, test_ht_int_guard_iter_forward);
    CVXRUN(&t, test_ht_int_guard_iter_backward);
    CVXRUN(&t, test_ht_int_guard_iter_key);
    CVXRUN(&t, test_ht_int_guard_iter_value);
    CVXRUN(&t, test_ht_int_guard_iter_index);

    return CVXSUMMARY(&t);
}

#endif /* HASHTABLE_GUARD_TESTS_H */
