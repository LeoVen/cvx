#ifndef DYNAMIC_ARRAY_VTABV_TESTS_H
#define DYNAMIC_ARRAY_VTABV_TESTS_H

#include "cvxtest.h"
#include "implementations.h"

static void test_da_vtabv_copy_called_on_copy(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    struct dynamic_array_int *col = da_int_new_with(da_int_vtabv_full, 8);
    da_int_push_back(col, 1);
    da_int_push_back(col, 2);
    da_int_push_back(col, 3);

    struct dynamic_array_int copy = da_int_copy(col);

    CVX_TEST_COUNTER_COPY(t, 3);
    CVXCHECK(t, copy.count == 3);
    CVXCHECK(t, copy.buffer[0] == 1);
    CVXCHECK(t, copy.buffer[1] == 2);
    CVXCHECK(t, copy.buffer[2] == 3);

    free(copy.buffer);
    da_int_drop(col);
}

static void test_da_vtabv_copy_called_on_clone(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    struct dynamic_array_int *col = da_int_new_with(da_int_vtabv_full, 8);
    da_int_push_back(col, 10);
    da_int_push_back(col, 20);
    da_int_push_back(col, 30);

    struct dynamic_array_int *clone = da_int_clone(col);

    CVX_TEST_COUNTER_COPY(t, 3);
    CVXCHECK(t, da_int_count(clone) == 3);
    CVXCHECK(t, da_int_get(clone, 0) == 10);
    CVXCHECK(t, da_int_get(clone, 1) == 20);
    CVXCHECK(t, da_int_get(clone, 2) == 30);

    da_int_drop(col);
    da_int_drop(clone);
}

static void test_da_vtabv_null_copy_no_crash(struct cvxtest *t)
{
    struct dynamic_array_int *col = da_int_new_with(NULL, 8);
    da_int_push_back(col, 7);
    da_int_push_back(col, 8);
    da_int_push_back(col, 9);

    struct dynamic_array_int *clone = da_int_clone(col);

    CVXCHECK(t, clone != NULL);
    CVXCHECK(t, da_int_count(clone) == 3);
    CVXCHECK(t, da_int_get(clone, 0) == 7);
    CVXCHECK(t, da_int_get(clone, 1) == 8);
    CVXCHECK(t, da_int_get(clone, 2) == 9);

    da_int_drop(col);
    da_int_drop(clone);
}

static void test_da_vtabv_drop_called_on_drop(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    struct dynamic_array_int *col = da_int_new_with(da_int_vtabv_full, 8);
    da_int_push_back(col, 1);
    da_int_push_back(col, 2);
    da_int_push_back(col, 3);

    da_int_drop(col);

    CVX_TEST_COUNTER_DROP(t, 3);
}

static void test_da_vtabv_null_drop_no_crash(struct cvxtest *t)
{
    struct dynamic_array_int *col = da_int_new_with(NULL, 8);
    da_int_push_back(col, 1);
    da_int_push_back(col, 2);
    da_int_push_back(col, 3);

    da_int_drop(col);

    CVXCHECK(t, true);
}

static void test_da_vtabv_drop_called_on_clear(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    struct dynamic_array_int *col = da_int_new_with(da_int_vtabv_full, 8);
    da_int_push_back(col, 1);
    da_int_push_back(col, 2);
    da_int_push_back(col, 3);

    da_int_clear(col);

    CVX_TEST_COUNTER_DROP(t, 3);

    da_int_drop(col);
}

static struct cvxresult run_dynamic_array_vtab_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dynamic_array vtabv\n");

    CVXRUN(&t, test_da_vtabv_copy_called_on_copy);
    CVXRUN(&t, test_da_vtabv_copy_called_on_clone);
    CVXRUN(&t, test_da_vtabv_null_copy_no_crash);

    CVXRUN(&t, test_da_vtabv_drop_called_on_drop);
    CVXRUN(&t, test_da_vtabv_null_drop_no_crash);
    CVXRUN(&t, test_da_vtabv_drop_called_on_clear);

    return CVXSUMMARY(&t);
}

#endif /* DYNAMIC_ARRAY_VTABV_TESTS_H */
