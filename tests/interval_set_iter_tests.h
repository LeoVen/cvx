#ifndef INTERVAL_SET_ITER_TESTS_H
#define INTERVAL_SET_ITER_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

/* Helper: create [1,5) [10,15) [20,25) */
static struct iset_int *is_int_make3(void)
{
    struct iset_int *col = is_int_new_with(is_int_vtabv_comp_only);
    is_int_add(col, 1, 5);
    is_int_add(col, 10, 15);
    is_int_add(col, 20, 25);
    return col;
}

/* ---- iter_init_start ---- */

static void test_is_int_iter_init_start(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();

    struct iset_int_iter it = is_int_iter_init_start(col);

    CVXCHECK(t, it.super.tag == (size_t)IS_ITER_TAG);
    CVXCHECK(t, it.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 0);
    CVXCHECK(t, it.target == col);

    is_int_drop(col);
}

static void test_is_int_iter_init_start_empty(struct cvxtest *t)
{
    struct iset_int *col = is_int_new_with(is_int_vtabv_comp_only);

    struct iset_int_iter it = is_int_iter_init_start(col);

    CVXCHECK(t, it.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 0);

    is_int_drop(col);
}

/* ---- iter_init_end ---- */

static void test_is_int_iter_init_end(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();

    struct iset_int_iter it = is_int_iter_init_end(col);

    CVXCHECK(t, it.super.tag == (size_t)IS_ITER_TAG);
    CVXCHECK(t, it.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 3);

    is_int_drop(col);
}

/* ---- iter_start / iter_end ---- */

static void test_is_int_iter_start(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();

    struct iset_int_iter *iter = is_int_iter_start(col);

    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        is_int_drop(col);
        return;
    }

    CVXCHECK(t, iter->super.tag == (size_t)IS_ITER_TAG);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter->index == 0);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

static void test_is_int_iter_end(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();

    struct iset_int_iter *iter = is_int_iter_end(col);

    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        is_int_drop(col);
        return;
    }

    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter->index == 3);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

/* ---- iter_at_start / iter_at_end ---- */

static void test_is_int_iter_at_start(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_start(col);

    CVXCHECK(t, is_int_iter_at_start(iter) == true);
    is_int_iter_next(iter);
    CVXCHECK(t, is_int_iter_at_start(iter) == false);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

static void test_is_int_iter_at_end(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_end(col);

    CVXCHECK(t, is_int_iter_at_end(iter) == true);
    is_int_iter_prev(iter);
    CVXCHECK(t, is_int_iter_at_end(iter) == false);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

/* ---- iter_count ---- */

static void test_is_int_iter_count(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_start(col);

    CVXCHECK(t, is_int_iter_count(iter) == 3);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

/* ---- iter_to_start / iter_to_end ---- */

static void test_is_int_iter_to_start(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_start(col);

    is_int_iter_next(iter);
    is_int_iter_next(iter);
    is_int_iter_to_start(iter);

    CVXCHECK(t, is_int_iter_at_start(iter) == true);
    CVXCHECK(t, iter->index == 0);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

static void test_is_int_iter_to_end(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_start(col);

    is_int_iter_to_end(iter);

    CVXCHECK(t, is_int_iter_at_end(iter) == true);
    CVXCHECK(t, iter->index == 3);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

/* ---- iter_next / iter_prev ---- */

static void test_is_int_iter_next_traversal(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_start(col);

    CVXCHECK(t, is_int_iter_value_lo(iter) == 1);
    CVXCHECK(t, is_int_iter_value_hi(iter) == 5);

    is_int_iter_next(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 10);
    CVXCHECK(t, is_int_iter_value_hi(iter) == 15);

    is_int_iter_next(iter);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 20);
    CVXCHECK(t, is_int_iter_value_hi(iter) == 25);

    is_int_iter_next(iter);
    CVXCHECK(t, is_int_iter_at_end(iter) == true);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

static void test_is_int_iter_next_at_end(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_end(col);

    is_int_iter_next(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

static void test_is_int_iter_prev_traversal(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_end(col);

    is_int_iter_prev(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 20);

    is_int_iter_prev(iter);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 10);

    is_int_iter_prev(iter);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 1);
    CVXCHECK(t, is_int_iter_at_start(iter) == true);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

static void test_is_int_iter_prev_at_start(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_start(col);

    is_int_iter_prev(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

/* ---- iter_forward / iter_backward ---- */

static void test_is_int_iter_forward(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_start(col);

    is_int_iter_forward(iter, 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 20);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

static void test_is_int_iter_forward_clamps(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_start(col);

    is_int_iter_forward(iter, 100);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_iter_at_end(iter) == true);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

static void test_is_int_iter_backward(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_end(col);

    is_int_iter_backward(iter, 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_iter_value_lo(iter) == 10);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

static void test_is_int_iter_backward_clamps(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_end(col);

    is_int_iter_backward(iter, 100);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_iter_at_start(iter) == true);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

/* ---- iter_value / iter_value_lo / iter_value_hi ---- */

static void test_is_int_iter_value(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_start(col);

    struct iset_int_entry e = is_int_iter_value(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, e.lo == 1);
    CVXCHECK(t, e.hi == 5);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

static void test_is_int_iter_value_at_end(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_end(col);

    is_int_iter_value(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

static void test_is_int_iter_value_lo_hi(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_start(col);

    CVXCHECK(t, is_int_iter_value_lo(iter) == 1);
    CVXCHECK(t, is_int_iter_value_hi(iter) == 5);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

/* ---- iter_index ---- */

static void test_is_int_iter_index(struct cvxtest *t)
{
    struct iset_int *col = is_int_make3();
    struct iset_int_iter *iter = is_int_iter_start(col);

    CVXCHECK(t, is_int_iter_index(iter) == 0);
    is_int_iter_next(iter);
    CVXCHECK(t, is_int_iter_index(iter) == 1);
    is_int_iter_next(iter);
    CVXCHECK(t, is_int_iter_index(iter) == 2);

    is_int_iter_drop(iter);
    is_int_drop(col);
}

/* ---- runner ---- */

static int run_interval_set_iter_tests(void)
{
    struct cvxtest t = { 0 };

    printf("interval_set (iter)\n");

    CVXRUN(&t, test_is_int_iter_init_start);
    CVXRUN(&t, test_is_int_iter_init_start_empty);
    CVXRUN(&t, test_is_int_iter_init_end);

    CVXRUN(&t, test_is_int_iter_start);
    CVXRUN(&t, test_is_int_iter_end);

    CVXRUN(&t, test_is_int_iter_at_start);
    CVXRUN(&t, test_is_int_iter_at_end);
    CVXRUN(&t, test_is_int_iter_count);

    CVXRUN(&t, test_is_int_iter_to_start);
    CVXRUN(&t, test_is_int_iter_to_end);

    CVXRUN(&t, test_is_int_iter_next_traversal);
    CVXRUN(&t, test_is_int_iter_next_at_end);

    CVXRUN(&t, test_is_int_iter_prev_traversal);
    CVXRUN(&t, test_is_int_iter_prev_at_start);

    CVXRUN(&t, test_is_int_iter_forward);
    CVXRUN(&t, test_is_int_iter_forward_clamps);
    CVXRUN(&t, test_is_int_iter_backward);
    CVXRUN(&t, test_is_int_iter_backward_clamps);

    CVXRUN(&t, test_is_int_iter_value);
    CVXRUN(&t, test_is_int_iter_value_at_end);
    CVXRUN(&t, test_is_int_iter_value_lo_hi);
    CVXRUN(&t, test_is_int_iter_index);

    return CVXSUMMARY(&t);
}

#endif /* INTERVAL_SET_ITER_TESTS_H */
