#ifndef ISTACK_SLINKED_TESTS_H
#define ISTACK_SLINKED_TESTS_H

#include "cvx/interface_macros.h"
#include "cvxtest.h"

#include "interfaces.h"

#define V int
#define SNAME sll_ist_int
#define PFX sll_ist
#define TAG 88
#define IMPL_STACK stack
#include "cvx/slinked_list.h"

#undef MAKE_STACK
#define MAKE_STACK(name) struct stack name = sll_ist_as_stack(sll_ist_new())

/* ---- push / count ---- */

static void test_sll_ist_push_count(struct cvxtest *t)
{
    MAKE_STACK(s);

    CVXCHECK(t, cvx_count(&s) == 0);
    cvx_push(&s, 1);
    cvx_push(&s, 2);
    cvx_push(&s, 3);
    CVXCHECK(t, cvx_count(&s) == 3);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_OK);

    cvx_drop(&s);
}

static void test_sll_ist_push_many(struct cvxtest *t)
{
    MAKE_STACK(s);

    for (int i = 0; i < 100; i++)
        cvx_push(&s, i);

    CVXCHECK(t, cvx_count(&s) == 100);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_OK);

    cvx_drop(&s);
}

/* ---- pop ---- */

static void test_sll_ist_pop_lifo_order(struct cvxtest *t)
{
    MAKE_STACK(s);

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

static void test_sll_ist_pop_null_out(struct cvxtest *t)
{
    MAKE_STACK(s);

    cvx_push(&s, 99);
    cvx_pop(&s, NULL);

    CVXCHECK(t, cvx_count(&s) == 0);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_OK);

    cvx_drop(&s);
}

static void test_sll_ist_pop_empty(struct cvxtest *t)
{
    MAKE_STACK(s);

    cvx_pop(&s, NULL);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_EMPTY);

    cvx_drop(&s);
}

static void test_sll_ist_pop_to_empty(struct cvxtest *t)
{
    MAKE_STACK(s);

    cvx_push(&s, 1);
    cvx_pop(&s, NULL);

    CVXCHECK(t, cvx_count(&s) == 0);
    /* underlying list must have cleaned up both head and tail */
    CVXCHECK(t, ((struct sll_ist_int *)s.instance)->head == NULL);
    CVXCHECK(t, ((struct sll_ist_int *)s.instance)->tail == NULL);

    cvx_drop(&s);
}

/* ---- peek ---- */

static void test_sll_ist_peek(struct cvxtest *t)
{
    MAKE_STACK(s);

    cvx_push(&s, 7);
    cvx_push(&s, 42);

    CVXCHECK(t, cvx_peek(&s) == 42);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_OK);
    /* peek must not remove the element */
    CVXCHECK(t, cvx_count(&s) == 2);

    cvx_drop(&s);
}

static void test_sll_ist_peek_empty(struct cvxtest *t)
{
    MAKE_STACK(s);

    cvx_peek(&s);
    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_EMPTY);

    cvx_drop(&s);
}

/* ---- replace ---- */

static void test_sll_ist_replace(struct cvxtest *t)
{
    MAKE_STACK(s);

    cvx_push(&s, 5);
    cvx_push(&s, 10);

    int old = 0;
    cvx_replace(&s, 99, &old);

    CVXCHECK(t, old == 10);
    CVXCHECK(t, cvx_peek(&s) == 99);
    CVXCHECK(t, cvx_count(&s) == 2);

    cvx_drop(&s);
}

static void test_sll_ist_replace_on_empty_with_null_out(struct cvxtest *t)
{
    MAKE_STACK(s);

    cvx_replace(&s, 42, NULL);

    CVXCHECK(t, cvx_count(&s) == 1);
    CVXCHECK(t, cvx_peek(&s) == 42);

    cvx_drop(&s);
}

static void test_sll_ist_replace_on_empty_with_out(struct cvxtest *t)
{
    MAKE_STACK(s);

    int out = 0;
    cvx_replace(&s, 42, &out);

    CVXCHECK(t, cvx_flag(&s) == CVX_FLAG_EMPTY);
    CVXCHECK(t, cvx_count(&s) == 0);

    cvx_drop(&s);
}

/* ---- runner ---- */

static int run_istack_slinked_list_tests(void)
{
    struct cvxtest t = { 0 };

    printf("istack (slinked_list)\n");

    CVXRUN(&t, test_sll_ist_push_count);
    CVXRUN(&t, test_sll_ist_push_many);

    CVXRUN(&t, test_sll_ist_pop_lifo_order);
    CVXRUN(&t, test_sll_ist_pop_null_out);
    CVXRUN(&t, test_sll_ist_pop_empty);
    CVXRUN(&t, test_sll_ist_pop_to_empty);

    CVXRUN(&t, test_sll_ist_peek);
    CVXRUN(&t, test_sll_ist_peek_empty);

    CVXRUN(&t, test_sll_ist_replace);
    CVXRUN(&t, test_sll_ist_replace_on_empty_with_null_out);
    CVXRUN(&t, test_sll_ist_replace_on_empty_with_out);

    return CVXSUMMARY(&t);
}

#endif /* ISTACK_SLINKED_TESTS_H */
