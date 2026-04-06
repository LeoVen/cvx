#ifndef SLINKED_LIST_TESTS_H
#define SLINKED_LIST_TESTS_H

#include "cvxtest.h"

#define V int
#define SNAME sll_int
#define PFX sll
#define TAG 77
#include "cvx/slinked_list.h"

/* ---- helpers ---- */

#undef CHECK_COUNT
#define CHECK_COUNT(t, col, n) CVXCHECK(t, sll_count(col) == (size_t)(n))

/* ---- init / new ---- */

static void test_sll_init(struct cvxtest *t)
{
    struct sll_int l = sll_init();
    cvx_container *col = (cvx_container *)(&l);

    CVXCHECK(t, col->tag == 77);
    CVXCHECK(t, l.head == NULL);
    CVXCHECK(t, l.tail == NULL);
    CVXCHECK(t, l.count == 0);
}

static void test_sll_new(struct cvxtest *t)
{
    cvx_container *col = sll_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->tag == 77);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CHECK_COUNT(t, col, 0);

    sll_drop(col);
}

/* ---- push_back / count ---- */

static void test_sll_push_back(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 10);
    sll_push_back(col, 20);
    sll_push_back(col, 30);

    CHECK_COUNT(t, col, 3);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, sll_front(col) == 10);
    CVXCHECK(t, sll_back(col) == 30);

    sll_drop(col);
}

static void test_sll_push_back_many(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    for (int i = 0; i < 100; i++)
        sll_push_back(col, i);

    CHECK_COUNT(t, col, 100);
    CVXCHECK(t, sll_front(col) == 0);
    CVXCHECK(t, sll_back(col) == 99);

    sll_drop(col);
}

/* ---- push_front ---- */

static void test_sll_push_front(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 2);
    sll_push_back(col, 3);
    sll_push_front(col, 1);

    CHECK_COUNT(t, col, 3);
    CVXCHECK(t, sll_front(col) == 1);
    CVXCHECK(t, sll_back(col) == 3);
    CVXCHECK(t, sll_get(col, 1) == 2);

    sll_drop(col);
}

/* ---- push_at ---- */

static void test_sll_push_at_middle(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 1);
    sll_push_back(col, 3);
    sll_push_at(col, 2, 1);

    CHECK_COUNT(t, col, 3);
    CVXCHECK(t, sll_get(col, 0) == 1);
    CVXCHECK(t, sll_get(col, 1) == 2);
    CVXCHECK(t, sll_get(col, 2) == 3);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    sll_drop(col);
}

static void test_sll_push_at_head(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 2);
    sll_push_at(col, 1, 0);

    CVXCHECK(t, sll_front(col) == 1);
    CVXCHECK(t, sll_back(col) == 2);

    sll_drop(col);
}

static void test_sll_push_at_tail(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 1);
    sll_push_at(col, 2, 1);

    CVXCHECK(t, sll_back(col) == 2);

    sll_drop(col);
}

static void test_sll_push_at_out_of_range(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 1);
    sll_push_at(col, 99, 5);

    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    sll_drop(col);
}

/* ---- pop_front ---- */

static void test_sll_pop_front(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 10);
    sll_push_back(col, 20);

    int out = sll_pop_front(col);

    CVXCHECK(t, out == 10);
    CHECK_COUNT(t, col, 1);
    CVXCHECK(t, sll_front(col) == 20);

    sll_drop(col);
}

static void test_sll_pop_front_to_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 1);
    sll_pop_front(col);

    CHECK_COUNT(t, col, 0);
    /* Both head and tail must be NULL after the last element is removed */
    CVXCHECK(t, ((struct sll_int *)col)->head == NULL);
    CVXCHECK(t, ((struct sll_int *)col)->tail == NULL);

    sll_drop(col);
}

static void test_sll_pop_front_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_pop_front(col);
    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    sll_drop(col);
}

/* ---- pop_back ---- */

static void test_sll_pop_back(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 10);
    sll_push_back(col, 20);

    int out = sll_pop_back(col);

    CVXCHECK(t, out == 20);
    CHECK_COUNT(t, col, 1);
    CVXCHECK(t, sll_back(col) == 10);

    sll_drop(col);
}

static void test_sll_pop_back_to_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 1);
    sll_pop_back(col);

    CHECK_COUNT(t, col, 0);
    CVXCHECK(t, ((struct sll_int *)col)->head == NULL);
    CVXCHECK(t, ((struct sll_int *)col)->tail == NULL);

    sll_drop(col);
}

static void test_sll_pop_back_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_pop_back(col);
    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    sll_drop(col);
}

/* ---- pop_at ---- */

static void test_sll_pop_at_middle(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 1);
    sll_push_back(col, 2);
    sll_push_back(col, 3);

    int out = sll_pop_at(col, 1);

    CVXCHECK(t, out == 2);
    CHECK_COUNT(t, col, 2);
    CVXCHECK(t, sll_get(col, 0) == 1);
    CVXCHECK(t, sll_get(col, 1) == 3);

    sll_drop(col);
}

static void test_sll_pop_at_out_of_range(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 1);
    sll_pop_at(col, 5);

    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    sll_drop(col);
}

static void test_sll_pop_at_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_pop_at(col, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    sll_drop(col);
}

/* ---- front / back ---- */

static void test_sll_front_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_front(col);
    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    sll_drop(col);
}

static void test_sll_back_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_back(col);
    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    sll_drop(col);
}

/* ---- get ---- */

static void test_sll_get(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 10);
    sll_push_back(col, 20);
    sll_push_back(col, 30);

    CVXCHECK(t, sll_get(col, 0) == 10);
    CVXCHECK(t, sll_get(col, 1) == 20);
    CVXCHECK(t, sll_get(col, 2) == 30);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    sll_drop(col);
}

static void test_sll_get_out_of_range(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 1);
    sll_get(col, 1);

    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    sll_drop(col);
}

static void test_sll_get_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_get(col, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    sll_drop(col);
}

/* ---- empty ---- */

static void test_sll_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    CVXCHECK(t, sll_empty(col) == true);

    sll_push_back(col, 1);
    CVXCHECK(t, sll_empty(col) == false);

    sll_pop_front(col);
    CVXCHECK(t, sll_empty(col) == true);

    sll_drop(col);
}

/* ---- replace_back ---- */

static void test_sll_replace_back(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 5);
    sll_push_back(col, 10);

    int old = sll_replace_back(col, 99);

    CVXCHECK(t, old == 10);
    CVXCHECK(t, sll_back(col) == 99);
    CHECK_COUNT(t, col, 2);

    sll_drop(col);
}

static void test_sll_replace_back_on_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_replace_back(col, 42);

    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);
    CHECK_COUNT(t, col, 0);

    sll_drop(col);
}

/* ---- replace_front ---- */

static void test_sll_replace_front(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 10);
    sll_push_back(col, 20);

    int old = sll_replace_front(col, 99);

    CVXCHECK(t, old == 10);
    CVXCHECK(t, sll_front(col) == 99);
    CHECK_COUNT(t, col, 2);

    sll_drop(col);
}

static void test_sll_replace_front_on_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_replace_front(col, 42);

    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);
    CHECK_COUNT(t, col, 0);

    sll_drop(col);
}

/* ---- clear ---- */

static void test_sll_clear(struct cvxtest *t)
{
    cvx_container *col = sll_new();

    sll_push_back(col, 1);
    sll_push_back(col, 2);
    sll_push_back(col, 3);
    sll_clear(col);

    CHECK_COUNT(t, col, 0);
    CVXCHECK(t, ((struct sll_int *)col)->head == NULL);
    CVXCHECK(t, ((struct sll_int *)col)->tail == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    sll_drop(col);
}

/* ---- clone (int / direct-assignment branch) ---- */

static void test_sll_clone_empty(struct cvxtest *t)
{
    cvx_container *col = sll_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    cvx_container *clone = sll_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        sll_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CHECK_COUNT(t, clone, 0);

    sll_drop(col);
    sll_drop(clone);
}

static void test_sll_clone_values(struct cvxtest *t)
{
    cvx_container *col = sll_new();
    sll_push_back(col, 10);
    sll_push_back(col, 20);
    sll_push_back(col, 30);

    cvx_container *clone = sll_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        sll_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CHECK_COUNT(t, clone, 3);
    CVXCHECK(t, sll_get(clone, 0) == 10);
    CVXCHECK(t, sll_get(clone, 1) == 20);
    CVXCHECK(t, sll_get(clone, 2) == 30);

    sll_drop(col);
    sll_drop(clone);
}

/* ---- wrong tag guard ---- */

static void test_sll_wrong_tag(struct cvxtest *t)
{
    cvx_container *col = sll_new();
    col->tag = 0;
    sll_push_back(col, 1);

    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);

    col->tag = 77;
    sll_drop(col);
}

/* ---- runner ---- */

static int run_slinked_list_tests(void)
{
    struct cvxtest t = { 0 };

    printf("slinked_list\n");

    CVXRUN(&t, test_sll_init);
    CVXRUN(&t, test_sll_new);

    CVXRUN(&t, test_sll_push_back);
    CVXRUN(&t, test_sll_push_back_many);
    CVXRUN(&t, test_sll_push_front);
    CVXRUN(&t, test_sll_push_at_middle);
    CVXRUN(&t, test_sll_push_at_head);
    CVXRUN(&t, test_sll_push_at_tail);
    CVXRUN(&t, test_sll_push_at_out_of_range);

    CVXRUN(&t, test_sll_pop_front);
    CVXRUN(&t, test_sll_pop_front_to_empty);
    CVXRUN(&t, test_sll_pop_front_empty);
    CVXRUN(&t, test_sll_pop_back);
    CVXRUN(&t, test_sll_pop_back_to_empty);
    CVXRUN(&t, test_sll_pop_back_empty);
    CVXRUN(&t, test_sll_pop_at_middle);
    CVXRUN(&t, test_sll_pop_at_out_of_range);
    CVXRUN(&t, test_sll_pop_at_empty);

    CVXRUN(&t, test_sll_front_empty);
    CVXRUN(&t, test_sll_back_empty);

    CVXRUN(&t, test_sll_get);
    CVXRUN(&t, test_sll_get_out_of_range);
    CVXRUN(&t, test_sll_get_empty);

    CVXRUN(&t, test_sll_empty);

    CVXRUN(&t, test_sll_replace_back);
    CVXRUN(&t, test_sll_replace_back_on_empty);
    CVXRUN(&t, test_sll_replace_front);
    CVXRUN(&t, test_sll_replace_front_on_empty);

    CVXRUN(&t, test_sll_clear);
    CVXRUN(&t, test_sll_wrong_tag);

    CVXRUN(&t, test_sll_clone_empty);
    CVXRUN(&t, test_sll_clone_values);

    return CVXSUMMARY(&t);
}

#endif /* SLINKED_LIST_TESTS_H */
