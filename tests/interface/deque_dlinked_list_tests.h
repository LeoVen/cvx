#ifndef DEQUE_DLINKED_LIST_TESTS_H
#define DEQUE_DLINKED_LIST_TESTS_H

#include "cvx/interface_macros.h"
#include "tests/cvxtest.h"
#include "tests/implementations.h"

#define MAKE_DEQUE(name) \
    struct dlinked_int name##_; \
    dll_int_init(&name##_, NULL); \
    struct deque_int name = dll_int_as_deque_int((cvx_container *)&name##_)

/* ---- push_front / count ---- */

static void test_dll_int_dq_push_front_count(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    CVXCHECK(t, cvx_count(&d) == 0);
    cvx_push_front(&d, 1);
    cvx_push_front(&d, 2);
    cvx_push_front(&d, 3);
    CVXCHECK(t, cvx_count(&d) == 3);
    CVXCHECK(t, cvx_flag(&d) == CVX_FLAG_OK);

    cvx_drop(&d);
}

static void test_dll_int_dq_push_back_count(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    CVXCHECK(t, cvx_count(&d) == 0);
    cvx_push_back(&d, 1);
    cvx_push_back(&d, 2);
    cvx_push_back(&d, 3);
    CVXCHECK(t, cvx_count(&d) == 3);
    CVXCHECK(t, cvx_flag(&d) == CVX_FLAG_OK);

    cvx_drop(&d);
}

static void test_dll_int_dq_push_mixed_count(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    cvx_push_front(&d, 10);
    cvx_push_back(&d, 20);
    cvx_push_front(&d, 30);
    cvx_push_back(&d, 40);
    CVXCHECK(t, cvx_count(&d) == 4);
    CVXCHECK(t, cvx_flag(&d) == CVX_FLAG_OK);

    cvx_drop(&d);
}

/* ---- pop_front ---- */

static void test_dll_int_dq_pop_front_order(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    /* push_back then pop_front gives FIFO order */
    cvx_push_back(&d, 10);
    cvx_push_back(&d, 20);
    cvx_push_back(&d, 30);

    CVXCHECK(t, cvx_pop_front(&d) == 10);
    CVXCHECK(t, cvx_pop_front(&d) == 20);
    CVXCHECK(t, cvx_pop_front(&d) == 30);
    CVXCHECK(t, cvx_count(&d) == 0);
    CVXCHECK(t, cvx_flag(&d) == CVX_FLAG_OK);

    cvx_drop(&d);
}

static void test_dll_int_dq_pop_front_stack_order(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    /* push_front then pop_front gives LIFO order */
    cvx_push_front(&d, 10);
    cvx_push_front(&d, 20);
    cvx_push_front(&d, 30);

    CVXCHECK(t, cvx_pop_front(&d) == 30);
    CVXCHECK(t, cvx_pop_front(&d) == 20);
    CVXCHECK(t, cvx_pop_front(&d) == 10);
    CVXCHECK(t, cvx_count(&d) == 0);

    cvx_drop(&d);
}

static void test_dll_int_dq_pop_front_empty(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    cvx_pop_front(&d);
    CVXCHECK(t, cvx_flag(&d) == CVX_FLAG_EMPTY);

    cvx_drop(&d);
}

static void test_dll_int_dq_pop_front_to_empty(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    cvx_push_back(&d, 1);
    cvx_pop_front(&d);

    CVXCHECK(t, cvx_count(&d) == 0);
    CVXCHECK(t, ((struct dlinked_int *)d.instance)->head == NULL);
    CVXCHECK(t, ((struct dlinked_int *)d.instance)->tail == NULL);

    cvx_drop(&d);
}

/* ---- pop_back ---- */

static void test_dll_int_dq_pop_back_order(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    /* push_back then pop_back gives LIFO order */
    cvx_push_back(&d, 10);
    cvx_push_back(&d, 20);
    cvx_push_back(&d, 30);

    CVXCHECK(t, cvx_pop_back(&d) == 30);
    CVXCHECK(t, cvx_pop_back(&d) == 20);
    CVXCHECK(t, cvx_pop_back(&d) == 10);
    CVXCHECK(t, cvx_count(&d) == 0);
    CVXCHECK(t, cvx_flag(&d) == CVX_FLAG_OK);

    cvx_drop(&d);
}

static void test_dll_int_dq_pop_back_empty(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    cvx_pop_back(&d);
    CVXCHECK(t, cvx_flag(&d) == CVX_FLAG_EMPTY);

    cvx_drop(&d);
}

static void test_dll_int_dq_pop_back_to_empty(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    cvx_push_front(&d, 1);
    cvx_pop_back(&d);

    CVXCHECK(t, cvx_count(&d) == 0);
    CVXCHECK(t, ((struct dlinked_int *)d.instance)->head == NULL);
    CVXCHECK(t, ((struct dlinked_int *)d.instance)->tail == NULL);

    cvx_drop(&d);
}

/* ---- mixed push/pop ---- */

static void test_dll_int_dq_mixed_ops(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    /* Build: front -> [30, 10, 20, 40] <- back */
    cvx_push_back(&d, 10);
    cvx_push_back(&d, 20);
    cvx_push_front(&d, 30);
    cvx_push_back(&d, 40);

    CVXCHECK(t, cvx_pop_front(&d) == 30);
    CVXCHECK(t, cvx_pop_back(&d) == 40);
    CVXCHECK(t, cvx_pop_front(&d) == 10);
    CVXCHECK(t, cvx_pop_back(&d) == 20);
    CVXCHECK(t, cvx_count(&d) == 0);

    cvx_drop(&d);
}

/* ---- peek_front ---- */

static void test_dll_int_dq_peek_front(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    cvx_push_front(&d, 10);
    cvx_push_front(&d, 20);

    CVXCHECK(t, cvx_peek_front(&d) == 20);
    CVXCHECK(t, cvx_flag(&d) == CVX_FLAG_OK);
    /* peek must not remove the element */
    CVXCHECK(t, cvx_count(&d) == 2);

    cvx_drop(&d);
}

static void test_dll_int_dq_peek_front_empty(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    cvx_peek_front(&d);
    CVXCHECK(t, cvx_flag(&d) == CVX_FLAG_EMPTY);

    cvx_drop(&d);
}

/* ---- peek_back ---- */

static void test_dll_int_dq_peek_back(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    cvx_push_back(&d, 10);
    cvx_push_back(&d, 20);

    CVXCHECK(t, cvx_peek_back(&d) == 20);
    CVXCHECK(t, cvx_flag(&d) == CVX_FLAG_OK);
    /* peek must not remove the element */
    CVXCHECK(t, cvx_count(&d) == 2);

    cvx_drop(&d);
}

static void test_dll_int_dq_peek_back_empty(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    cvx_peek_back(&d);
    CVXCHECK(t, cvx_flag(&d) == CVX_FLAG_EMPTY);

    cvx_drop(&d);
}

/* ---- clone ---- */

static void test_dll_int_dq_clone(struct cvxtest *t)
{
    MAKE_DEQUE(d);

    cvx_push_back(&d, 10);
    cvx_push_back(&d, 20);
    cvx_push_back(&d, 30);

    struct dlinked_int copy;
    dll_int_clone((struct dlinked_int *)d.instance, &copy);
    CVXCHECK(t, copy.super.flag == CVX_FLAG_OK);

    /* clone is independent: popping from original does not affect copy */
    cvx_pop_front(&d);
    CVXCHECK(t, dll_int_count(&copy) == 3);
    CVXCHECK(t, dll_int_front(&copy) == 10);
    CVXCHECK(t, dll_int_back(&copy) == 30);

    dll_int_drop(&copy);
    cvx_drop(&d);
}

/* ---- runner ---- */

static struct cvxresult run_deque_dlinked_list_tests(void)
{
    struct cvxtest t = { 0 };

    printf("deque (dlinked_list)\n");

    CVXRUN(&t, test_dll_int_dq_push_front_count);
    CVXRUN(&t, test_dll_int_dq_push_back_count);
    CVXRUN(&t, test_dll_int_dq_push_mixed_count);

    CVXRUN(&t, test_dll_int_dq_pop_front_order);
    CVXRUN(&t, test_dll_int_dq_pop_front_stack_order);
    CVXRUN(&t, test_dll_int_dq_pop_front_empty);
    CVXRUN(&t, test_dll_int_dq_pop_front_to_empty);

    CVXRUN(&t, test_dll_int_dq_pop_back_order);
    CVXRUN(&t, test_dll_int_dq_pop_back_empty);
    CVXRUN(&t, test_dll_int_dq_pop_back_to_empty);

    CVXRUN(&t, test_dll_int_dq_mixed_ops);

    CVXRUN(&t, test_dll_int_dq_peek_front);
    CVXRUN(&t, test_dll_int_dq_peek_front_empty);

    CVXRUN(&t, test_dll_int_dq_peek_back);
    CVXRUN(&t, test_dll_int_dq_peek_back_empty);

    CVXRUN(&t, test_dll_int_dq_clone);

    return CVXSUMMARY(&t);
}

#endif /* DEQUE_DLINKED_LIST_TESTS_H */
