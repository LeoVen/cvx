#ifndef DLINKED_LIST_TESTS_H
#define DLINKED_LIST_TESTS_H

#include "cvxtest.h"

#include "implementations.h"

/* ---- init / new ---- */

static void test_dll_int_init(struct cvxtest *t)
{
    struct dlinked_int l = dll_int_init(NULL);
    cvx_container *col = (cvx_container *)(&l);

    CVXCHECK(t, col->tag == 88);
    CVXCHECK(t, l.head == NULL);
    CVXCHECK(t, l.tail == NULL);
    CVXCHECK(t, l.count == 0);
    CVXCHECK(t, l.vtabv == NULL);
}

/* ---- copy ---- */

static void test_dll_int_copy_empty(struct cvxtest *t)
{
    struct dlinked_int orig = dll_int_init(NULL);
    struct dlinked_int copy = dll_int_copy(&orig);

    CVXCHECK(t, copy.count == 0);
    CVXCHECK(t, copy.head == NULL);
    CVXCHECK(t, copy.tail == NULL);
    CVXCHECK(t, copy.super.flag == CVX_FLAG_OK);
}

static void test_dll_int_copy_values(struct cvxtest *t)
{
    struct dlinked_int orig = dll_int_init(NULL);

    dll_int_push_back(&orig, 10);
    dll_int_push_back(&orig, 20);
    dll_int_push_back(&orig, 30);

    struct dlinked_int copy = dll_int_copy(&orig);

    CVXCHECK(t, copy.count == 3);
    CVXCHECK(t, dll_int_get(&copy, 0) == 10);
    CVXCHECK(t, dll_int_get(&copy, 1) == 20);
    CVXCHECK(t, dll_int_get(&copy, 2) == 30);
    /* Nodes must be distinct allocations */
    CVXCHECK(t, copy.head != orig.head);
    /* Prev links must be correct */
    CVXCHECK(t, copy.head->prev == NULL);
    CVXCHECK(t, copy.head->next->prev == copy.head);
    CVXCHECK(t, copy.tail->next == NULL);

    dll_int_clear(&orig);
    dll_int_clear(&copy);
}

static void test_dll_int_new(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->super.tag == 88);
    CVXCHECK(t, col->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_count(col) == 0);

    dll_int_drop(col);
}

static void test_dll_int_new_with(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new_with(dll_int_vtabv_full);
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->super.tag == 88);
    CVXCHECK(t, col->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_count(col) == 0);
    CVXCHECK(t, col->vtabv == dll_int_vtabv_full);

    dll_int_drop(col);
}

/* ---- clone ---- */

static void test_dll_int_clone_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    struct dlinked_int *clone = dll_int_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        dll_int_drop(col);
        return;
    }

    CVXCHECK(t, clone->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_count(clone) == 0);

    dll_int_drop(col);
    dll_int_drop(clone);
}

static void test_dll_int_clone_values(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);

    struct dlinked_int *clone = dll_int_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        dll_int_drop(col);
        return;
    }

    CVXCHECK(t, clone->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_count(clone) == 3);
    CVXCHECK(t, dll_int_get(clone, 0) == 10);
    CVXCHECK(t, dll_int_get(clone, 1) == 20);
    CVXCHECK(t, dll_int_get(clone, 2) == 30);

    CVXCHECK(t, clone->head->prev == NULL);
    CVXCHECK(t, clone->tail->next == NULL);
    CVXCHECK(t, clone->head->next->prev == clone->head);

    dll_int_drop(col);
    dll_int_drop(clone);
}

/* ---- clear ---- */

static void test_dll_int_clear(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 1);
    dll_int_push_back(col, 2);
    dll_int_push_back(col, 3);
    dll_int_clear(col);

    CVXCHECK(t, dll_int_count(col) == 0);
    CVXCHECK(t, col->head == NULL);
    CVXCHECK(t, col->tail == NULL);
    CVXCHECK(t, col->super.flag == CVX_FLAG_OK);

    dll_int_drop(col);
}

/* ---- empty ---- */

static void test_dll_int_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    CVXCHECK(t, dll_int_empty(col) == true);

    dll_int_push_back(col, 1);
    CVXCHECK(t, dll_int_empty(col) == false);

    dll_int_pop_front(col);
    CVXCHECK(t, dll_int_empty(col) == true);

    dll_int_drop(col);
}

/* ---- front / back ---- */

static void test_dll_int_front_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_front(col);
    CVXCHECK(t, col->super.flag == CVX_FLAG_EMPTY);

    dll_int_drop(col);
}

static void test_dll_int_back_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_back(col);
    CVXCHECK(t, col->super.flag == CVX_FLAG_EMPTY);

    dll_int_drop(col);
}

/* ---- get ---- */

static void test_dll_int_get(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);

    CVXCHECK(t, dll_int_get(col, 0) == 10);
    CVXCHECK(t, dll_int_get(col, 1) == 20);
    CVXCHECK(t, dll_int_get(col, 2) == 30);
    CVXCHECK(t, col->super.flag == CVX_FLAG_OK);

    dll_int_drop(col);
}

static void test_dll_int_get_out_of_range(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 1);
    dll_int_get(col, 1);

    CVXCHECK(t, col->super.flag == CVX_FLAG_RANGE);

    dll_int_drop(col);
}

static void test_dll_int_get_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_get(col, 0);
    CVXCHECK(t, col->super.flag == CVX_FLAG_RANGE);

    dll_int_drop(col);
}

/* ---- push_front ---- */

static void test_dll_int_push_front(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 2);
    dll_int_push_back(col, 3);
    dll_int_push_front(col, 1);

    CVXCHECK(t, dll_int_count(col) == 3);
    CVXCHECK(t, dll_int_front(col) == 1);
    CVXCHECK(t, dll_int_back(col) == 3);
    CVXCHECK(t, dll_int_get(col, 1) == 2);

    CVXCHECK(t, col->head->prev == NULL);
    CVXCHECK(t, col->head->next->prev == col->head);

    dll_int_drop(col);
}

/* ---- push_back / count ---- */

static void test_dll_int_push_back(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);

    CVXCHECK(t, dll_int_count(col) == 3);
    CVXCHECK(t, col->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_front(col) == 10);
    CVXCHECK(t, dll_int_back(col) == 30);

    dll_int_drop(col);
}

static void test_dll_int_push_back_many(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    for (int i = 0; i < 100; i++)
        dll_int_push_back(col, i);

    CVXCHECK(t, dll_int_count(col) == 100);
    CVXCHECK(t, dll_int_front(col) == 0);
    CVXCHECK(t, dll_int_back(col) == 99);

    dll_int_drop(col);
}

/* ---- prev-link integrity ---- */

static void test_dll_int_prev_links(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 1);
    dll_int_push_back(col, 2);
    dll_int_push_back(col, 3);

    CVXCHECK(t, col->head->prev == NULL);
    CVXCHECK(t, col->head->next->prev == col->head);
    CVXCHECK(t, col->tail->prev == col->head->next);

    dll_int_drop(col);
}

/* ---- push_at ---- */

static void test_dll_int_push_at_middle(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 1);
    dll_int_push_back(col, 3);
    dll_int_push_at(col, 2, 1);

    CVXCHECK(t, dll_int_count(col) == 3);
    CVXCHECK(t, dll_int_get(col, 0) == 1);
    CVXCHECK(t, dll_int_get(col, 1) == 2);
    CVXCHECK(t, dll_int_get(col, 2) == 3);
    CVXCHECK(t, col->super.flag == CVX_FLAG_OK);

    CVXCHECK(t, col->head->next->prev == col->head);
    CVXCHECK(t, col->tail->prev->next == col->tail);

    dll_int_drop(col);
}

static void test_dll_int_push_at_head(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 2);
    dll_int_push_at(col, 1, 0);

    CVXCHECK(t, dll_int_front(col) == 1);
    CVXCHECK(t, dll_int_back(col) == 2);

    dll_int_drop(col);
}

static void test_dll_int_push_at_tail(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 1);
    dll_int_push_at(col, 2, 1);

    CVXCHECK(t, dll_int_back(col) == 2);

    dll_int_drop(col);
}

static void test_dll_int_push_at_out_of_range(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 1);
    dll_int_push_at(col, 99, 5);

    CVXCHECK(t, col->super.flag == CVX_FLAG_RANGE);

    dll_int_drop(col);
}

/* ---- pop_front ---- */

static void test_dll_int_pop_front(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);

    int out = dll_int_pop_front(col);

    CVXCHECK(t, out == 10);
    CVXCHECK(t, dll_int_count(col) == 1);
    CVXCHECK(t, dll_int_front(col) == 20);

    CVXCHECK(t, col->head->prev == NULL);

    dll_int_drop(col);
}

static void test_dll_int_pop_front_to_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 1);
    dll_int_pop_front(col);

    CVXCHECK(t, dll_int_count(col) == 0);
    CVXCHECK(t, col->head == NULL);
    CVXCHECK(t, col->tail == NULL);

    dll_int_drop(col);
}

static void test_dll_int_pop_front_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_pop_front(col);
    CVXCHECK(t, col->super.flag == CVX_FLAG_EMPTY);

    dll_int_drop(col);
}

/* ---- pop_back ---- */

static void test_dll_int_pop_back(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);

    int out = dll_int_pop_back(col);

    CVXCHECK(t, out == 20);
    CVXCHECK(t, dll_int_count(col) == 1);
    CVXCHECK(t, dll_int_back(col) == 10);

    CVXCHECK(t, col->tail->next == NULL);

    dll_int_drop(col);
}

static void test_dll_int_pop_back_to_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 1);
    dll_int_pop_back(col);

    CVXCHECK(t, dll_int_count(col) == 0);
    CVXCHECK(t, col->head == NULL);
    CVXCHECK(t, col->tail == NULL);

    dll_int_drop(col);
}

static void test_dll_int_pop_back_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_pop_back(col);
    CVXCHECK(t, col->super.flag == CVX_FLAG_EMPTY);

    dll_int_drop(col);
}

/* ---- pop_at ---- */

static void test_dll_int_pop_at_middle(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 1);
    dll_int_push_back(col, 2);
    dll_int_push_back(col, 3);

    int out = dll_int_pop_at(col, 1);

    CVXCHECK(t, out == 2);
    CVXCHECK(t, dll_int_count(col) == 2);
    CVXCHECK(t, dll_int_get(col, 0) == 1);
    CVXCHECK(t, dll_int_get(col, 1) == 3);

    CVXCHECK(t, col->tail->prev == col->head);
    CVXCHECK(t, col->head->next == col->tail);

    dll_int_drop(col);
}

/* ---- pop_at edge cases ---- */

// pop_at(col, 0) must delegate to pop_front.
static void test_dll_int_pop_at_front(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);

    int v = dll_int_pop_at(col, 0);
    CVXCHECK(t, v == 10);
    CVXCHECK(t, dll_int_count(col) == 2);
    CVXCHECK(t, dll_int_front(col) == 20);
    CVXCHECK(t, col->super.flag == CVX_FLAG_OK);

    dll_int_drop(col);
}

// pop_at(col, count-1) must delegate to pop_back.
static void test_dll_int_pop_at_back(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);

    int v = dll_int_pop_at(col, 2); // count-1 == 2
    CVXCHECK(t, v == 30);
    CVXCHECK(t, dll_int_count(col) == 2);
    CVXCHECK(t, dll_int_back(col) == 20);
    CVXCHECK(t, col->super.flag == CVX_FLAG_OK);

    dll_int_drop(col);
}

static void test_dll_int_pop_at_out_of_range(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 1);
    dll_int_pop_at(col, 5);

    CVXCHECK(t, col->super.flag == CVX_FLAG_RANGE);

    dll_int_drop(col);
}

static void test_dll_int_pop_at_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_pop_at(col, 0);
    CVXCHECK(t, col->super.flag == CVX_FLAG_EMPTY);

    dll_int_drop(col);
}

/* ---- replace_front ---- */

static void test_dll_int_replace_front(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);

    int old = dll_int_replace_front(col, 99);

    CVXCHECK(t, old == 10);
    CVXCHECK(t, dll_int_front(col) == 99);
    CVXCHECK(t, dll_int_count(col) == 2);

    dll_int_drop(col);
}

static void test_dll_int_replace_front_on_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_replace_front(col, 42);

    CVXCHECK(t, col->super.flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, dll_int_count(col) == 0);

    dll_int_drop(col);
}

/* ---- replace_back ---- */

static void test_dll_int_replace_back(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_push_back(col, 5);
    dll_int_push_back(col, 10);

    int old = dll_int_replace_back(col, 99);

    CVXCHECK(t, old == 10);
    CVXCHECK(t, dll_int_back(col) == 99);
    CVXCHECK(t, dll_int_count(col) == 2);

    dll_int_drop(col);
}

static void test_dll_int_replace_back_on_empty(struct cvxtest *t)
{
    struct dlinked_int *col = dll_int_new();

    dll_int_replace_back(col, 42);

    CVXCHECK(t, col->super.flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, dll_int_count(col) == 0);

    dll_int_drop(col);
}

/* ---- runner ---- */

static struct cvxresult run_dlinked_list_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dlinked_list\n");

    CVXRUN(&t, test_dll_int_init);

    CVXRUN(&t, test_dll_int_copy_empty);
    CVXRUN(&t, test_dll_int_copy_values);

    CVXRUN(&t, test_dll_int_new);
    CVXRUN(&t, test_dll_int_new_with);

    CVXRUN(&t, test_dll_int_clone_empty);
    CVXRUN(&t, test_dll_int_clone_values);

    CVXRUN(&t, test_dll_int_clear);

    CVXRUN(&t, test_dll_int_empty);

    CVXRUN(&t, test_dll_int_front_empty);
    CVXRUN(&t, test_dll_int_back_empty);

    CVXRUN(&t, test_dll_int_get);
    CVXRUN(&t, test_dll_int_get_out_of_range);
    CVXRUN(&t, test_dll_int_get_empty);

    CVXRUN(&t, test_dll_int_push_front);

    CVXRUN(&t, test_dll_int_push_back);
    CVXRUN(&t, test_dll_int_push_back_many);
    CVXRUN(&t, test_dll_int_prev_links);

    CVXRUN(&t, test_dll_int_push_at_middle);
    CVXRUN(&t, test_dll_int_push_at_head);
    CVXRUN(&t, test_dll_int_push_at_tail);
    CVXRUN(&t, test_dll_int_push_at_out_of_range);

    CVXRUN(&t, test_dll_int_pop_front);
    CVXRUN(&t, test_dll_int_pop_front_to_empty);
    CVXRUN(&t, test_dll_int_pop_front_empty);

    CVXRUN(&t, test_dll_int_pop_back);
    CVXRUN(&t, test_dll_int_pop_back_to_empty);
    CVXRUN(&t, test_dll_int_pop_back_empty);

    CVXRUN(&t, test_dll_int_pop_at_middle);
    CVXRUN(&t, test_dll_int_pop_at_front);
    CVXRUN(&t, test_dll_int_pop_at_back);
    CVXRUN(&t, test_dll_int_pop_at_out_of_range);
    CVXRUN(&t, test_dll_int_pop_at_empty);

    CVXRUN(&t, test_dll_int_replace_front);
    CVXRUN(&t, test_dll_int_replace_front_on_empty);

    CVXRUN(&t, test_dll_int_replace_back);
    CVXRUN(&t, test_dll_int_replace_back_on_empty);

    return CVXSUMMARY(&t);
}

#endif /* DLINKED_LIST_TESTS_H */
