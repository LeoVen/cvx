#ifndef ISTACK_TESTS_H
#define ISTACK_TESTS_H

#include "cvx/interface_macros.h"
#include "cvxtest.h"

#include "interfaces.h"

#define V int
#define SNAME da_ist_int
#define PFX da_ist
#define TAG 55
#define IMPL_STACK stack
#include "cvx/dynamic_array.h"

#undef MAKE_STACK
#define MAKE_STACK(name, capacity) \
    struct stack name = da_ist_as_stack(da_ist_new_with(capacity))

/* ---- push / count ---- */

static void test_ist_push_count(struct cvxtest *t)
{
    MAKE_STACK(s, 4);

    CVXCHECK(t, cvx_count(&s) == 0);
    cvx_push(&s, 1);
    cvx_push(&s, 2);
    cvx_push(&s, 3);
    CVXCHECK(t, cvx_count(&s) == 3);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_OK);

    cvx_drop(&s);
}

static void test_ist_push_grows(struct cvxtest *t)
{
    MAKE_STACK(s, 4);

    for (int i = 0; i < 20; i++)
        cvx_push(&s, i);

    CVXCHECK(t, cvx_count(&s) == 20);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_OK);

    cvx_drop(&s);
}

/* ---- pop ---- */

static void test_ist_pop_lifo_order(struct cvxtest *t)
{
    MAKE_STACK(s, 4);

    cvx_push(&s, 10);
    cvx_push(&s, 20);
    cvx_push(&s, 30);

    int out = 0;
    cvx_pop(&s, &out);
    CVXCHECK(t, out == 30);
    cvx_pop(&s, &out);
    CVXCHECK(t, out == 20);
    cvx_pop(&s, &out);
    CVXCHECK(t, out == 10);
    CVXCHECK(t, cvx_count(&s) == 0);

    cvx_drop(&s);
}

static void test_ist_pop_null_out(struct cvxtest *t)
{
    MAKE_STACK(s, 4);

    cvx_push(&s, 99);
    cvx_pop(&s, NULL);

    CVXCHECK(t, cvx_count(&s) == 0);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_OK);

    cvx_drop(&s);
}

static void test_ist_pop_empty(struct cvxtest *t)
{
    MAKE_STACK(s, 4);

    cvx_pop(&s, NULL);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_EMPTY);

    cvx_drop(&s);
}

/* ---- peek ---- */

static void test_ist_peek(struct cvxtest *t)
{
    MAKE_STACK(s, 4);

    cvx_push(&s, 7);
    cvx_push(&s, 42);

    CVXCHECK(t, cvx_peek(&s) == 42);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_OK);
    /* peek must not remove the element */
    CVXCHECK(t, cvx_count(&s) == 2);

    cvx_drop(&s);
}

static void test_ist_peek_empty(struct cvxtest *t)
{
    MAKE_STACK(s, 4);

    cvx_peek(&s);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_EMPTY);

    cvx_drop(&s);
}

/* ---- replace ---- */

static void test_ist_replace(struct cvxtest *t)
{
    MAKE_STACK(s, 4);

    cvx_push(&s, 5);
    cvx_push(&s, 10);

    int old = 0;
    cvx_replace(&s, 99, &old);

    CVXCHECK(t, old == 10);
    CVXCHECK(t, cvx_peek(&s) == 99);
    CVXCHECK(t, cvx_count(&s) == 2);

    cvx_drop(&s);
}

static void test_ist_replace_on_empty_with_null_out(struct cvxtest *t)
{
    MAKE_STACK(s, 4);

    cvx_replace(&s, 42, NULL);

    CVXCHECK(t, cvx_count(&s) == 1);
    CVXCHECK(t, cvx_peek(&s) == 42);

    cvx_drop(&s);
}

static void test_ist_replace_on_empty_with_out(struct cvxtest *t)
{
    MAKE_STACK(s, 4);

    int out = 0;
    cvx_replace(&s, 42, &out);

    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_EMPTY);
    CVXCHECK(t, cvx_count(&s) == 0);

    cvx_drop(&s);
}

/* ---- runner ---- */

static int run_istack_dynamic_array_tests(void)
{
    struct cvxtest t = { 0 };

    printf("istack\n");

    CVXRUN(&t, test_ist_push_count);
    CVXRUN(&t, test_ist_push_grows);

    CVXRUN(&t, test_ist_pop_lifo_order);
    CVXRUN(&t, test_ist_pop_null_out);
    CVXRUN(&t, test_ist_pop_empty);

    CVXRUN(&t, test_ist_peek);
    CVXRUN(&t, test_ist_peek_empty);

    CVXRUN(&t, test_ist_replace);
    CVXRUN(&t, test_ist_replace_on_empty_with_null_out);
    CVXRUN(&t, test_ist_replace_on_empty_with_out);

    return CVXSUMMARY(&t);
}

#endif /* ISTACK_TESTS_H */
