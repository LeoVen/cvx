#ifndef DLINKED_LIST_ITER_BIDIRECTIONAL_H
#define DLINKED_LIST_ITER_BIDIRECTIONAL_H

#include "cvx/interface_macros.h"
#include "tests/cvxtest.h"
#include "tests/implementations.h"

/* Helper: allocate a filled collection and return a bidirectional_iter wrapping its heap iterator
 */
#define MAKE_BI(name, col) \
    cvx_container *col = dll_int_new(); \
    dll_int_push_back(col, 10); \
    dll_int_push_back(col, 20); \
    dll_int_push_back(col, 30); \
    struct bidirectional_iter name = dll_int_iter_as_bidirectional_iter(dll_int_iter_start(col))

/* Check the flag stored on the underlying iterator instance */
#define CHECK_ITER_FLAG(t, iter, expected) \
    CVXCHECK(t, ((cvx_container *)((iter).instance))->flag == (expected))

/* ---- Construction ---- */

static void test_bi_start(struct cvxtest *t)
{
    /* Wrap the *collection* so start() creates a fresh iterator */
    cvx_container *col = dll_int_new();
    struct bidirectional_iter col_bi = dll_int_iter_as_bidirectional_iter(col);
    cvx_container *new_iter = cvx_start(&col_bi);

    CVXCHECK(t, new_iter != NULL);
    if (new_iter)
    {
        CVXCHECK(t, dll_int_iter_at_start(new_iter) == true);
        dll_int_iter_drop(new_iter);
    }

    dll_int_drop(col);
}

static void test_bi_end(struct cvxtest *t)
{
    /* Wrap the *collection* so end() creates a past-end iterator */
    cvx_container *col = dll_int_new();
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);

    struct bidirectional_iter col_bi = dll_int_iter_as_bidirectional_iter(col);
    cvx_container *end_iter = cvx_end(&col_bi);

    CVXCHECK(t, end_iter != NULL);
    if (end_iter)
    {
        CVXCHECK(t, dll_int_iter_at_end(end_iter) == true);
        dll_int_iter_drop(end_iter);
    }

    dll_int_drop(col);
}

static void test_bi_drop(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    CVXCHECK(t, col != NULL);
    cvx_drop(&iter);
    dll_int_drop(col);
}

/* ---- State ---- */

static void test_bi_at_start_true(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    CVXCHECK(t, cvx_at_start(&iter) == true);
    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_at_start_false(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    cvx_next(&iter);
    CVXCHECK(t, cvx_at_start(&iter) == false);
    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_at_end_false(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    CVXCHECK(t, cvx_at_end(&iter) == false);
    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_at_end_true(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);

    cvx_container *end_iter = dll_int_iter_end(col);
    struct bidirectional_iter iter = dll_int_iter_as_bidirectional_iter(end_iter);

    CVXCHECK(t, cvx_at_end(&iter) == true);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_at_start_and_end_empty(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    struct bidirectional_iter iter = dll_int_iter_as_bidirectional_iter(dll_int_iter_start(col));

    CVXCHECK(t, cvx_at_start(&iter) == true);
    CVXCHECK(t, cvx_at_end(&iter) == true);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_count(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    CVXCHECK(t, cvx_count(&iter) == 3);
    cvx_drop(&iter);
    dll_int_drop(col);
}

/* ---- Movement ---- */

static void test_bi_to_start(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    cvx_next(&iter);
    cvx_next(&iter);
    cvx_to_start(&iter);

    CVXCHECK(t, cvx_at_start(&iter) == true);
    CVXCHECK(t, cvx_index(&iter) == 0);
    CVXCHECK(t, cvx_value(&iter) == 10);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_to_end(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    cvx_to_end(&iter);

    CVXCHECK(t, cvx_at_end(&iter) == true);
    CVXCHECK(t, cvx_index(&iter) == 3);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_next(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    cvx_next(&iter);
    CVXCHECK(t, cvx_index(&iter) == 1);
    cvx_next(&iter);
    CVXCHECK(t, cvx_index(&iter) == 2);
    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_prev(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    cvx_next(&iter);
    cvx_next(&iter);
    cvx_prev(&iter);

    CVXCHECK(t, cvx_index(&iter) == 1);
    CVXCHECK(t, cvx_value(&iter) == 20);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_prev_from_end(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);

    cvx_container *end_iter = dll_int_iter_end(col);
    struct bidirectional_iter iter = dll_int_iter_as_bidirectional_iter(end_iter);

    cvx_prev(&iter);

    CVXCHECK(t, cvx_index(&iter) == 2);
    CVXCHECK(t, cvx_value(&iter) == 30);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_forward(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    cvx_forward(&iter, 2);

    CVXCHECK(t, cvx_index(&iter) == 2);
    CVXCHECK(t, cvx_value(&iter) == 30);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_forward_clamp(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    cvx_forward(&iter, 100);

    CVXCHECK(t, cvx_at_end(&iter) == true);
    CHECK_ITER_FLAG(t, iter, CVX_FLAG_OK);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_backward(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);

    cvx_container *end_iter = dll_int_iter_end(col);
    struct bidirectional_iter iter = dll_int_iter_as_bidirectional_iter(end_iter);

    cvx_backward(&iter, 2);

    CVXCHECK(t, cvx_index(&iter) == 1);
    CVXCHECK(t, cvx_value(&iter) == 20);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_backward_clamp(struct cvxtest *t)
{
    cvx_container *col = dll_int_new();
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);

    cvx_container *end_iter = dll_int_iter_end(col);
    struct bidirectional_iter iter = dll_int_iter_as_bidirectional_iter(end_iter);

    cvx_backward(&iter, 100);

    CVXCHECK(t, cvx_at_start(&iter) == true);
    CHECK_ITER_FLAG(t, iter, CVX_FLAG_OK);

    cvx_drop(&iter);
    dll_int_drop(col);
}

/* ---- Access ---- */

static void test_bi_value(struct cvxtest *t)
{
    MAKE_BI(iter, col);

    CVXCHECK(t, cvx_value(&iter) == 10);
    cvx_next(&iter);
    CVXCHECK(t, cvx_value(&iter) == 20);
    cvx_next(&iter);
    CVXCHECK(t, cvx_value(&iter) == 30);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_index(struct cvxtest *t)
{
    MAKE_BI(iter, col);

    CVXCHECK(t, cvx_index(&iter) == 0);
    cvx_next(&iter);
    CVXCHECK(t, cvx_index(&iter) == 1);

    cvx_drop(&iter);
    dll_int_drop(col);
}

/* ---- Error paths through the interface ---- */

static void test_bi_next_at_end(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    cvx_to_end(&iter);
    cvx_next(&iter);

    CHECK_ITER_FLAG(t, iter, CVX_FLAG_RANGE);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_prev_at_start(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    cvx_prev(&iter);

    CHECK_ITER_FLAG(t, iter, CVX_FLAG_RANGE);

    cvx_drop(&iter);
    dll_int_drop(col);
}

static void test_bi_value_at_end(struct cvxtest *t)
{
    MAKE_BI(iter, col);
    cvx_to_end(&iter);
    int val = cvx_value(&iter);

    CVXCHECK(t, val == 0);
    CHECK_ITER_FLAG(t, iter, CVX_FLAG_RANGE);

    cvx_drop(&iter);
    dll_int_drop(col);
}

/* ---- runner ---- */

static int run_dlinked_list_iter_bidirectional_tests(void)
{
    struct cvxtest t = { 0 };
    printf("dlinked_list (bidirectional_iterator)\n");

    CVXRUN(&t, test_bi_start);
    CVXRUN(&t, test_bi_end);
    CVXRUN(&t, test_bi_drop);

    CVXRUN(&t, test_bi_at_start_true);
    CVXRUN(&t, test_bi_at_start_false);
    CVXRUN(&t, test_bi_at_end_false);
    CVXRUN(&t, test_bi_at_end_true);
    CVXRUN(&t, test_bi_at_start_and_end_empty);
    CVXRUN(&t, test_bi_count);

    CVXRUN(&t, test_bi_to_start);
    CVXRUN(&t, test_bi_to_end);
    CVXRUN(&t, test_bi_next);
    CVXRUN(&t, test_bi_prev);
    CVXRUN(&t, test_bi_prev_from_end);
    CVXRUN(&t, test_bi_forward);
    CVXRUN(&t, test_bi_forward_clamp);
    CVXRUN(&t, test_bi_backward);
    CVXRUN(&t, test_bi_backward_clamp);

    CVXRUN(&t, test_bi_value);
    CVXRUN(&t, test_bi_index);

    CVXRUN(&t, test_bi_next_at_end);
    CVXRUN(&t, test_bi_prev_at_start);
    CVXRUN(&t, test_bi_value_at_end);

    return CVXSUMMARY(&t);
}

#endif /* DLINKED_LIST_ITER_BIDIRECTIONAL_H */
