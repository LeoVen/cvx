#ifndef SLINKED_LIST_ITER_TESTS_H
#define SLINKED_LIST_ITER_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#define V int
#define SNAME sll_it_int
#define PFX sll_it
#define TAG 76
#include "cvx/slinked_list.h"

#define SLL_ITER_TAG (76 * CVX_ITER_TAG_MULT)

/* Helper: fill list col with values 10, 20, 30 */
static void sll_it_fill3(cvx_container *col)
{
    sll_it_push_back(col, 10);
    sll_it_push_back(col, 20);
    sll_it_push_back(col, 30);
}

/* ---- iter_init_start ---- */

static void test_sll_it_init_start(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    struct sll_it_int_iter it = sll_it_iter_init_start(col);
    cvx_container *iter = (cvx_container *)&it;

    CVXCHECK(t, iter->tag == (size_t)SLL_ITER_TAG);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 0);

    sll_it_drop(col);
}

static void test_sll_it_init_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    struct sll_it_int_iter it = sll_it_iter_init_start(col);
    cvx_container *iter = (cvx_container *)&it;

    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_start ---- */

static void test_sll_it_start(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);

    CVXCHECK(t, iter->tag == (size_t)SLL_ITER_TAG);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, ((struct sll_it_int_iter *)iter)->index == 0);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *iter = sll_it_iter_start(col);

    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_drop ---- */

static void test_sll_it_drop(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);
    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        sll_it_drop(col);
        return;
    }

    sll_it_iter_drop(iter); /* must not crash */

    sll_it_drop(col);
}

static void test_sll_it_drop_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_it_iter_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_at_start ---- */

static void test_sll_it_at_start(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);
    CVXCHECK(t, sll_it_iter_at_start(iter) == true);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_at_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = sll_it_iter_at_start(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_at_end ---- */

static void test_sll_it_at_end(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);
    sll_it_iter_to_end(iter);
    CVXCHECK(t, sll_it_iter_at_end(iter) == true);

    sll_it_iter_to_start(iter);
    CVXCHECK(t, sll_it_iter_at_start(iter) == true);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_at_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = sll_it_iter_at_end(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_count ---- */

static void test_sll_it_count(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    for (int i = 0; i < 5; i++)
        sll_it_push_back(col, i);

    cvx_container *iter = sll_it_iter_start(col);
    CVXCHECK(t, sll_it_iter_count(iter) == 5);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_count_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = sll_it_iter_count(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_to_end ---- */

static void test_sll_it_to_end(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);
    sll_it_iter_to_end(iter);

    CVXCHECK(t, sll_it_iter_at_end(iter) == true);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_to_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_it_iter_to_end(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_next ---- */

static void test_sll_it_next(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);
    CVXCHECK(t, sll_it_iter_index(iter) == 0);

    sll_it_iter_next(iter);
    CVXCHECK(t, sll_it_iter_index(iter) == 1);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);

    sll_it_iter_next(iter);
    CVXCHECK(t, sll_it_iter_index(iter) == 2);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_next_at_end(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);
    sll_it_iter_to_end(iter);
    sll_it_iter_next(iter);

    CVXCHECK(t, iter->flag == CVX_FLAG_RANGE);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_next_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_it_iter_next(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_forward ---- */

static void test_sll_it_forward(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);
    sll_it_iter_forward(iter, 2);

    CVXCHECK(t, sll_it_iter_index(iter) == 2);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_forward_range_error(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);
    sll_it_iter_forward(iter, 100);

    CVXCHECK(t, iter->flag == CVX_FLAG_RANGE);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_forward_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    sll_it_iter_forward(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_value ---- */

static void test_sll_it_value(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);
    CVXCHECK(t, sll_it_iter_value(iter) == 10);

    sll_it_iter_next(iter);
    CVXCHECK(t, sll_it_iter_value(iter) == 20);

    sll_it_iter_next(iter);
    CVXCHECK(t, sll_it_iter_value(iter) == 30);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_value_at_end(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);
    sll_it_iter_to_end(iter);

    int val = sll_it_iter_value(iter);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_RANGE);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_value_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int val = sll_it_iter_value(col);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_index ---- */

static void test_sll_it_index(struct cvxtest *t)
{
    cvx_container *col = sll_it_new();
    sll_it_fill3(col);

    cvx_container *iter = sll_it_iter_start(col);
    CVXCHECK(t, sll_it_iter_index(iter) == 0);

    sll_it_iter_next(iter);
    CVXCHECK(t, sll_it_iter_index(iter) == 1);

    sll_it_iter_next(iter);
    CVXCHECK(t, sll_it_iter_index(iter) == 2);

    sll_it_iter_drop(iter);
    sll_it_drop(col);
}

static void test_sll_it_index_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = sll_it_iter_index(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static int run_slinked_list_iter_tests(void)
{
    struct cvxtest t = { 0 };

    printf("slinked_list (iterator)\n");

    CVXRUN(&t, test_sll_it_init_start);
    CVXRUN(&t, test_sll_it_init_start_wrong_tag);

    CVXRUN(&t, test_sll_it_start);
    CVXRUN(&t, test_sll_it_start_wrong_tag);

    CVXRUN(&t, test_sll_it_drop);
    CVXRUN(&t, test_sll_it_drop_wrong_tag);

    CVXRUN(&t, test_sll_it_at_start);
    CVXRUN(&t, test_sll_it_at_start_wrong_tag);

    CVXRUN(&t, test_sll_it_at_end);
    CVXRUN(&t, test_sll_it_at_end_wrong_tag);

    CVXRUN(&t, test_sll_it_count);
    CVXRUN(&t, test_sll_it_count_wrong_tag);

    CVXRUN(&t, test_sll_it_to_end);
    CVXRUN(&t, test_sll_it_to_end_wrong_tag);

    CVXRUN(&t, test_sll_it_next);
    CVXRUN(&t, test_sll_it_next_at_end);
    CVXRUN(&t, test_sll_it_next_wrong_tag);

    CVXRUN(&t, test_sll_it_forward);
    CVXRUN(&t, test_sll_it_forward_range_error);
    CVXRUN(&t, test_sll_it_forward_wrong_tag);

    CVXRUN(&t, test_sll_it_value);
    CVXRUN(&t, test_sll_it_value_at_end);
    CVXRUN(&t, test_sll_it_value_wrong_tag);

    CVXRUN(&t, test_sll_it_index);
    CVXRUN(&t, test_sll_it_index_wrong_tag);

    return CVXSUMMARY(&t);
}

#endif /* SLINKED_LIST_ITER_TESTS_H */
