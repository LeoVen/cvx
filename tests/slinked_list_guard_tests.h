#ifndef SLINKED_LIST_GUARD_TESTS_H
#define SLINKED_LIST_GUARD_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#define V int
#define SNAME sll_guard_int
#define PFX sll_guard
#define TAG 95
#include "cvx/slinked_list.h"

/*
 * Guard tests for slinked_list.h
 *
 * Every function that opens with CVX_CONTAINER_GUARDS must set
 * CVX_FLAG_WRONG_TAG and return its error value when the container tag does
 * not match.  Each test below creates an invalid container via
 * MAKE_INVALID_CONTAINER and verifies that contract.
 */

static void test_sll_guard_clone(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *result = sll_guard_clone(col);
    CVXCHECK(t, result == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_drop(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_guard_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_clear(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_guard_clear(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_count(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = sll_guard_count(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_empty(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = sll_guard_empty(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = sll_guard_front(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = sll_guard_back(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_get(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int result = sll_guard_get(col, 0);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_push_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_guard_push_front(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_push_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_guard_push_back(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_push_at(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_guard_push_at(col, 1, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_pop_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_guard_pop_front(col, NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_pop_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_guard_pop_back(col, NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_pop_at(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_guard_pop_at(col, NULL, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_replace_front(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_guard_replace_front(col, 1, NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_guard_replace_back(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_guard_replace_back(col, 1, NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static int run_slinked_list_guard_tests(void)
{
    struct cvxtest t = { 0 };

    printf("slinked_list (guards)\n");

    CVXRUN(&t, test_sll_guard_clone);
    CVXRUN(&t, test_sll_guard_drop);
    CVXRUN(&t, test_sll_guard_clear);
    CVXRUN(&t, test_sll_guard_count);
    CVXRUN(&t, test_sll_guard_empty);
    CVXRUN(&t, test_sll_guard_front);
    CVXRUN(&t, test_sll_guard_back);
    CVXRUN(&t, test_sll_guard_get);
    CVXRUN(&t, test_sll_guard_push_front);
    CVXRUN(&t, test_sll_guard_push_back);
    CVXRUN(&t, test_sll_guard_push_at);
    CVXRUN(&t, test_sll_guard_pop_front);
    CVXRUN(&t, test_sll_guard_pop_back);
    CVXRUN(&t, test_sll_guard_pop_at);
    CVXRUN(&t, test_sll_guard_replace_front);
    CVXRUN(&t, test_sll_guard_replace_back);

    return CVXSUMMARY(&t);
}

#endif /* SLINKED_LIST_GUARD_TESTS_H */
