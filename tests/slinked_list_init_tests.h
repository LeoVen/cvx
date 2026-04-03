#ifndef SLINKED_LIST_INIT_TESTS_H
#define SLINKED_LIST_INIT_TESTS_H

#include <string.h>

#include "cvxtest.h"
#include "cvxtestutils.h"

#define V char *
#define V_COPY strdup
#define V_DROP free
#define SNAME sll_str
#define PFX sll_str
#define TAG 92
#include "cvx/slinked_list.h"

/* ---- _init ---- */

static void test_sll_str_init(struct cvxtest *t)
{
    struct sll_str list = sll_str_init();
    cvx_container *col = (cvx_container *)(&list);

    CVXCHECK(t, col->tag == 92);
    CVXCHECK(t, list.head == NULL);
    CVXCHECK(t, list.tail == NULL);
    CVXCHECK(t, list.count == 0);

    /* Stack-allocated, no elements: nothing to free */
}

static void test_sll_str_init_push_clear(struct cvxtest *t)
{
    struct sll_str list = sll_str_init();
    cvx_container *col = (cvx_container *)(&list);

    sll_str_push_back(col, mkstr("hello"));
    sll_str_push_back(col, mkstr("world"));

    CVXCHECK(t, list.count == 2);

    /* _clear must call V_DROP on each node value and free nodes */
    sll_str_clear(col);

    CVXCHECK(t, list.count == 0);
    CVXCHECK(t, list.head == NULL);
    CVXCHECK(t, list.tail == NULL);
}

/* ---- _new ---- */

static void test_sll_str_new(struct cvxtest *t)
{
    cvx_container *col = sll_str_new();

    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->tag == 92);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    struct sll_str *list = (struct sll_str *)col;
    CVXCHECK(t, list->head == NULL);
    CVXCHECK(t, list->tail == NULL);
    CVXCHECK(t, list->count == 0);

    sll_str_drop(col);
}

static void test_sll_str_new_push_drop(struct cvxtest *t)
{
    cvx_container *col = sll_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    sll_str_push_back(col, mkstr("one"));
    sll_str_push_back(col, mkstr("two"));
    sll_str_push_back(col, mkstr("three"));

    CVXCHECK(t, sll_str_count(col) == 3);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    sll_str_drop(col);
}

static void test_sll_str_new_front_back_drop(struct cvxtest *t)
{
    cvx_container *col = sll_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    sll_str_push_front(col, mkstr("front"));
    sll_str_push_back(col, mkstr("middle"));
    sll_str_push_back(col, mkstr("back"));

    CVXCHECK(t, sll_str_count(col) == 3);

    sll_str_drop(col);
}

/* ---- _clone ---- */

static void test_sll_str_clone_empty(struct cvxtest *t)
{
    cvx_container *col = sll_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    cvx_container *clone = sll_str_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        sll_str_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CVXCHECK(t, sll_str_count(clone) == 0);

    sll_str_drop(col);
    sll_str_drop(clone);
}

static void test_sll_str_clone_deep(struct cvxtest *t)
{
    cvx_container *col = sll_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    sll_str_push_back(col, mkstr("a"));
    sll_str_push_back(col, mkstr("b"));
    sll_str_push_back(col, mkstr("c"));

    cvx_container *clone = sll_str_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        sll_str_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CVXCHECK(t, sll_str_count(clone) == 3);

    CVXCHECK(t, strcmp(sll_str_front(clone), "a") == 0);
    CVXCHECK(t, strcmp(sll_str_back(clone), "c") == 0);

    struct sll_str *orig_list = (struct sll_str *)col;
    struct sll_str *clone_list = (struct sll_str *)clone;
    CVXCHECK(t, orig_list->head->value != clone_list->head->value);
    CVXCHECK(t, orig_list->tail->value != clone_list->tail->value);

    sll_str_drop(col);
    sll_str_drop(clone);
}

static void test_sll_str_clone_independence(struct cvxtest *t)
{
    cvx_container *col = sll_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    sll_str_push_back(col, mkstr("original"));

    cvx_container *clone = sll_str_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        sll_str_drop(col);
        return;
    }

    sll_str_drop(col);

    CVXCHECK(t, strcmp(sll_str_front(clone), "original") == 0);
    CVXCHECK(t, sll_str_count(clone) == 1);

    sll_str_drop(clone);
}

static void test_sll_str_clone_order(struct cvxtest *t)
{
    cvx_container *col = sll_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    sll_str_push_back(col, mkstr("first"));
    sll_str_push_back(col, mkstr("second"));
    sll_str_push_back(col, mkstr("third"));

    cvx_container *clone = sll_str_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        sll_str_drop(col);
        return;
    }

    CVXCHECK(t, strcmp(sll_str_get(clone, 0), "first") == 0);
    CVXCHECK(t, strcmp(sll_str_get(clone, 1), "second") == 0);
    CVXCHECK(t, strcmp(sll_str_get(clone, 2), "third") == 0);

    sll_str_drop(col);
    sll_str_drop(clone);
}

/* ---- _clear resets and allows reuse ---- */

static void test_sll_str_clear_drops_elements(struct cvxtest *t)
{
    cvx_container *col = sll_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    sll_str_push_back(col, mkstr("p"));
    sll_str_push_back(col, mkstr("q"));

    sll_str_clear(col);

    CVXCHECK(t, sll_str_count(col) == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    struct sll_str *list = (struct sll_str *)col;
    CVXCHECK(t, list->head == NULL);
    CVXCHECK(t, list->tail == NULL);

    sll_str_push_back(col, mkstr("r"));
    CVXCHECK(t, sll_str_count(col) == 1);

    sll_str_drop(col);
}

/* ---- runner ---- */

static int run_slinked_list_init_tests(void)
{
    struct cvxtest t = { 0 };

    printf("slinked_list (init, char*)\n");

    CVXRUN(&t, test_sll_str_init);
    CVXRUN(&t, test_sll_str_init_push_clear);

    CVXRUN(&t, test_sll_str_new);
    CVXRUN(&t, test_sll_str_new_push_drop);
    CVXRUN(&t, test_sll_str_new_front_back_drop);

    CVXRUN(&t, test_sll_str_clone_empty);
    CVXRUN(&t, test_sll_str_clone_deep);
    CVXRUN(&t, test_sll_str_clone_independence);
    CVXRUN(&t, test_sll_str_clone_order);

    CVXRUN(&t, test_sll_str_clear_drops_elements);

    return CVXSUMMARY(&t);
}

#endif /* SLINKED_LIST_INIT_TESTS_H */
