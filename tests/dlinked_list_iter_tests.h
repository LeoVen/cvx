#ifndef DLINKED_LIST_ITER_TESTS_H
#define DLINKED_LIST_ITER_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

/* Helper: fill list col with values 10, 20, 30 */
static void dll_int_fill3(struct dlinked_int *col)
{
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);
}

/* ---- iter_init_start ---- */

static void test_dll_int_iter_init_start(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter it = dll_int_iter_init_start(col);

    CVXCHECK(t, it.super.tag == (size_t)DLL_ITER_TAG);
    CVXCHECK(t, it.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 0);
    CVXCHECK(t, it.cursor == col->head);

    dll_int_drop(col);
}

/* ---- iter_init_end ---- */

static void test_dll_int_iter_init_end(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter it = dll_int_iter_init_end(col);

    CVXCHECK(t, it.super.tag == (size_t)DLL_ITER_TAG);
    CVXCHECK(t, it.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 3);
    CVXCHECK(t, it.cursor == NULL);

    dll_int_drop(col);
}

/* ---- iter_start ---- */

static void test_dll_int_iter_start(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);

    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        dll_int_drop(col);
        return;
    }
    CVXCHECK(t, iter->super.tag == (size_t)DLL_ITER_TAG);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter->index == 0);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_end ---- */

static void test_dll_int_iter_end(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_end(col);

    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        dll_int_drop(col);
        return;
    }
    CVXCHECK(t, iter->super.tag == (size_t)DLL_ITER_TAG);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter->index == 3);
    CVXCHECK(t, iter->cursor == NULL);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_drop ---- */

static void test_dll_int_iter_drop(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        dll_int_drop(col);
        return;
    }

    dll_int_iter_drop(iter); /* must not crash */

    dll_int_drop(col);
}

/* ---- iter_at_start ---- */

static void test_dll_int_iter_at_start_true(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_at_start(iter) == true);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_at_start_false(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_at_start(iter) == false);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_at_end ---- */

static void test_dll_int_iter_at_end_true(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_end(col);
    CVXCHECK(t, dll_int_iter_at_end(iter) == true);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_at_end_false(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_at_end(iter) == false);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_empty_at_start_and_end(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_at_start(iter) == true);
    CVXCHECK(t, dll_int_iter_at_end(iter) == true);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_count ---- */

static void test_dll_int_iter_count(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    for (int i = 0; i < 5; i++)
        dll_int_push_back(col, i);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_count(iter) == 5);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_to_start ---- */

static void test_dll_int_iter_to_start(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    dll_int_iter_next(iter);
    dll_int_iter_next(iter);
    dll_int_iter_to_start(iter);

    CVXCHECK(t, dll_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_value(iter) == 10);
    CVXCHECK(t, iter->cursor == col->head);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_to_end ---- */

static void test_dll_int_iter_to_end(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    dll_int_iter_to_end(iter);

    CVXCHECK(t, dll_int_iter_at_end(iter) == true);
    CVXCHECK(t, dll_int_iter_index(iter) == 3);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter->cursor == NULL);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_next ---- */

static void test_dll_int_iter_next(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_index(iter) == 0);

    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);

    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 2);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_next_at_end(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_end(col);
    dll_int_iter_next(iter);

    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_prev ---- */

static void test_dll_int_iter_prev(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_end(col);
    CVXCHECK(t, dll_int_iter_index(iter) == 3);

    dll_int_iter_prev(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 2);
    CVXCHECK(t, dll_int_iter_value(iter) == 30);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);

    dll_int_iter_prev(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    CVXCHECK(t, dll_int_iter_value(iter) == 20);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_prev_at_start(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    dll_int_iter_prev(iter);

    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_forward ---- */

static void test_dll_int_iter_forward(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    dll_int_iter_forward(iter, 2);

    CVXCHECK(t, dll_int_iter_index(iter) == 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_value(iter) == 30);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_forward_clamp(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    dll_int_iter_forward(iter, 100);

    CVXCHECK(t, dll_int_iter_index(iter) == 3);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_at_end(iter) == true);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_backward ---- */

static void test_dll_int_iter_backward(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_end(col);
    dll_int_iter_backward(iter, 2);

    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_value(iter) == 20);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

static void test_dll_int_iter_backward_clamp(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_end(col);
    dll_int_iter_backward(iter, 100);

    CVXCHECK(t, dll_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_at_start(iter) == true);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- round-trip: next then prev ---- */

static void test_dll_int_iter_next_prev_roundtrip(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
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
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
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
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_end(col);
    int val = dll_int_iter_value(iter);

    CVXCHECK(t, val == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- iter_index ---- */

static void test_dll_int_iter_index(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_fill3(col);

    struct dlinked_int_iter *iter = dll_int_iter_start(col);
    CVXCHECK(t, dll_int_iter_index(iter) == 0);

    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 1);

    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 2);

    dll_int_iter_drop(iter);
    dll_int_drop(col);
}

/* ---- runner ---- */

static struct cvxresult run_dlinked_list_iter_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dlinked_list (iterator)\n");

    CVXRUN(&t, test_dll_int_iter_init_start);

    CVXRUN(&t, test_dll_int_iter_init_end);

    CVXRUN(&t, test_dll_int_iter_start);

    CVXRUN(&t, test_dll_int_iter_end);

    CVXRUN(&t, test_dll_int_iter_drop);

    CVXRUN(&t, test_dll_int_iter_at_start_true);
    CVXRUN(&t, test_dll_int_iter_at_start_false);

    CVXRUN(&t, test_dll_int_iter_at_end_true);
    CVXRUN(&t, test_dll_int_iter_at_end_false);
    CVXRUN(&t, test_dll_int_iter_empty_at_start_and_end);

    CVXRUN(&t, test_dll_int_iter_count);

    CVXRUN(&t, test_dll_int_iter_to_start);

    CVXRUN(&t, test_dll_int_iter_to_end);

    CVXRUN(&t, test_dll_int_iter_next);
    CVXRUN(&t, test_dll_int_iter_next_at_end);

    CVXRUN(&t, test_dll_int_iter_prev);
    CVXRUN(&t, test_dll_int_iter_prev_at_start);

    CVXRUN(&t, test_dll_int_iter_next_prev_roundtrip);

    CVXRUN(&t, test_dll_int_iter_forward);
    CVXRUN(&t, test_dll_int_iter_forward_clamp);

    CVXRUN(&t, test_dll_int_iter_backward);
    CVXRUN(&t, test_dll_int_iter_backward_clamp);

    CVXRUN(&t, test_dll_int_iter_value);
    CVXRUN(&t, test_dll_int_iter_value_at_end);

    CVXRUN(&t, test_dll_int_iter_index);

    return CVXSUMMARY(&t);
}

#endif /* DLINKED_LIST_ITER_TESTS_H */
