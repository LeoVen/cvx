#ifndef HASHTABLE_TESTS_H
#define HASHTABLE_TESTS_H

#include "cvx/flags.h"
#include "tests/alloc.h"
#include "tests/cvxtest.h"
#include "tests/cvxtestutils.h"
#include "tests/implementations.h"

static void test_ht_int_init(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    CVXCHECK(t, col.super.tag == 22);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, col.count == 0);
    CVXCHECK(t, col.capacity == 0);
    CVXCHECK(t, col.buffer == NULL);
    CVXCHECK(t, col.vtabk == ht_int_vtabk);
    CVXCHECK(t, col.vtabv == NULL);
    CVXCHECK(t, col.load == 0.7);
    // null vtabk
    struct ht_int_int bad;
    ht_int_init(&bad, NULL, NULL, 0);
    CVXCHECK(t, bad.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, bad.super.tag == 0);
    // vtabk missing hash
    struct ht_int_int_vtabk no_hash = { .comp = int_comp };
    ht_int_init(&bad, &no_hash, NULL, 0);
    CVXCHECK(t, bad.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, bad.super.tag == 0);
    // vtabk missing comp
    struct ht_int_int_vtabk no_comp = { .hash = int_hash };
    ht_int_init(&bad, &no_comp, NULL, 0);
    CVXCHECK(t, bad.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, bad.super.tag == 0);
}

static void test_ht_int_init_with_capacity(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 10);
    CVXCHECK(t, col.super.tag == 22);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, col.count == 0);
    CVXCHECK(t, col.capacity >= 10);
    CVXCHECK(t, col.buffer != NULL);
    ht_int_drop(&col);
    // zero capacity: no allocation
    struct ht_int_int col2;
    ht_int_init(&col2, ht_int_vtabk, NULL, 0);
    CVXCHECK(t, col2.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, col2.capacity == 0);
    CVXCHECK(t, col2.buffer == NULL);
    // alloc failure
    CVX_MALLOC_FAIL_NEXT();
    struct ht_int_int col3;
    ht_int_init(&col3, ht_int_vtabk, NULL, 10);
    CVXCHECK(t, col3.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, col3.buffer == NULL);
    CVXCHECK(t, col3.capacity == 0);
    CVX_MALLOC_RESET();
}

static void test_ht_int_clone(struct cvxtest *t)
{
    // empty clone
    struct ht_int_int orig;
    ht_int_init(&orig, ht_int_vtabk, NULL, 0);
    struct ht_int_int clone;
    ht_int_clone(&orig, &clone);
    CVXCHECK(t, clone.super.tag == 22);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, clone.count == 0);
    CVXCHECK(t, clone.buffer == NULL);
    CVXCHECK(t, clone.vtabk == ht_int_vtabk);
    // nonempty clone
    ht_int_insert(&orig, 10, 100);
    ht_int_insert(&orig, 20, 200);
    struct ht_int_int clone2;
    ht_int_clone(&orig, &clone2);
    CVXCHECK(t, clone2.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, clone2.count == 2);
    CVXCHECK(t, ht_int_get(&clone2, 10) == 100);
    CVXCHECK(t, ht_int_get(&clone2, 20) == 200);
    CVXCHECK(t, clone2.buffer != orig.buffer);
    ht_int_drop(&orig);
    ht_int_drop(&clone2);
    // vtab clone callbacks
    struct ht_int_int orig2;
    ht_int_init(&orig2, ht_int_vtabk_full, ht_int_vtabv_full, 0);
    ht_int_insert(&orig2, 1, 10);
    ht_int_insert(&orig2, 2, 20);
    CVX_TEST_COUNTER_CLONE_RESET();
    struct ht_int_int clone3;
    ht_int_clone(&orig2, &clone3);
    // 2 entries × (1 K + 1 V) = 4 clone calls
    CVX_TEST_COUNTER_CLONE(t, 4);
    CVXCHECK(t, clone3.count == 2);
    ht_int_drop(&orig2);
    ht_int_drop(&clone3);
    // null clone does not crash
    struct ht_int_int orig3;
    ht_int_init(&orig3, ht_int_vtabk, NULL, 0);
    ht_int_insert(&orig3, 1, 10);
    struct ht_int_int clone4;
    ht_int_clone(&orig3, &clone4);
    CVXCHECK(t, clone4.count == 1);
    CVXCHECK(t, ht_int_get(&clone4, 1) == 10);
    ht_int_drop(&orig3);
    ht_int_drop(&clone4);
    // alloc failure
    struct ht_int_int orig4;
    ht_int_init(&orig4, ht_int_vtabk, NULL, 0);
    ht_int_insert(&orig4, 1, 10);
    CVX_MALLOC_FAIL_NEXT();
    struct ht_int_int clone5;
    ht_int_clone(&orig4, &clone5);
    CVXCHECK(t, clone5.super.flag == CVX_FLAG_ALLOC);
    CVX_MALLOC_RESET();
    ht_int_drop(&orig4);
    // guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    struct ht_int_int dummy = { 0 };
    ht_int__proxy_clone(col, (cvx_container *)&dummy);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_drop(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    ht_int_drop(&col); // must not crash, empty
    struct ht_int_int col2;
    ht_int_init(&col2, ht_int_vtabk, NULL, 0);
    ht_int_insert(&col2, 1, 10);
    ht_int_insert(&col2, 2, 20);
    ht_int_drop(&col2); // must not crash, nonempty
    // vtab drop callbacks
    struct ht_int_int col3;
    ht_int_init(&col3, ht_int_vtabk_full, ht_int_vtabv_full, 0);
    ht_int_insert(&col3, 1, 10);
    ht_int_insert(&col3, 2, 20);
    CVX_TEST_COUNTER_DROP_RESET();
    ht_int_drop(&col3);
    // 2 entries × (1 K + 1 V) = 4 drop calls
    CVX_TEST_COUNTER_DROP(t, 4);
    (void)t;
}

static void test_ht_int_flag(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    CVXCHECK(t, ht_int_flag(&col) == CVX_FLAG_OK);
}

static void test_ht_int_count(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    CVXCHECK(t, ht_int_count(&col) == 0);

    ht_int_insert(&col, 1, 10);
    CVXCHECK(t, ht_int_count(&col) == 1);

    ht_int_drop(&col);

    // guard
    MAKE_INVALID_CONTAINER(c);
    size_t res = ht_int__proxy_count(c);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, c->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_capacity(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    CVXCHECK(t, ht_int_capacity(&col) == 0);

    ht_int_insert(&col, 1, 10);
    CVXCHECK(t, ht_int_capacity(&col) > 0);

    ht_int_drop(&col);

    // guard
    MAKE_INVALID_CONTAINER(c);
    size_t res = ht_int__proxy_capacity(c);
    CVXCHECK(t, res == 0);
    CVXCHECK(t, c->flag == CVX_FLAG_WRONG_TAG);
}

static void test_ht_int_load(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    CVXCHECK(t, ht_int_load(&col) == 0.7);
}

static void test_ht_int_empty(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    CVXCHECK(t, ht_int_empty(&col) == true);

    ht_int_insert(&col, 1, 10);
    CVXCHECK(t, ht_int_empty(&col) == false);

    ht_int_drop(&col);

    // guard
    MAKE_INVALID_CONTAINER(c);
    bool res = ht_int__proxy_empty(c);
    CVXCHECK(t, res == false);
    CVXCHECK(t, c->flag == CVX_FLAG_WRONG_TAG);
}

/* ---- insert ---- */

static void test_ht_int_insert(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);

    bool ok = ht_int_insert(&col, 42, 100);
    CVXCHECK(t, ok == true);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, ht_int_count(&col) == 1);

    // multiple inserts
    for (int i = 0; i < 10; i++)
        ht_int_insert(&col, i * 100, i * 1000);
    CVXCHECK(t, ht_int_count(&col) == 11);

    // duplicate
    ok = ht_int_insert(&col, 42, 999);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_DUPLICATE);
    CVXCHECK(t, ht_int_get(&col, 42) == 100);

    // triggers resize (insert 60 total)
    struct ht_int_int col2;
    ht_int_init(&col2, ht_int_vtabk, NULL, 0);
    for (int i = 0; i < 60; i++)
        ht_int_insert(&col2, i, i);
    CVXCHECK(t, ht_int_count(&col2) == 60);
    CVXCHECK(t, col2.super.flag == CVX_FLAG_OK);
    for (int i = 0; i < 60; i++)
        CVXCHECK(t, ht_int_get(&col2, i) == i);
    ht_int_drop(&col2);

    // no vtab (no hash/comp)
    struct ht_int_int no_vtab;
    ht_int_init(&no_vtab, ht_int_vtabk, NULL, 0);
    no_vtab.vtabk = NULL;
    ok = ht_int_insert(&no_vtab, 1, 10);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, no_vtab.super.flag == CVX_FLAG_VTAB);

    // alloc failure on first insert (buffer malloc)
    struct ht_int_int col3;
    ht_int_init(&col3, ht_int_vtabk, NULL, 0);
    CVX_MALLOC_FAIL_NEXT();
    ok = ht_int_insert(&col3, 1, 10);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, col3.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, ht_int_count(&col3) == 0);
    CVX_MALLOC_RESET();

    // alloc failure on resize
    struct ht_int_int col4;
    ht_int_init(&col4, ht_int_vtabk, NULL, 0);
    for (int i = 0; i < 38; i++)
        ht_int_insert(&col4, i, i);
    CVXCHECK(t, ht_int_count(&col4) == 38);
    CVX_MALLOC_FAIL_NEXT();
    ok = ht_int_insert(&col4, 1000, 999);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, col4.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, ht_int_count(&col4) == 38);
    CVX_MALLOC_RESET();
    ht_int_drop(&col4);

    // guard
    MAKE_INVALID_CONTAINER(c);
    ok = ht_int__proxy_insert(c, 1, 10);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, c->flag == CVX_FLAG_WRONG_TAG);

    ht_int_drop(&col);
}

/* ---- update ---- */

static void test_ht_int_update(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    ht_int_insert(&col, 5, 50);

    int old = 0;
    bool ok = ht_int_update(&col, 5, 99, &old);
    CVXCHECK(t, ok == true);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, old == 50);
    CVXCHECK(t, ht_int_get(&col, 5) == 99);

    // miss
    ok = ht_int_update(&col, 999, 0, NULL);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_NOT_FOUND);

    // no vtab
    col.vtabk = NULL;
    ok = ht_int_update(&col, 5, 1, NULL);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_VTAB);
    col.vtabk = ht_int_vtabk;

    // guard
    MAKE_INVALID_CONTAINER(c);
    ok = ht_int__proxy_update(c, 1, 10, NULL);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, c->flag == CVX_FLAG_WRONG_TAG);

    ht_int_drop(&col);
}

/* ---- remove ---- */

static void test_ht_int_remove(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    ht_int_insert(&col, 7, 70);

    int out = 0;
    bool ok = ht_int_remove(&col, 7, &out);
    CVXCHECK(t, ok == true);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, out == 70);
    CVXCHECK(t, ht_int_count(&col) == 0);
    CVXCHECK(t, ht_int_contains(&col, 7) == false);

    // miss
    ht_int_insert(&col, 1, 10);
    ok = ht_int_remove(&col, 999, NULL);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_NOT_FOUND);
    CVXCHECK(t, ht_int_count(&col) == 1);

    // empty
    struct ht_int_int col2;
    ht_int_init(&col2, ht_int_vtabk, NULL, 0);
    ok = ht_int_remove(&col2, 1, NULL);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, col2.super.flag == CVX_FLAG_EMPTY);

    // tombstone: insert 10, remove middle, verify rest findable
    struct ht_int_int col3;
    ht_int_init(&col3, ht_int_vtabk, NULL, 0);
    for (int i = 0; i < 10; i++)
        ht_int_insert(&col3, i, i * 10);
    ht_int_remove(&col3, 5, NULL);
    CVXCHECK(t, ht_int_count(&col3) == 9);
    CVXCHECK(t, ht_int_contains(&col3, 5) == false);
    for (int i = 0; i < 10; i++)
    {
        if (i == 5)
            continue;
        CVXCHECK(t, ht_int_get(&col3, i) == i * 10);
    }
    ht_int_drop(&col3);

    // vtab drop on remove (no out-param)
    struct ht_int_int col4;
    ht_int_init(&col4, ht_int_vtabk_full, ht_int_vtabv_full, 0);
    ht_int_insert(&col4, 5, 50);
    CVX_TEST_COUNTER_DROP_RESET();
    ht_int_remove(&col4, 5, NULL);
    CVX_TEST_COUNTER_DROP(t, 2); // 1 K + 1 V
    ht_int_drop(&col4);

    // no drop on val when out-param provided
    struct ht_int_int col5;
    ht_int_init(&col5, ht_int_vtabk_full, ht_int_vtabv_full, 0);
    ht_int_insert(&col5, 5, 50);
    CVX_TEST_COUNTER_DROP_RESET();
    int val_out = 0;
    ht_int_remove(&col5, 5, &val_out);
    CVX_TEST_COUNTER_DROP(t, 1); // key drop only
    CVXCHECK(t, val_out == 50);
    ht_int_drop(&col5);

    // no vtab
    col.vtabk = NULL;
    ok = ht_int_remove(&col, 1, NULL);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_VTAB);
    col.vtabk = ht_int_vtabk;

    // guard
    MAKE_INVALID_CONTAINER(c);
    ok = ht_int__proxy_remove(c, 1, NULL);
    CVXCHECK(t, ok == false);
    CVXCHECK(t, c->flag == CVX_FLAG_WRONG_TAG);

    ht_int_drop(&col);
}

/* ---- get ---- */

static void test_ht_int_get(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    ht_int_insert(&col, 3, 33);

    int val = ht_int_get(&col, 3);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, val == 33);

    val = ht_int_get(&col, 999);
    CVXCHECK(t, col.super.flag == CVX_FLAG_NOT_FOUND);
    CVXCHECK(t, val == 0);

    // no vtab
    col.vtabk = NULL;
    val = ht_int_get(&col, 3);
    CVXCHECK(t, col.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, val == 0);
    col.vtabk = ht_int_vtabk;

    // guard
    MAKE_INVALID_CONTAINER(c);
    val = ht_int__proxy_get(c, 1);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, c->flag == CVX_FLAG_WRONG_TAG);

    ht_int_drop(&col);
}

/* ---- get_ref ---- */

static void test_ht_int_get_ref(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    ht_int_insert(&col, 8, 80);

    int *ref = ht_int_get_ref(&col, 8);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, ref != NULL);
    if (ref)
    {
        CVXCHECK(t, *ref == 80);
        *ref = 88;
        CVXCHECK(t, ht_int_get(&col, 8) == 88);
    }

    ref = ht_int_get_ref(&col, 999);
    CVXCHECK(t, col.super.flag == CVX_FLAG_NOT_FOUND);
    CVXCHECK(t, ref == NULL);

    ht_int_drop(&col);
}

/* ---- contains ---- */

static void test_ht_int_contains(struct cvxtest *t)
{
    struct ht_int_int col;
    ht_int_init(&col, ht_int_vtabk, NULL, 0);
    ht_int_insert(&col, 4, 40);

    CVXCHECK(t, ht_int_contains(&col, 4) == true);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, ht_int_contains(&col, 999) == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);

    // no vtab
    col.vtabk = NULL;
    bool found = ht_int_contains(&col, 4);
    CVXCHECK(t, found == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_VTAB);
    col.vtabk = ht_int_vtabk;

    // guard
    MAKE_INVALID_CONTAINER(c);
    found = ht_int__proxy_contains(c, 1);
    CVXCHECK(t, found == false);
    CVXCHECK(t, c->flag == CVX_FLAG_WRONG_TAG);

    ht_int_drop(&col);
}

static struct cvxresult run_hashtable_tests(void)
{
    struct cvxtest t = { 0 };

    printf("hashtable\n");

    CVXRUN(&t, test_ht_int_init);
    CVXRUN(&t, test_ht_int_init_with_capacity);

    CVXRUN(&t, test_ht_int_clone);
    CVXRUN(&t, test_ht_int_drop);

    CVXRUN(&t, test_ht_int_flag);
    CVXRUN(&t, test_ht_int_count);
    CVXRUN(&t, test_ht_int_capacity);
    CVXRUN(&t, test_ht_int_load);
    CVXRUN(&t, test_ht_int_empty);

    CVXRUN(&t, test_ht_int_insert);
    CVXRUN(&t, test_ht_int_update);
    CVXRUN(&t, test_ht_int_remove);
    CVXRUN(&t, test_ht_int_get);
    CVXRUN(&t, test_ht_int_get_ref);
    CVXRUN(&t, test_ht_int_contains);

    return CVXSUMMARY(&t);
}

#endif /* HASHTABLE_TESTS_H */
