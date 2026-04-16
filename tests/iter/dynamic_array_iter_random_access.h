#ifndef DYNAMIC_ARRAY_ITER_RANDOM_ACCESS_H
#define DYNAMIC_ARRAY_ITER_RANDOM_ACCESS_H

#include "cvx/interface_macros.h"
#include "tests/cvxtest.h"
#include "tests/implementations.h"

/* Helper: allocate a filled collection and return a raccess_iter wrapping its heap iterator */
#define MAKE_RAI(name, col) \
    struct dynamic_array_int *col = da_int_new(); \
    da_int_push_back(col, 10); \
    da_int_push_back(col, 20); \
    da_int_push_back(col, 30); \
    struct raccess_iter name = da_int_iter_as_raccess_iter((cvx_container *)da_int_iter_start(col))

/* Check the flag stored on the underlying iterator instance */
#define CHECK_ITER_FLAG(t, iter, expected) \
    CVXCHECK(t, ((cvx_container *)((iter).instance))->flag == (expected))

/* ---- Construction ---- */

static void test_rai_start(struct cvxtest *t)
{
    /* Wrap the *collection* so start() can create a fresh iterator */
    struct dynamic_array_int *col = da_int_new();
    struct raccess_iter col_rai = da_int_iter_as_raccess_iter((cvx_container *)col);
    struct dynamic_array_int_iter *new_iter = (struct dynamic_array_int_iter *)cvx_start(&col_rai);
    CVXCHECK(t, new_iter != NULL);
    if (new_iter)
        da_int_iter_drop(new_iter);
    da_int_drop(col);
}

static void test_rai_end(struct cvxtest *t)
{
    /* Wrap the *collection* so end() can create an end iterator */
    struct dynamic_array_int *col = da_int_new();
    da_int_push_back(col, 10);
    da_int_push_back(col, 20);
    da_int_push_back(col, 30);
    struct raccess_iter col_rai = da_int_iter_as_raccess_iter((cvx_container *)col);
    struct dynamic_array_int_iter *end_iter = (struct dynamic_array_int_iter *)col_rai.vtable->end(col_rai.instance);
    CVXCHECK(t, end_iter != NULL);
    if (end_iter)
    {
        CVXCHECK(t, da_int_iter_at_end(end_iter) == true);
        da_int_iter_drop(end_iter);
    }
    da_int_drop(col);
}

/* ---- State ---- */

static void test_rai_at_start(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    CVXCHECK(t, cvx_at_start(&iter) == true);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_at_end_false(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    CVXCHECK(t, cvx_at_end(&iter) == false);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_at_end_true(struct cvxtest *t)
{
    struct dynamic_array_int *col = da_int_new();
    da_int_push_back(col, 10);
    da_int_push_back(col, 20);
    da_int_push_back(col, 30);
    struct dynamic_array_int_iter *end_iter = da_int_iter_end(col);
    struct raccess_iter iter = da_int_iter_as_raccess_iter((cvx_container *)end_iter);
    CVXCHECK(t, cvx_at_end(&iter) == true);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_count(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    CVXCHECK(t, cvx_count(&iter) == 3);
    cvx_drop(&iter);
    da_int_drop(col);
}

/* ---- Movement ---- */

static void test_rai_next(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_next(&iter);
    CVXCHECK(t, cvx_index(&iter) == 1);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_prev(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_next(&iter);
    cvx_next(&iter);
    cvx_prev(&iter);
    CVXCHECK(t, cvx_index(&iter) == 1);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_to_start(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_next(&iter);
    cvx_next(&iter);
    cvx_to_start(&iter);
    CVXCHECK(t, cvx_at_start(&iter) == true);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_to_end(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_to_end(&iter);
    CVXCHECK(t, cvx_at_end(&iter) == true);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_forward(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    iter.vtable->forward(iter.instance, 2);
    CVXCHECK(t, cvx_index(&iter) == 2);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_backward(struct cvxtest *t)
{
    struct dynamic_array_int *col = da_int_new();
    da_int_push_back(col, 10);
    da_int_push_back(col, 20);
    da_int_push_back(col, 30);
    struct dynamic_array_int_iter *end_iter = da_int_iter_end(col);
    struct raccess_iter iter = da_int_iter_as_raccess_iter((cvx_container *)end_iter);
    iter.vtable->backward(iter.instance, 1);
    CVXCHECK(t, cvx_index(&iter) == 2);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_go_to(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    iter.vtable->go_to(iter.instance, 1);
    CVXCHECK(t, cvx_index(&iter) == 1);
    cvx_drop(&iter);
    da_int_drop(col);
}

/* ---- Access ---- */

static void test_rai_value(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    CVXCHECK(t, cvx_value(&iter) == 10);
    cvx_next(&iter);
    CVXCHECK(t, cvx_value(&iter) == 20);
    cvx_next(&iter);
    CVXCHECK(t, cvx_value(&iter) == 30);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_index(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    CVXCHECK(t, cvx_index(&iter) == 0);
    cvx_next(&iter);
    CVXCHECK(t, cvx_index(&iter) == 1);
    cvx_drop(&iter);
    da_int_drop(col);
}

/* ---- Errors through interface ---- */

static void test_rai_next_at_end(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_to_end(&iter);
    cvx_next(&iter);
    CHECK_ITER_FLAG(t, iter, CVX_FLAG_RANGE);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_prev_at_start(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_prev(&iter);
    CHECK_ITER_FLAG(t, iter, CVX_FLAG_RANGE);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_value_at_end(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_to_end(&iter);
    int val = cvx_value(&iter);
    CVXCHECK(t, val == 0);
    CHECK_ITER_FLAG(t, iter, CVX_FLAG_RANGE);
    cvx_drop(&iter);
    da_int_drop(col);
}

static void test_rai_go_to_out_of_range(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    iter.vtable->go_to(iter.instance, cvx_count(&iter) + 1);
    CHECK_ITER_FLAG(t, iter, CVX_FLAG_RANGE);
    cvx_drop(&iter);
    da_int_drop(col);
}

/* ---- runner ---- */

static struct cvxresult run_dynamic_array_iter_random_access_tests(void)
{
    struct cvxtest t = { 0 };
    printf("dynamic_array (random_access_iterator)\n");

    CVXRUN(&t, test_rai_start);
    CVXRUN(&t, test_rai_end);

    CVXRUN(&t, test_rai_at_start);
    CVXRUN(&t, test_rai_at_end_false);
    CVXRUN(&t, test_rai_at_end_true);
    CVXRUN(&t, test_rai_count);

    CVXRUN(&t, test_rai_next);
    CVXRUN(&t, test_rai_prev);
    CVXRUN(&t, test_rai_to_start);
    CVXRUN(&t, test_rai_to_end);
    CVXRUN(&t, test_rai_forward);
    CVXRUN(&t, test_rai_backward);
    CVXRUN(&t, test_rai_go_to);

    CVXRUN(&t, test_rai_value);
    CVXRUN(&t, test_rai_index);

    CVXRUN(&t, test_rai_next_at_end);
    CVXRUN(&t, test_rai_prev_at_start);
    CVXRUN(&t, test_rai_value_at_end);
    CVXRUN(&t, test_rai_go_to_out_of_range);

    return CVXSUMMARY(&t);
}

#endif /* DYNAMIC_ARRAY_ITER_RANDOM_ACCESS_H */
