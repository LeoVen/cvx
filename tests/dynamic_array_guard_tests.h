#ifndef DYNAMIC_ARRAY_GUARD_TESTS_H
#define DYNAMIC_ARRAY_GUARD_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#define V int
#define SNAME da_guard_int
#define PFX da_guard
#define TAG 94
#include "cvx/dynamic_array.h"

/*
 * Guard tests for dynamic_array.h
 *
 * Every function that opens with CVX_CONTAINER_GUARDS must set
 * CVX_FLAG_WRONG_TAG and return its error value when the container tag does
 * not match.  Each test below creates an invalid container via
 * MAKE_INVALID_CONTAINER and verifies that contract.
 */

static void test_da_guard_clone(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *result = da_guard_clone(col);
    CVXCHECK(t, result == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_drop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_guard_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_clear(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_guard_clear(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_count(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = da_guard_count(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_capacity(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = da_guard_capacity(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_full(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = da_guard_full(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_empty(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = da_guard_empty(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = da_guard_front(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = da_guard_back(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_get(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = da_guard_get(col, 0);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_push_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_guard_push_front(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_push_at(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_guard_push_at(col, 1, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_push_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_guard_push_back(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_pop_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_guard_pop_front(col, NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_pop_at(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_guard_pop_at(col, NULL, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_pop_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_guard_pop_back(col, NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_replace_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_guard_replace_front(col, 1, NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_da_guard_replace_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    da_guard_replace_back(col, 1, NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static int run_dynamic_array_guard_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dynamic_array (guards)\n");

    CVXRUN(&t, test_da_guard_clone);
    CVXRUN(&t, test_da_guard_drop);
    CVXRUN(&t, test_da_guard_clear);
    CVXRUN(&t, test_da_guard_count);
    CVXRUN(&t, test_da_guard_capacity);
    CVXRUN(&t, test_da_guard_full);
    CVXRUN(&t, test_da_guard_empty);
    CVXRUN(&t, test_da_guard_front);
    CVXRUN(&t, test_da_guard_back);
    CVXRUN(&t, test_da_guard_get);
    CVXRUN(&t, test_da_guard_push_front);
    CVXRUN(&t, test_da_guard_push_at);
    CVXRUN(&t, test_da_guard_push_back);
    CVXRUN(&t, test_da_guard_pop_front);
    CVXRUN(&t, test_da_guard_pop_at);
    CVXRUN(&t, test_da_guard_pop_back);
    CVXRUN(&t, test_da_guard_replace_front);
    CVXRUN(&t, test_da_guard_replace_back);

    return CVXSUMMARY(&t);
}

#endif /* DYNAMIC_ARRAY_GUARD_TESTS_H */
