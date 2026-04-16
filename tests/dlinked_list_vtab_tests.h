#ifndef DLINKED_LIST_VTABV_TESTS_H
#define DLINKED_LIST_VTABV_TESTS_H

#include "cvxtest.h"
#include "implementations.h"

static void test_dll_vtabv_copy_called_on_copy(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    struct dlinked_int self = dll_int_init(dll_int_vtabv_full);
    dll_int_push_back(&self, 1);
    dll_int_push_back(&self, 2);
    dll_int_push_back(&self, 3);

    struct dlinked_int copy = dll_int_copy(&self);

    CVX_TEST_COUNTER_COPY(t, 3);
    CVXCHECK(t, copy.count == 3);
    CVXCHECK(t, dll_int_get(&copy, 0) == 1);
    CVXCHECK(t, dll_int_get(&copy, 1) == 2);
    CVXCHECK(t, dll_int_get(&copy, 2) == 3);

    dll_int_clear(&self);
    dll_int_clear(&copy);
}

static void test_dll_vtabv_copy_called_on_clone(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    struct dlinked_int *col = dll_int_new_with(dll_int_vtabv_full);
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);

    struct dlinked_int *clone = dll_int_clone(col);

    CVX_TEST_COUNTER_COPY(t, 3);
    CVXCHECK(t, dll_int_count(clone) == 3);
    CVXCHECK(t, dll_int_get(clone, 0) == 10);
    CVXCHECK(t, dll_int_get(clone, 1) == 20);
    CVXCHECK(t, dll_int_get(clone, 2) == 30);

    dll_int_drop(col);
    dll_int_drop(clone);
}

static void test_dll_vtabv_null_copy_no_crash(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_push_back(col, 7);
    dll_int_push_back(col, 8);
    dll_int_push_back(col, 9);

    struct dlinked_int *clone = dll_int_clone(col);

    CVXCHECK(t, clone != NULL);
    CVXCHECK(t, dll_int_count(clone) == 3);
    CVXCHECK(t, dll_int_get(clone, 0) == 7);
    CVXCHECK(t, dll_int_get(clone, 1) == 8);
    CVXCHECK(t, dll_int_get(clone, 2) == 9);

    dll_int_drop(col);
    dll_int_drop(clone);
}

static void test_dll_vtabv_drop_called_on_drop(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    struct dlinked_int *col = dll_int_new_with(dll_int_vtabv_full);
    dll_int_push_back(col, 1);
    dll_int_push_back(col, 2);
    dll_int_push_back(col, 3);

    dll_int_drop(col);

    CVX_TEST_COUNTER_DROP(t, 3);
}

static void test_dll_vtabv_null_drop_no_crash(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_push_back(col, 1);
    dll_int_push_back(col, 2);
    dll_int_push_back(col, 3);

    dll_int_drop(col);

    CVXCHECK(t, true);
}

static void test_dll_vtabv_drop_called_on_clear(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    struct dlinked_int *col = dll_int_new_with(dll_int_vtabv_full);
    dll_int_push_back(col, 1);
    dll_int_push_back(col, 2);
    dll_int_push_back(col, 3);

    dll_int_clear(col);

    CVX_TEST_COUNTER_DROP(t, 3);

    dll_int_drop(col);
}

static struct cvxresult run_dlinked_list_vtab_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dlinked_list vtabv\n");

    CVXRUN(&t, test_dll_vtabv_copy_called_on_copy);
    CVXRUN(&t, test_dll_vtabv_copy_called_on_clone);
    CVXRUN(&t, test_dll_vtabv_null_copy_no_crash);

    CVXRUN(&t, test_dll_vtabv_drop_called_on_drop);
    CVXRUN(&t, test_dll_vtabv_null_drop_no_crash);
    CVXRUN(&t, test_dll_vtabv_drop_called_on_clear);

    return CVXSUMMARY(&t);
}

#endif /* DLINKED_LIST_VTABV_TESTS_H */
