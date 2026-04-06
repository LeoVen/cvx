#ifndef SLINKED_LIST_ITER_FORWARD_H
#define SLINKED_LIST_ITER_FORWARD_H

#include "cvx/interface_macros.h"
#include "tests/cvxtest.h"
#include "tests/implementations.h"

/* Helper: allocate a 3-element list and return a forward_iter wrapping its heap iterator */
#define MAKE_FI(name, col) \
    cvx_container *col = sll_int_new(); \
    sll_int_push_back(col, 10); \
    sll_int_push_back(col, 20); \
    sll_int_push_back(col, 30); \
    struct forward_iter name = sll_int_iter_as_forward_iter(sll_int_iter_start(col))

/* ---- Construction ---- */

static void test_fi_start(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();
    sll_int_push_back(col, 10);
    sll_int_push_back(col, 20);
    sll_int_push_back(col, 30);

    /* Wrap the collection so start() can create a fresh iterator */
    struct forward_iter col_fi = sll_int_iter_as_forward_iter(col);
    cvx_container *new_iter = cvx_start(&col_fi);

    CVXCHECK(t, new_iter != NULL);
    if (new_iter)
    {
        CVXCHECK(t, sll_int_iter_at_start(new_iter) == true);
        sll_int_iter_drop(new_iter);
    }

    sll_int_drop(col);
}

static void test_fi_drop(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    cvx_drop(&iter); /* must not crash */
    sll_int_drop(col);
}

/* ---- State ---- */

static void test_fi_at_start_true(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    CVXCHECK(t, cvx_at_start(&iter) == true);
    cvx_drop(&iter);
    sll_int_drop(col);
}

static void test_fi_at_start_false(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    cvx_next(&iter);
    CVXCHECK(t, cvx_at_start(&iter) == false);
    cvx_drop(&iter);
    sll_int_drop(col);
}

static void test_fi_at_end_false(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    CVXCHECK(t, cvx_at_end(&iter) == false);
    cvx_drop(&iter);
    sll_int_drop(col);
}

static void test_fi_at_end_true(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    cvx_forward(&iter, 3);
    CVXCHECK(t, cvx_at_end(&iter) == true);
    cvx_drop(&iter);
    sll_int_drop(col);
}

static void test_fi_at_start_and_end_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();
    struct forward_iter iter = sll_int_iter_as_forward_iter(sll_int_iter_start(col));
    CVXCHECK(t, cvx_at_start(&iter) == true);
    CVXCHECK(t, cvx_at_end(&iter) == true);
    cvx_drop(&iter);
    sll_int_drop(col);
}

static void test_fi_count(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    CVXCHECK(t, cvx_count(&iter) == 3);
    cvx_drop(&iter);
    sll_int_drop(col);
}

/* ---- Movement ---- */

static void test_fi_to_start(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    cvx_next(&iter);
    cvx_next(&iter);
    cvx_to_start(&iter);
    CVXCHECK(t, cvx_at_start(&iter) == true);
    CVXCHECK(t, cvx_index(&iter) == 0);
    CVXCHECK(t, cvx_value(&iter) == 10);
    cvx_drop(&iter);
    sll_int_drop(col);
}

static void test_fi_next(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    cvx_next(&iter);
    CVXCHECK(t, cvx_index(&iter) == 1);
    cvx_next(&iter);
    CVXCHECK(t, cvx_index(&iter) == 2);
    cvx_drop(&iter);
    sll_int_drop(col);
}

static void test_fi_next_at_end(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    cvx_forward(&iter, 3);
    cvx_next(&iter);
    CVXCHECK(t, cvx_flag(&iter) == CVX_FLAG_RANGE);
    cvx_drop(&iter);
    sll_int_drop(col);
}

static void test_fi_forward(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    cvx_forward(&iter, 2);
    CVXCHECK(t, cvx_index(&iter) == 2);
    CVXCHECK(t, cvx_value(&iter) == 30);
    cvx_drop(&iter);
    sll_int_drop(col);
}

static void test_fi_forward_range_error(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    cvx_forward(&iter, 100);
    CVXCHECK(t, cvx_flag(&iter) == CVX_FLAG_RANGE);
    cvx_drop(&iter);
    sll_int_drop(col);
}

/* ---- Access ---- */

static void test_fi_value(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    CVXCHECK(t, cvx_value(&iter) == 10);
    cvx_next(&iter);
    CVXCHECK(t, cvx_value(&iter) == 20);
    cvx_next(&iter);
    CVXCHECK(t, cvx_value(&iter) == 30);
    cvx_drop(&iter);
    sll_int_drop(col);
}

static void test_fi_value_at_end(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    cvx_forward(&iter, 3);
    int val = cvx_value(&iter);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, cvx_flag(&iter) == CVX_FLAG_RANGE);
    cvx_drop(&iter);
    sll_int_drop(col);
}

static void test_fi_index(struct cvxtest *t)
{
    MAKE_FI(iter, col);
    CVXCHECK(t, cvx_index(&iter) == 0);
    cvx_next(&iter);
    CVXCHECK(t, cvx_index(&iter) == 1);
    cvx_drop(&iter);
    sll_int_drop(col);
}

/* ---- runner ---- */

static int run_slinked_list_iter_forward_tests(void)
{
    struct cvxtest t = { 0 };
    printf("slinked_list (forward_iterator)\n");

    CVXRUN(&t, test_fi_start);
    CVXRUN(&t, test_fi_drop);

    CVXRUN(&t, test_fi_at_start_true);
    CVXRUN(&t, test_fi_at_start_false);
    CVXRUN(&t, test_fi_at_end_false);
    CVXRUN(&t, test_fi_at_end_true);
    CVXRUN(&t, test_fi_at_start_and_end_empty);
    CVXRUN(&t, test_fi_count);

    CVXRUN(&t, test_fi_to_start);
    CVXRUN(&t, test_fi_next);
    CVXRUN(&t, test_fi_next_at_end);
    CVXRUN(&t, test_fi_forward);
    CVXRUN(&t, test_fi_forward_range_error);

    CVXRUN(&t, test_fi_value);
    CVXRUN(&t, test_fi_value_at_end);
    CVXRUN(&t, test_fi_index);

    return CVXSUMMARY(&t);
}

#endif /* SLINKED_LIST_ITER_FORWARD_H */
