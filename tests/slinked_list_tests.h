#ifndef SLINKED_LIST_TESTS_H
#define SLINKED_LIST_TESTS_H

#include "cvxtest.h"

#include "implementations.h"

/* ---- init / new ---- */

static void test_sll_int_init(struct cvxtest *t)
{
    struct slinked_int l = sll_int_init(NULL);
    cvx_container *col = (cvx_container *)(&l);

    CVXCHECK(t, col->tag == 77);
    CVXCHECK(t, l.head == NULL);
    CVXCHECK(t, l.tail == NULL);
    CVXCHECK(t, l.count == 0);
    CVXCHECK(t, l.vtabv == NULL);
}

static void test_sll_int_new(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->tag == 77);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, sll_int_count(col) == 0);

    sll_int_drop(col);
}

/* ---- push_back / count ---- */

static void test_sll_int_push_back(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 10);
    sll_int_push_back(col, 20);
    sll_int_push_back(col, 30);

    CVXCHECK(t, sll_int_count(col) == 3);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, sll_int_front(col) == 10);
    CVXCHECK(t, sll_int_back(col) == 30);

    sll_int_drop(col);
}

static void test_sll_int_push_back_many(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    for (int i = 0; i < 100; i++)
        sll_int_push_back(col, i);

    CVXCHECK(t, sll_int_count(col) == 100);
    CVXCHECK(t, sll_int_front(col) == 0);
    CVXCHECK(t, sll_int_back(col) == 99);

    sll_int_drop(col);
}

/* ---- push_front ---- */

static void test_sll_int_push_front(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 2);
    sll_int_push_back(col, 3);
    sll_int_push_front(col, 1);

    CVXCHECK(t, sll_int_count(col) == 3);
    CVXCHECK(t, sll_int_front(col) == 1);
    CVXCHECK(t, sll_int_back(col) == 3);
    CVXCHECK(t, sll_int_get(col, 1) == 2);

    sll_int_drop(col);
}

/* ---- push_at ---- */

static void test_sll_int_push_at_middle(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 1);
    sll_int_push_back(col, 3);
    sll_int_push_at(col, 2, 1);

    CVXCHECK(t, sll_int_count(col) == 3);
    CVXCHECK(t, sll_int_get(col, 0) == 1);
    CVXCHECK(t, sll_int_get(col, 1) == 2);
    CVXCHECK(t, sll_int_get(col, 2) == 3);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    sll_int_drop(col);
}

static void test_sll_int_push_at_head(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 2);
    sll_int_push_at(col, 1, 0);

    CVXCHECK(t, sll_int_front(col) == 1);
    CVXCHECK(t, sll_int_back(col) == 2);

    sll_int_drop(col);
}

static void test_sll_int_push_at_tail(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 1);
    sll_int_push_at(col, 2, 1);

    CVXCHECK(t, sll_int_back(col) == 2);

    sll_int_drop(col);
}

static void test_sll_int_push_at_out_of_range(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 1);
    sll_int_push_at(col, 99, 5);

    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    sll_int_drop(col);
}

/* ---- pop_front ---- */

static void test_sll_int_pop_front(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 10);
    sll_int_push_back(col, 20);

    int out = sll_int_pop_front(col);

    CVXCHECK(t, out == 10);
    CVXCHECK(t, sll_int_count(col) == 1);
    CVXCHECK(t, sll_int_front(col) == 20);

    sll_int_drop(col);
}

static void test_sll_int_pop_front_to_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 1);
    sll_int_pop_front(col);

    CVXCHECK(t, sll_int_count(col) == 0);
    /* Both head and tail must be NULL after the last element is removed */
    CVXCHECK(t, ((struct slinked_int *)col)->head == NULL);
    CVXCHECK(t, ((struct slinked_int *)col)->tail == NULL);

    sll_int_drop(col);
}

static void test_sll_int_pop_front_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_pop_front(col);
    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    sll_int_drop(col);
}

/* ---- pop_back ---- */

static void test_sll_int_pop_back(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 10);
    sll_int_push_back(col, 20);

    int out = sll_int_pop_back(col);

    CVXCHECK(t, out == 20);
    CVXCHECK(t, sll_int_count(col) == 1);
    CVXCHECK(t, sll_int_back(col) == 10);

    sll_int_drop(col);
}

static void test_sll_int_pop_back_to_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 1);
    sll_int_pop_back(col);

    CVXCHECK(t, sll_int_count(col) == 0);
    CVXCHECK(t, ((struct slinked_int *)col)->head == NULL);
    CVXCHECK(t, ((struct slinked_int *)col)->tail == NULL);

    sll_int_drop(col);
}

static void test_sll_int_pop_back_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_pop_back(col);
    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    sll_int_drop(col);
}

/* ---- pop_at ---- */

static void test_sll_int_pop_at_middle(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 1);
    sll_int_push_back(col, 2);
    sll_int_push_back(col, 3);

    int out = sll_int_pop_at(col, 1);

    CVXCHECK(t, out == 2);
    CVXCHECK(t, sll_int_count(col) == 2);
    CVXCHECK(t, sll_int_get(col, 0) == 1);
    CVXCHECK(t, sll_int_get(col, 1) == 3);

    sll_int_drop(col);
}

static void test_sll_int_pop_at_out_of_range(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 1);
    sll_int_pop_at(col, 5);

    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    sll_int_drop(col);
}

static void test_sll_int_pop_at_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_pop_at(col, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    sll_int_drop(col);
}

/* ---- front / back ---- */

static void test_sll_int_front_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_front(col);
    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    sll_int_drop(col);
}

static void test_sll_int_back_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_back(col);
    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    sll_int_drop(col);
}

/* ---- get ---- */

static void test_sll_int_get(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 10);
    sll_int_push_back(col, 20);
    sll_int_push_back(col, 30);

    CVXCHECK(t, sll_int_get(col, 0) == 10);
    CVXCHECK(t, sll_int_get(col, 1) == 20);
    CVXCHECK(t, sll_int_get(col, 2) == 30);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    sll_int_drop(col);
}

static void test_sll_int_get_out_of_range(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 1);
    sll_int_get(col, 1);

    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    sll_int_drop(col);
}

static void test_sll_int_get_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_get(col, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    sll_int_drop(col);
}

/* ---- empty ---- */

static void test_sll_int_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    CVXCHECK(t, sll_int_empty(col) == true);

    sll_int_push_back(col, 1);
    CVXCHECK(t, sll_int_empty(col) == false);

    sll_int_pop_front(col);
    CVXCHECK(t, sll_int_empty(col) == true);

    sll_int_drop(col);
}

/* ---- replace_back ---- */

static void test_sll_int_replace_back(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 5);
    sll_int_push_back(col, 10);

    int old = sll_int_replace_back(col, 99);

    CVXCHECK(t, old == 10);
    CVXCHECK(t, sll_int_back(col) == 99);
    CVXCHECK(t, sll_int_count(col) == 2);

    sll_int_drop(col);
}

static void test_sll_int_replace_back_on_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_replace_back(col, 42);

    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, sll_int_count(col) == 0);

    sll_int_drop(col);
}

/* ---- replace_front ---- */

static void test_sll_int_replace_front(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 10);
    sll_int_push_back(col, 20);

    int old = sll_int_replace_front(col, 99);

    CVXCHECK(t, old == 10);
    CVXCHECK(t, sll_int_front(col) == 99);
    CVXCHECK(t, sll_int_count(col) == 2);

    sll_int_drop(col);
}

static void test_sll_int_replace_front_on_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_replace_front(col, 42);

    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, sll_int_count(col) == 0);

    sll_int_drop(col);
}

/* ---- clear ---- */

static void test_sll_int_clear(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();

    sll_int_push_back(col, 1);
    sll_int_push_back(col, 2);
    sll_int_push_back(col, 3);
    sll_int_clear(col);

    CVXCHECK(t, sll_int_count(col) == 0);
    CVXCHECK(t, ((struct slinked_int *)col)->head == NULL);
    CVXCHECK(t, ((struct slinked_int *)col)->tail == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    sll_int_drop(col);
}

/* ---- clone (int / direct-assignment branch) ---- */

static void test_sll_int_clone_empty(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    cvx_container *clone = sll_int_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        sll_int_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CVXCHECK(t, sll_int_count(clone) == 0);

    sll_int_drop(col);
    sll_int_drop(clone);
}

static void test_sll_int_clone_values(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();
    sll_int_push_back(col, 10);
    sll_int_push_back(col, 20);
    sll_int_push_back(col, 30);

    cvx_container *clone = sll_int_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        sll_int_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CVXCHECK(t, sll_int_count(clone) == 3);
    CVXCHECK(t, sll_int_get(clone, 0) == 10);
    CVXCHECK(t, sll_int_get(clone, 1) == 20);
    CVXCHECK(t, sll_int_get(clone, 2) == 30);

    sll_int_drop(col);
    sll_int_drop(clone);
}

/* ---- wrong tag guard ---- */

static void test_sll_int_wrong_tag(struct cvxtest *t)
{
    cvx_container *col = sll_int_new();
    col->tag = 0;
    sll_int_push_back(col, 1);

    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);

    col->tag = 77;
    sll_int_drop(col);
}

/* ---- runner ---- */

static int run_slinked_list_tests(void)
{
    struct cvxtest t = { 0 };

    printf("slinked_list\n");

    CVXRUN(&t, test_sll_int_init);
    CVXRUN(&t, test_sll_int_new);

    CVXRUN(&t, test_sll_int_push_back);
    CVXRUN(&t, test_sll_int_push_back_many);
    CVXRUN(&t, test_sll_int_push_front);
    CVXRUN(&t, test_sll_int_push_at_middle);
    CVXRUN(&t, test_sll_int_push_at_head);
    CVXRUN(&t, test_sll_int_push_at_tail);
    CVXRUN(&t, test_sll_int_push_at_out_of_range);

    CVXRUN(&t, test_sll_int_pop_front);
    CVXRUN(&t, test_sll_int_pop_front_to_empty);
    CVXRUN(&t, test_sll_int_pop_front_empty);
    CVXRUN(&t, test_sll_int_pop_back);
    CVXRUN(&t, test_sll_int_pop_back_to_empty);
    CVXRUN(&t, test_sll_int_pop_back_empty);
    CVXRUN(&t, test_sll_int_pop_at_middle);
    CVXRUN(&t, test_sll_int_pop_at_out_of_range);
    CVXRUN(&t, test_sll_int_pop_at_empty);

    CVXRUN(&t, test_sll_int_front_empty);
    CVXRUN(&t, test_sll_int_back_empty);

    CVXRUN(&t, test_sll_int_get);
    CVXRUN(&t, test_sll_int_get_out_of_range);
    CVXRUN(&t, test_sll_int_get_empty);

    CVXRUN(&t, test_sll_int_empty);

    CVXRUN(&t, test_sll_int_replace_back);
    CVXRUN(&t, test_sll_int_replace_back_on_empty);
    CVXRUN(&t, test_sll_int_replace_front);
    CVXRUN(&t, test_sll_int_replace_front_on_empty);

    CVXRUN(&t, test_sll_int_clear);
    CVXRUN(&t, test_sll_int_wrong_tag);

    CVXRUN(&t, test_sll_int_clone_empty);
    CVXRUN(&t, test_sll_int_clone_values);

    return CVXSUMMARY(&t);
}

#endif /* SLINKED_LIST_TESTS_H */
