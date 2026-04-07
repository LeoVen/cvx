#ifndef DLINKED_LIST_ITER_TESTS_H
#define DLINKED_LIST_ITER_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

/* Helper: fill list col with values 10, 20, 30 */
static void dll_int_fill3(cvx_container *col)
{
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);
}

/* ---- iter_init_start ---- */

static void test_dll_int_iter_init_start(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter it = dll_int_iter_init_start(col);
    cvx_container *iter = (cvx_container *)&it;

    CVXCHECK(t, iter->tag == (size_t)DLL_ITER_TAG);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 0);
    CVXCHECK(t, it.cursor == ((struct dlinked_int *)col)->head);

    dll_int_drop(col);
}

static void test_dll_int_iter_init_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    struct dlinked_int_iter it = dll_int_iter_init_start(col);
    cvx_container *iter = (cvx_container *)&it;

    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_init_end ---- */

static void test_dll_int_iter_init_end(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter it = dll_int_iter_init_end(col);
    cvx_container *iter = (cvx_container *)&it;

    CVXCHECK(t, iter->tag == (size_t)DLL_ITER_TAG);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 3);
    CVXCHECK(t, it.cursor == NULL);

    dll_int_drop(col);
}

static void test_dll_int_iter_init_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    struct dlinked_int_iter it = dll_int_iter_init_end(col);
    cvx_container *iter = (cvx_container *)&it;

    CVXCHECK(t, iter->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_start ---- */

static void test_dll_int_iter_start(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);

    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        dll_int_drop(col);
        return;
    }
    CVXCHECK(t, iter->tag == (size_t)DLL_ITER_TAG);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, ((struct dlinked_int_iter *)iter)->index == 0);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *iter = dll_int_iter_start(col);

    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_end ---- */

static void test_dll_int_iter_end(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_end(col);

    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        dll_int_drop(col);
        return;
    }
    CVXCHECK(t, iter->tag == (size_t)DLL_ITER_TAG);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, ((struct dlinked_int_iter *)iter)->index == 3);
    CVXCHECK(t, ((struct dlinked_int_iter *)iter)->cursor == NULL);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    cvx_container *iter = dll_int_iter_end(col);

    CVXCHECK(t, iter == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_drop ---- */

static void test_dll_int_iter_drop(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        dll_int_drop(col);
        return;
    }

    dll_int_iter_drop(iter); /* must not crash */

    dll_int_drop(col);
}

static void test_dll_int_iter_drop_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int_iter_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_at_start ---- */

static void test_dll_int_iter_at_start_true(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_at_start(iter) == true);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_at_start_false(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_at_start(iter) == false);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_at_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = dll_int_iter_at_start(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_at_end ---- */

static void test_dll_int_iter_at_end_true(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_end(col);
    CVXCHECK(t, dll_int_iter_at_end(iter) == true);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_at_end_false(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_at_end(iter) == false);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_at_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    bool result = dll_int_iter_at_end(col);
    CVXCHECK(t, result == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_empty_at_start_and_end(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();

    cvx_container *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_at_start(iter) == true);
    CVXCHECK(t, dll_int_iter_at_end(iter) == true);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_count ---- */

static void test_dll_int_iter_count(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    for (int i = 0; i < 5; i++)
        dll_int_push_back(col, i);

    cvx_container *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_count(iter) == 5);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_count_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = dll_int_iter_count(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_to_start ---- */

static void test_dll_int_iter_to_start(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    dll_int_iter_next(iter);
    dll_int_iter_next(iter);
    dll_int_iter_to_start(iter);

    CVXCHECK(t, dll_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_value(iter) == 10);
    CVXCHECK(t, ((struct dlinked_int_iter *)iter)->cursor == ((struct dlinked_int *)col)->head);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_to_start_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int_iter_to_start(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_to_end ---- */

static void test_dll_int_iter_to_end(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    dll_int_iter_to_end(iter);

    CVXCHECK(t, dll_int_iter_at_end(iter) == true);
    CVXCHECK(t, dll_int_iter_index(iter) == 3);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, ((struct dlinked_int_iter *)iter)->cursor == NULL);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_to_end_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int_iter_to_end(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_next ---- */

static void test_dll_int_iter_next(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_index(iter) == 0);

    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);

    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 2);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_next_at_end(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_end(col);
    dll_int_iter_next(iter);

    CVXCHECK(t, iter->flag == CVX_FLAG_RANGE);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_next_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int_iter_next(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_prev ---- */

static void test_dll_int_iter_prev(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_end(col);
    CVXCHECK(t, dll_int_iter_index(iter) == 3);

    dll_int_iter_prev(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 2);
    CVXCHECK(t, dll_int_iter_value(iter) == 30);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);

    dll_int_iter_prev(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    CVXCHECK(t, dll_int_iter_value(iter) == 20);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_prev_at_start(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    dll_int_iter_prev(iter);

    CVXCHECK(t, iter->flag == CVX_FLAG_RANGE);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_prev_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int_iter_prev(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_forward ---- */

static void test_dll_int_iter_forward(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    dll_int_iter_forward(iter, 2);

    CVXCHECK(t, dll_int_iter_index(iter) == 2);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_value(iter) == 30);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_forward_clamp(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    dll_int_iter_forward(iter, 100);

    CVXCHECK(t, dll_int_iter_index(iter) == 3);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_at_end(iter) == true);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_forward_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int_iter_forward(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_backward ---- */

static void test_dll_int_iter_backward(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_end(col);
    dll_int_iter_backward(iter, 2);

    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_value(iter) == 20);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_backward_clamp(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_end(col);
    dll_int_iter_backward(iter, 100);

    CVXCHECK(t, dll_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_at_start(iter) == true);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_backward_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    dll_int_iter_backward(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- round-trip: next then prev ---- */

static void test_dll_int_iter_next_prev_roundtrip(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    dll_int_iter_next(iter);
    dll_int_iter_next(iter);
    dll_int_iter_prev(iter);

    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    CVXCHECK(t, dll_int_iter_value(iter) == 20);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_value ---- */

static void test_dll_int_iter_value(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_value(iter) == 10);

    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_value(iter) == 20);

    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_value(iter) == 30);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_value_at_end(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_end(col);
    int val = dll_int_iter_value(iter);

    CVXCHECK(t, val == 0);
    CVXCHECK(t, iter->flag == CVX_FLAG_RANGE);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_value_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    int val = dll_int_iter_value(col);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- iter_index ---- */

static void test_dll_int_iter_index(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_fill3(col);

    cvx_container *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_index(iter) == 0);

    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 1);

    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 2);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_index_wrong_tag(struct cvxtest *t)
{
    MAKE_INVALID_CONTAINER(col);
    size_t result = dll_int_iter_index(col);
    CVXCHECK(t, result == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- runner ---- */

static int run_dlinked_list_iter_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dlinked_list (iterator)\n");

    CVXRUN(&t, test_dll_int_iter_init_start);
    CVXRUN(&t, test_dll_int_iter_init_start_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_init_end);
    CVXRUN(&t, test_dll_int_iter_init_end_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_start);
    CVXRUN(&t, test_dll_int_iter_start_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_end);
    CVXRUN(&t, test_dll_int_iter_end_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_drop);
    CVXRUN(&t, test_dll_int_iter_drop_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_at_start_true);
    CVXRUN(&t, test_dll_int_iter_at_start_false);
    CVXRUN(&t, test_dll_int_iter_at_start_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_at_end_true);
    CVXRUN(&t, test_dll_int_iter_at_end_false);
    CVXRUN(&t, test_dll_int_iter_at_end_wrong_tag);
    CVXRUN(&t, test_dll_int_iter_empty_at_start_and_end);

    CVXRUN(&t, test_dll_int_iter_count);
    CVXRUN(&t, test_dll_int_iter_count_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_to_start);
    CVXRUN(&t, test_dll_int_iter_to_start_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_to_end);
    CVXRUN(&t, test_dll_int_iter_to_end_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_next);
    CVXRUN(&t, test_dll_int_iter_next_at_end);
    CVXRUN(&t, test_dll_int_iter_next_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_prev);
    CVXRUN(&t, test_dll_int_iter_prev_at_start);
    CVXRUN(&t, test_dll_int_iter_prev_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_forward);
    CVXRUN(&t, test_dll_int_iter_forward_clamp);
    CVXRUN(&t, test_dll_int_iter_forward_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_backward);
    CVXRUN(&t, test_dll_int_iter_backward_clamp);
    CVXRUN(&t, test_dll_int_iter_backward_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_next_prev_roundtrip);

    CVXRUN(&t, test_dll_int_iter_value);
    CVXRUN(&t, test_dll_int_iter_value_at_end);
    CVXRUN(&t, test_dll_int_iter_value_wrong_tag);

    CVXRUN(&t, test_dll_int_iter_index);
    CVXRUN(&t, test_dll_int_iter_index_wrong_tag);

    return CVXSUMMARY(&t);
}

#endif /* DLINKED_LIST_ITER_TESTS_H */
