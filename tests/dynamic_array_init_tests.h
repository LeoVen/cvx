#ifndef DYNAMIC_ARRAY_INIT_TESTS_H
#define DYNAMIC_ARRAY_INIT_TESTS_H

#include <string.h>

#include "cvxtest.h"
#include "cvxtestutils.h"

#define V char *
#define V_COPY strdup
#define V_DROP free
#define SNAME da_str
#define PFX da_str
#define TAG 91
#include "cvx/dynamic_array.h"

/* ---- _init ---- */

static void test_da_str_init(struct cvxtest *t)
{
    struct da_str arr = da_str_init();

    CVXCHECK(t, cvx_col(arr)->tag == 91);
    CVXCHECK(t, arr.buffer == NULL);
    CVXCHECK(t, arr.capacity == 0);
    CVXCHECK(t, arr.count == 0);

    /* Nothing to free: buffer is NULL, struct is on the stack */
}

/* ---- _init_with ---- */

static void test_da_str_init_with(struct cvxtest *t)
{
    struct da_str arr = da_str_init_with(4);

    CVXCHECK(t, arr.buffer != NULL);
    CVXCHECK(t, arr.capacity == 4);
    CVXCHECK(t, arr.count == 0);
    CVXCHECK(t, cvx_col(arr)->flag == CVX_FLAG_OK || cvx_col(arr)->flag == 0);

    da_str_clear(cvx_col(arr)); /* frees buffer, sets count/capacity to 0 */
}

static void test_da_str_init_with_then_push(struct cvxtest *t)
{
    struct da_str arr = da_str_init_with(4);

    da_str_push_back(cvx_col(arr), mkstr("hello"));
    da_str_push_back(cvx_col(arr), mkstr("world"));

    CVXCHECK(t, arr.count == 2);
    CVXCHECK(t, strcmp(arr.buffer[0], "hello") == 0);
    CVXCHECK(t, strcmp(arr.buffer[1], "world") == 0);

    da_str_clear(cvx_col(arr));
}

/* ---- _new ---- */

static void test_da_str_new(struct cvxtest *t)
{
    cvx_container *col = da_str_new();

    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->tag == 91);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    struct da_str *arr = (struct da_str *)col;
    CVXCHECK(t, arr->buffer == NULL);
    CVXCHECK(t, arr->capacity == 0);
    CVXCHECK(t, arr->count == 0);

    da_str_drop(col);
}

static void test_da_str_new_push_drop(struct cvxtest *t)
{
    cvx_container *col = da_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    da_str_push_back(col, mkstr("one"));
    da_str_push_back(col, mkstr("two"));
    da_str_push_back(col, mkstr("three"));

    CVXCHECK(t, da_str_count(col) == 3);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    /* _drop must call V_DROP on each element — valgrind verifies no leak */
    da_str_drop(col);
}

/* ---- _new_with ---- */

static void test_da_str_new_with(struct cvxtest *t)
{
    cvx_container *col = da_str_new_with(8);

    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->tag == 91);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    struct da_str *arr = (struct da_str *)col;
    CVXCHECK(t, arr->capacity == 8);
    CVXCHECK(t, arr->count == 0);

    da_str_drop(col);
}

static void test_da_str_new_with_push_drop(struct cvxtest *t)
{
    cvx_container *col = da_str_new_with(2);
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    /* Push beyond initial capacity to trigger grow */
    da_str_push_back(col, mkstr("a"));
    da_str_push_back(col, mkstr("b"));
    da_str_push_back(col, mkstr("c")); /* triggers realloc */

    CVXCHECK(t, da_str_count(col) == 3);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    da_str_drop(col);
}

/* ---- _copy ---- */

static void test_da_str_copy_empty(struct cvxtest *t)
{
    struct da_str orig = da_str_init();
    struct da_str copy = da_str_copy(&orig);

    CVXCHECK(t, copy.count == 0);
    CVXCHECK(t, copy.buffer == NULL);
    CVXCHECK(t, cvx_col(copy)->flag == CVX_FLAG_OK);

    /* No elements to free; buffer is NULL */
}

static void test_da_str_copy_deep(struct cvxtest *t)
{
    struct da_str orig = da_str_init_with(4);
    da_str_push_back(cvx_col(orig), mkstr("foo"));
    da_str_push_back(cvx_col(orig), mkstr("bar"));

    struct da_str copy = da_str_copy(&orig);

    CVXCHECK(t, copy.count == 2);
    CVXCHECK(t, cvx_col(copy)->flag == CVX_FLAG_OK);

    /* Deep copy: pointers must differ */
    CVXCHECK(t, copy.buffer[0] != orig.buffer[0]);
    CVXCHECK(t, copy.buffer[1] != orig.buffer[1]);

    /* Values must match */
    CVXCHECK(t, strcmp(copy.buffer[0], "foo") == 0);
    CVXCHECK(t, strcmp(copy.buffer[1], "bar") == 0);

    da_str_clear(cvx_col(orig));
    da_str_clear(cvx_col(copy));
}

static void test_da_str_copy_independence(struct cvxtest *t)
{
    struct da_str orig = da_str_init_with(4);
    da_str_push_back(cvx_col(orig), mkstr("alpha"));

    struct da_str copy = da_str_copy(&orig);

    /* Mutate the copy; original must be unaffected */
    free(copy.buffer[0]);
    copy.buffer[0] = strdup("beta");

    CVXCHECK(t, strcmp(orig.buffer[0], "alpha") == 0);
    CVXCHECK(t, strcmp(copy.buffer[0], "beta") == 0);

    da_str_clear(cvx_col(orig));
    da_str_clear(cvx_col(copy));
}

/* ---- _clone ---- */

static void test_da_str_clone_empty(struct cvxtest *t)
{
    cvx_container *col = da_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    cvx_container *clone = da_str_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        da_str_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CVXCHECK(t, da_str_count(clone) == 0);

    da_str_drop(col);
    da_str_drop(clone);
}

static void test_da_str_clone_deep(struct cvxtest *t)
{
    cvx_container *col = da_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    da_str_push_back(col, mkstr("x"));
    da_str_push_back(col, mkstr("y"));
    da_str_push_back(col, mkstr("z"));

    cvx_container *clone = da_str_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        da_str_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CVXCHECK(t, da_str_count(clone) == 3);

    struct da_str *orig_arr = (struct da_str *)col;
    struct da_str *clone_arr = (struct da_str *)clone;

    /* Deep copy: pointers must differ */
    CVXCHECK(t, clone_arr->buffer[0] != orig_arr->buffer[0]);
    CVXCHECK(t, clone_arr->buffer[1] != orig_arr->buffer[1]);
    CVXCHECK(t, clone_arr->buffer[2] != orig_arr->buffer[2]);

    /* Values must match */
    CVXCHECK(t, strcmp(clone_arr->buffer[0], "x") == 0);
    CVXCHECK(t, strcmp(clone_arr->buffer[1], "y") == 0);
    CVXCHECK(t, strcmp(clone_arr->buffer[2], "z") == 0);

    da_str_drop(col);
    da_str_drop(clone);
}

static void test_da_str_clone_independence(struct cvxtest *t)
{
    cvx_container *col = da_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    da_str_push_back(col, mkstr("original"));

    cvx_container *clone = da_str_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        da_str_drop(col);
        return;
    }

    /* Drop the original; clone must still be valid */
    da_str_drop(col);

    struct da_str *clone_arr = (struct da_str *)clone;
    CVXCHECK(t, strcmp(clone_arr->buffer[0], "original") == 0);

    da_str_drop(clone);
}

/* ---- _clear resets and allows reuse ---- */

static void test_da_str_clear_drops_elements(struct cvxtest *t)
{
    cvx_container *col = da_str_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    da_str_push_back(col, mkstr("p"));
    da_str_push_back(col, mkstr("q"));

    da_str_clear(col); /* must call V_DROP on both strings */

    CVXCHECK(t, da_str_count(col) == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    /* Can still push after clear */
    da_str_push_back(col, mkstr("r"));
    CVXCHECK(t, da_str_count(col) == 1);

    da_str_drop(col);
}

/* ---- runner ---- */

static int run_dynamic_array_init_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dynamic_array (init, char*)\n");

    CVXRUN(&t, test_da_str_init);
    CVXRUN(&t, test_da_str_init_with);
    CVXRUN(&t, test_da_str_init_with_then_push);

    CVXRUN(&t, test_da_str_new);
    CVXRUN(&t, test_da_str_new_push_drop);

    CVXRUN(&t, test_da_str_new_with);
    CVXRUN(&t, test_da_str_new_with_push_drop);

    CVXRUN(&t, test_da_str_copy_empty);
    CVXRUN(&t, test_da_str_copy_deep);
    CVXRUN(&t, test_da_str_copy_independence);

    CVXRUN(&t, test_da_str_clone_empty);
    CVXRUN(&t, test_da_str_clone_deep);
    CVXRUN(&t, test_da_str_clone_independence);

    CVXRUN(&t, test_da_str_clear_drops_elements);

    return CVXSUMMARY(&t);
}

#endif /* DYNAMIC_ARRAY_INIT_TESTS_H */
