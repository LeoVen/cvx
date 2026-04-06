#ifndef DYNAMIC_ARRAY_TESTS_H
#define DYNAMIC_ARRAY_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"
#include "implementations.h"

/* ---- init / new ---- */

static void test_da_int_init(struct cvxtest *t)
{
    struct dynamic_array_int arr = da_int_init(NULL);
    cvx_container *col = (cvx_container *)(&arr);

    CVXCHECK(t, col->tag == 99);
    CVXCHECK(t, arr.buffer == NULL);
    CVXCHECK(t, arr.capacity == 0);
    CVXCHECK(t, arr.count == 0);
}

static void test_da_int_init_with(struct cvxtest *t)
{
    struct dynamic_array_int arr = da_int_init_with(NULL, 8);
    cvx_container *col = cvx_col(arr);

    CVXCHECK(t, arr.buffer != NULL);
    CVXCHECK(t, da_int_capacity(col) == 8);
    CVXCHECK(t, da_int_count(col) == 0);

    free(arr.buffer);
}

static void test_da_int_new(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->tag == 99);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, da_int_count(col) == 0);
    CVXCHECK(t, da_int_capacity(col) == 0);
    da_int_drop(col);
}

static void test_da_int_new_with(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, da_int_capacity(col) == 4);
    CVXCHECK(t, da_int_count(col) == 0);
    da_int_drop(col);
}

static void test_da_int_new_with_zero_returns_null(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 0);
    CVXCHECK(t, col == NULL);
}

/* ---- push_back / count / capacity growth ---- */

static void test_da_int_push_back_grows(struct cvxtest *t)
{
    cvx_container *col = da_int_new();

    for (int i = 0; i < 20; i++)
        da_int_push_back(col, i);

    CVXCHECK(t, da_int_count(col) == 20);
    CVXCHECK(t, da_int_capacity(col) >= 20);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    da_int_drop(col);
}

static void test_da_int_push_back_values(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 10);
    da_int_push_back(col, 20);
    da_int_push_back(col, 30);

    struct dynamic_array_int *self = (struct dynamic_array_int *)col;
    CVXCHECK(t, self->buffer[0] == 10);
    CVXCHECK(t, self->buffer[1] == 20);
    CVXCHECK(t, self->buffer[2] == 30);

    da_int_drop(col);
}

/* ---- push_front ---- */

static void test_da_int_push_front(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 1);
    da_int_push_back(col, 2);
    da_int_push_front(col, 0);

    struct dynamic_array_int *self = (struct dynamic_array_int *)col;
    CVXCHECK(t, self->buffer[0] == 0);
    CVXCHECK(t, self->buffer[1] == 1);
    CVXCHECK(t, self->buffer[2] == 2);
    CVXCHECK(t, da_int_count(col) == 3);

    da_int_drop(col);
}

/* ---- push_at ---- */

static void test_da_int_push_at_middle(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 1);
    da_int_push_back(col, 3);
    da_int_push_at(col, 2, 1);

    struct dynamic_array_int *self = (struct dynamic_array_int *)col;
    CVXCHECK(t, self->buffer[0] == 1);
    CVXCHECK(t, self->buffer[1] == 2);
    CVXCHECK(t, self->buffer[2] == 3);
    CVXCHECK(t, da_int_count(col) == 3);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    da_int_drop(col);
}

static void test_da_int_push_at_out_of_range(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 1);
    da_int_push_at(col, 99, 5);

    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    da_int_drop(col);
}

/* ---- pop_back ---- */

static void test_da_int_pop_back(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 42);
    da_int_push_back(col, 7);

    int out = da_int_pop_back(col);

    CVXCHECK(t, out == 7);
    CVXCHECK(t, da_int_count(col) == 1);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    da_int_drop(col);
}

static void test_da_int_pop_back_empty(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_pop_back(col);

    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    da_int_drop(col);
}

/* ---- pop_front ---- */

static void test_da_int_pop_front(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 10);
    da_int_push_back(col, 20);

    int out = da_int_pop_front(col);

    CVXCHECK(t, out == 10);
    CVXCHECK(t, da_int_count(col) == 1);

    struct dynamic_array_int *self = (struct dynamic_array_int *)col;
    CVXCHECK(t, self->buffer[0] == 20);

    da_int_drop(col);
}

static void test_da_int_pop_front_empty(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_pop_front(col);

    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    da_int_drop(col);
}

/* ---- pop_at ---- */

static void test_da_int_pop_at_middle(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 1);
    da_int_push_back(col, 2);
    da_int_push_back(col, 3);

    int out = da_int_pop_at(col, 1);

    CVXCHECK(t, out == 2);
    CVXCHECK(t, da_int_count(col) == 2);

    struct dynamic_array_int *self = (struct dynamic_array_int *)col;
    CVXCHECK(t, self->buffer[0] == 1);
    CVXCHECK(t, self->buffer[1] == 3);

    da_int_drop(col);
}

static void test_da_int_pop_at_out_of_range(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 1);
    da_int_pop_at(col, 5);

    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    da_int_drop(col);
}

static void test_da_int_pop_at_empty(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_pop_at(col, 0);

    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    da_int_drop(col);
}

/* ---- front / back ---- */

static void test_da_int_front(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 11);
    da_int_push_back(col, 22);

    CVXCHECK(t, da_int_front(col) == 11);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    da_int_drop(col);
}

static void test_da_int_back(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 11);
    da_int_push_back(col, 22);

    CVXCHECK(t, da_int_back(col) == 22);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    da_int_drop(col);
}

static void test_da_int_front_empty(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_front(col);

    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    da_int_drop(col);
}

static void test_da_int_back_empty(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_back(col);

    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);

    da_int_drop(col);
}

/* ---- empty / full ---- */

static void test_da_int_empty(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    CVXCHECK(t, da_int_empty(col) == true);

    da_int_push_back(col, 1);
    CVXCHECK(t, da_int_empty(col) == false);

    da_int_drop(col);
}

static void test_da_int_full(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 2);
    CVXCHECK(t, da_int_full(col) == false);

    da_int_push_back(col, 1);
    da_int_push_back(col, 2);
    CVXCHECK(t, da_int_full(col) == true);

    da_int_drop(col);
}

/* ---- replace_back ---- */

static void test_da_int_replace_back(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 5);
    da_int_push_back(col, 10);

    int old = da_int_replace_back(col, 99);

    CVXCHECK(t, old == 10);
    CVXCHECK(t, da_int_back(col) == 99);
    CVXCHECK(t, da_int_count(col) == 2);

    da_int_drop(col);
}

static void test_da_int_replace_back_on_empty(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);

    da_int_replace_back(col, 42);

    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, da_int_count(col) == 0);

    da_int_drop(col);
}

/* ---- get ---- */

static void test_da_int_get(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 10);
    da_int_push_back(col, 20);
    da_int_push_back(col, 30);

    CVXCHECK(t, da_int_get(col, 0) == 10);
    CVXCHECK(t, da_int_get(col, 1) == 20);
    CVXCHECK(t, da_int_get(col, 2) == 30);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    da_int_drop(col);
}

static void test_da_int_get_out_of_range(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 1);

    da_int_get(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    da_int_drop(col);
}

static void test_da_int_get_empty(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);

    da_int_get(col, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_RANGE);

    da_int_drop(col);
}

/* ---- replace_front ---- */

static void test_da_int_replace_front(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 10);
    da_int_push_back(col, 20);

    int old = da_int_replace_front(col, 99);

    CVXCHECK(t, old == 10);
    CVXCHECK(t, da_int_front(col) == 99);
    CVXCHECK(t, da_int_count(col) == 2);

    da_int_drop(col);
}

static void test_da_int_replace_front_on_empty(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);

    da_int_replace_front(col, 42);

    CVXCHECK(t, col->flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, da_int_count(col) == 0);

    da_int_drop(col);
}

/* ---- clear ---- */

static void test_da_int_clear(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 1);
    da_int_push_back(col, 2);
    da_int_clear(col);

    CVXCHECK(t, da_int_count(col) == 0);
    CVXCHECK(t, da_int_capacity(col) == 0);
    CVXCHECK(t, ((struct dynamic_array_int *)col)->buffer == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    da_int_drop(col);
}

/* ---- wrong tag guard ---- */

static void test_da_int_wrong_tag(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    col->tag = 0; /* corrupt the tag */
    da_int_push_back(col, 1);

    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);

    /* restore tag so drop works */
    col->tag = 99;
    da_int_drop(col);
}

/* ---- copy (int / memcpy branch) ---- */

static void test_da_int_copy_empty(struct cvxtest *t)
{
    struct dynamic_array_int orig = da_int_init(NULL);
    struct dynamic_array_int copy = da_int_copy(&orig);

    CVXCHECK(t, copy.count == 0);
    CVXCHECK(t, copy.buffer == NULL);
    CVXCHECK(t, ((cvx_container *)&copy)->flag == CVX_FLAG_OK);
}

static void test_da_int_copy_values(struct cvxtest *t)
{
    struct dynamic_array_int orig = da_int_init_with(NULL, 4);
    cvx_container *col = (cvx_container *)&orig;

    da_int_push_back(col, 1);
    da_int_push_back(col, 2);
    da_int_push_back(col, 3);

    struct dynamic_array_int copy = da_int_copy(&orig);

    CVXCHECK(t, copy.count == 3);
    CVXCHECK(t, copy.buffer[0] == 1);
    CVXCHECK(t, copy.buffer[1] == 2);
    CVXCHECK(t, copy.buffer[2] == 3);
    /* Buffers must be distinct allocations */
    CVXCHECK(t, copy.buffer != orig.buffer);

    free(orig.buffer);
    free(copy.buffer);
}

/* ---- clone (int / memcpy branch) ---- */

static void test_da_int_clone_empty(struct cvxtest *t)
{
    cvx_container *col = da_int_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    cvx_container *clone = da_int_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        da_int_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CVXCHECK(t, da_int_count(col) == 0);

    da_int_drop(col);
    da_int_drop(clone);
}

static void test_da_int_clone_values(struct cvxtest *t)
{
    cvx_container *col = da_int_new_with(NULL, 4);
    da_int_push_back(col, 10);
    da_int_push_back(col, 20);
    da_int_push_back(col, 30);

    cvx_container *clone = da_int_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        da_int_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CVXCHECK(t, da_int_get(clone, 0) == 10);
    CVXCHECK(t, da_int_get(clone, 1) == 20);
    CVXCHECK(t, da_int_get(clone, 2) == 30);
    /* Distinct allocation */
    CVXCHECK(t, ((struct dynamic_array_int *)clone)->buffer !=
                    ((struct dynamic_array_int *)col)->buffer);

    da_int_drop(col);
    da_int_drop(clone);
}

/* ---- runner ---- */

static int run_dynamic_array_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dynamic_array\n");

    CVXRUN(&t, test_da_int_init);
    CVXRUN(&t, test_da_int_init_with);
    CVXRUN(&t, test_da_int_new);
    CVXRUN(&t, test_da_int_new_with);
    CVXRUN(&t, test_da_int_new_with_zero_returns_null);

    CVXRUN(&t, test_da_int_push_back_grows);
    CVXRUN(&t, test_da_int_push_back_values);
    CVXRUN(&t, test_da_int_push_front);
    CVXRUN(&t, test_da_int_push_at_middle);
    CVXRUN(&t, test_da_int_push_at_out_of_range);

    CVXRUN(&t, test_da_int_pop_back);
    CVXRUN(&t, test_da_int_pop_back_empty);
    CVXRUN(&t, test_da_int_pop_front);
    CVXRUN(&t, test_da_int_pop_front_empty);
    CVXRUN(&t, test_da_int_pop_at_middle);
    CVXRUN(&t, test_da_int_pop_at_out_of_range);
    CVXRUN(&t, test_da_int_pop_at_empty);

    CVXRUN(&t, test_da_int_front);
    CVXRUN(&t, test_da_int_back);
    CVXRUN(&t, test_da_int_front_empty);
    CVXRUN(&t, test_da_int_back_empty);

    CVXRUN(&t, test_da_int_empty);
    CVXRUN(&t, test_da_int_full);

    CVXRUN(&t, test_da_int_get);
    CVXRUN(&t, test_da_int_get_out_of_range);
    CVXRUN(&t, test_da_int_get_empty);

    CVXRUN(&t, test_da_int_replace_back);
    CVXRUN(&t, test_da_int_replace_back_on_empty);

    CVXRUN(&t, test_da_int_replace_front);
    CVXRUN(&t, test_da_int_replace_front_on_empty);

    CVXRUN(&t, test_da_int_clear);
    CVXRUN(&t, test_da_int_wrong_tag);

    CVXRUN(&t, test_da_int_copy_empty);
    CVXRUN(&t, test_da_int_copy_values);

    CVXRUN(&t, test_da_int_clone_empty);
    CVXRUN(&t, test_da_int_clone_values);

    return CVXSUMMARY(&t);
}

#endif /* DYNAMIC_ARRAY_TESTS_H */
