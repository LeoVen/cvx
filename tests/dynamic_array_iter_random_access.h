#ifndef DYNAMIC_ARRAY_ITER_RANDOM_ACCESS_H
#define DYNAMIC_ARRAY_ITER_RANDOM_ACCESS_H

#include "cvx/interface_macros.h"
#include "cvxtest.h"

/* Define the rai interface type */
#define V int
#define INTERFACE rai
#include "cvx/iter/random_access_iterator.h"

/* Instantiate dynamic_array with rai cast (TAG=93, distinct from all other container TAGs) */
#define V int
#define SNAME da_rai_int
#define PFX da_rai
#define TAG 93
#define IMPL_RANDOM_ACCESS_ITER rai
#include "cvx/dynamic_array.h"

/* Helper: allocate a filled collection and return a rai wrapping its heap iterator */
#define MAKE_RAI(name, col) \
    cvx_container *col = da_rai_new(); \
    da_rai_push_back(col, 10); \
    da_rai_push_back(col, 20); \
    da_rai_push_back(col, 30); \
    struct rai name = da_rai_iter_as_rai(da_rai_iter_start(col))

/* Check the flag stored on the underlying iterator instance */
#define CHECK_ITER_FLAG(t, iter, expected) \
    CVXCHECK(t, ((cvx_container *)((iter).instance))->flag == (expected))

/* ---- Construction ---- */

static void test_rai_start(struct cvxtest *t)
{
    /* Wrap the *collection* so start() can create a fresh iterator */
    cvx_container *col = da_rai_new();
    struct rai col_rai = da_rai_iter_as_rai(col);
    cvx_container *new_iter = cvx_start(&col_rai);
    CVXCHECK(t, new_iter != NULL);
    if (new_iter)
        da_rai_iter_drop(new_iter);
    da_rai_drop(col);
}

static void test_rai_end(struct cvxtest *t)
{
    /* Wrap the *collection* so end() can create an end iterator */
    cvx_container *col = da_rai_new();
    da_rai_push_back(col, 10);
    da_rai_push_back(col, 20);
    da_rai_push_back(col, 30);
    struct rai col_rai = da_rai_iter_as_rai(col);
    cvx_container *end_iter = col_rai.vtable->end(col_rai.instance);
    CVXCHECK(t, end_iter != NULL);
    if (end_iter)
    {
        CVXCHECK(t, da_rai_iter_at_end(end_iter) == true);
        da_rai_iter_drop(end_iter);
    }
    da_rai_drop(col);
}

/* ---- State ---- */

static void test_rai_at_start(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    CVXCHECK(t, cvx_at_start(&iter) == true);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_at_end_false(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    CVXCHECK(t, cvx_at_end(&iter) == false);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_at_end_true(struct cvxtest *t)
{
    cvx_container *col = da_rai_new();
    da_rai_push_back(col, 10);
    da_rai_push_back(col, 20);
    da_rai_push_back(col, 30);
    cvx_container *end_iter = da_rai_iter_end(col);
    struct rai iter = da_rai_iter_as_rai(end_iter);
    CVXCHECK(t, cvx_at_end(&iter) == true);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_count(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    CVXCHECK(t, cvx_count(&iter) == 3);
    cvx_drop(&iter);
    da_rai_drop(col);
}

/* ---- Movement ---- */

static void test_rai_next(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_next(&iter);
    CVXCHECK(t, cvx_index(&iter) == 1);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_prev(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_next(&iter);
    cvx_next(&iter);
    cvx_prev(&iter);
    CVXCHECK(t, cvx_index(&iter) == 1);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_to_start(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_next(&iter);
    cvx_next(&iter);
    cvx_to_start(&iter);
    CVXCHECK(t, cvx_at_start(&iter) == true);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_to_end(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_to_end(&iter);
    CVXCHECK(t, cvx_at_end(&iter) == true);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_forward(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    iter.vtable->forward(iter.instance, 2);
    CVXCHECK(t, cvx_index(&iter) == 2);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_backward(struct cvxtest *t)
{
    cvx_container *col = da_rai_new();
    da_rai_push_back(col, 10);
    da_rai_push_back(col, 20);
    da_rai_push_back(col, 30);
    cvx_container *end_iter = da_rai_iter_end(col);
    struct rai iter = da_rai_iter_as_rai(end_iter);
    iter.vtable->backward(iter.instance, 1);
    CVXCHECK(t, cvx_index(&iter) == 2);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_go_to(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    iter.vtable->go_to(iter.instance, 1);
    CVXCHECK(t, cvx_index(&iter) == 1);
    cvx_drop(&iter);
    da_rai_drop(col);
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
    da_rai_drop(col);
}

static void test_rai_index(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    CVXCHECK(t, cvx_index(&iter) == 0);
    cvx_next(&iter);
    CVXCHECK(t, cvx_index(&iter) == 1);
    cvx_drop(&iter);
    da_rai_drop(col);
}

/* ---- Errors through interface ---- */

static void test_rai_next_at_end(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_to_end(&iter);
    cvx_next(&iter);
    CHECK_ITER_FLAG(t, iter, CVX_FLAG_RANGE);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_prev_at_start(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_prev(&iter);
    CHECK_ITER_FLAG(t, iter, CVX_FLAG_RANGE);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_value_at_end(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    cvx_to_end(&iter);
    int val = cvx_value(&iter);
    CVXCHECK(t, val == 0);
    CHECK_ITER_FLAG(t, iter, CVX_FLAG_RANGE);
    cvx_drop(&iter);
    da_rai_drop(col);
}

static void test_rai_go_to_out_of_range(struct cvxtest *t)
{
    MAKE_RAI(iter, col);
    iter.vtable->go_to(iter.instance, cvx_count(&iter) + 1);
    CHECK_ITER_FLAG(t, iter, CVX_FLAG_RANGE);
    cvx_drop(&iter);
    da_rai_drop(col);
}

/* ---- runner ---- */

static int run_dynamic_array_iter_random_access_tests(void)
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
