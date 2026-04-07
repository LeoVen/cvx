#ifndef SLINKED_LIST_VTABV_TESTS_H
#define SLINKED_LIST_VTABV_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"
#include "implementations.h"

static void test_sll_vtabv_copy_called_on_copy(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    struct slinked_int self = sll_int_init(sll_int_vtabv_full);
    sll_int_push_back(cvx_col(self), 1);
    sll_int_push_back(cvx_col(self), 2);
    sll_int_push_back(cvx_col(self), 3);

    struct slinked_int copy = sll_int_copy(&self);

    CVX_TEST_COUNTER_COPY(t, 3);
    CVXCHECK(t, copy.count == 3);
    CVXCHECK(t, sll_int_get(cvx_col(copy), 0) == 1);
    CVXCHECK(t, sll_int_get(cvx_col(copy), 1) == 2);
    CVXCHECK(t, sll_int_get(cvx_col(copy), 2) == 3);

    sll_int_clear(cvx_col(self));
    sll_int_clear(cvx_col(copy));
}

static void test_sll_vtabv_copy_called_on_clone(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    cvx_container *col = sll_int_new_with(sll_int_vtabv_full);
    sll_int_push_back(col, 10);
    sll_int_push_back(col, 20);
    sll_int_push_back(col, 30);

    cvx_container *clone = sll_int_clone(col);

    CVX_TEST_COUNTER_COPY(t, 3);
    CVXCHECK(t, sll_int_count(clone) == 3);
    CVXCHECK(t, sll_int_get(clone, 0) == 10);
    CVXCHECK(t, sll_int_get(clone, 1) == 20);
    CVXCHECK(t, sll_int_get(clone, 2) == 30);

    sll_int_drop(col);
    sll_int_drop(clone);
}

static void test_sll_vtabv_drop_called_on_drop(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    cvx_container *col = sll_int_new_with(sll_int_vtabv_full);
    sll_int_push_back(col, 1);
    sll_int_push_back(col, 2);
    sll_int_push_back(col, 3);

    sll_int_drop(col);

    CVX_TEST_COUNTER_DROP(t, 3);
}

static void test_sll_vtabv_drop_called_on_clear(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    cvx_container *col = sll_int_new_with(sll_int_vtabv_full);
    sll_int_push_back(col, 1);
    sll_int_push_back(col, 2);
    sll_int_push_back(col, 3);

    sll_int_clear(col);

    CVX_TEST_COUNTER_DROP(t, 3);

    sll_int_drop(col);
}

static void test_sll_vtabv_null_copy_no_crash(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();
    sll_int_push_back(col, 7);
    sll_int_push_back(col, 8);
    sll_int_push_back(col, 9);

    cvx_container *clone = sll_int_clone(col);

    CVXCHECK(t, clone != NULL);
    CVXCHECK(t, sll_int_count(clone) == 3);
    CVXCHECK(t, sll_int_get(clone, 0) == 7);
    CVXCHECK(t, sll_int_get(clone, 1) == 8);
    CVXCHECK(t, sll_int_get(clone, 2) == 9);

    sll_int_drop(col);
    sll_int_drop(clone);
}

static void test_sll_vtabv_null_drop_no_crash(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();
    sll_int_push_back(col, 1);
    sll_int_push_back(col, 2);
    sll_int_push_back(col, 3);

    sll_int_drop(col);

    CVXCHECK(t, true);
}

static int run_slinked_list_vtabv_tests(void)
{
    struct cvxtest t = { 0 };

    printf("slinked_list vtabv\n");

    CVXRUN(&t, test_sll_vtabv_copy_called_on_copy);
    CVXRUN(&t, test_sll_vtabv_copy_called_on_clone);
    CVXRUN(&t, test_sll_vtabv_drop_called_on_drop);
    CVXRUN(&t, test_sll_vtabv_drop_called_on_clear);
    CVXRUN(&t, test_sll_vtabv_null_copy_no_crash);
    CVXRUN(&t, test_sll_vtabv_null_drop_no_crash);

    return CVXSUMMARY(&t);
}

#endif /* SLINKED_LIST_VTABV_TESTS_H */
