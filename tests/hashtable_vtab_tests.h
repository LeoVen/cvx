#ifndef HASHTABLE_VTAB_TESTS_H
#define HASHTABLE_VTAB_TESTS_H

#include "cvxtest.h"

#include "implementations.h"

// K=int and V=int both use int_copy / int_drop, so copy and drop counters
// accumulate calls from both K and V together.
// Per filled entry: 1 K copy + 1 V copy = 2 copy calls, etc.

/* ---- copy called on _copy ---- */

static void test_ht_int_vtab_copy_on_copy(struct cvxtest *t)
{
    struct ht_int_int orig = ht_int_init(ht_int_vtabk_full, ht_int_vtabv_full);
    ht_int_insert(&orig, 1, 10);
    ht_int_insert(&orig, 2, 20);

    CVX_TEST_COUNTER_COPY_RESET();
    struct ht_int_int copy = ht_int_copy(&orig);

    // 2 entries × (1 K + 1 V) = 4 copy calls
    CVX_TEST_COUNTER_COPY(t, 4);
    CVXCHECK(t, copy.count == 2);

    ht_int_clear(&orig);
    ht_int_clear(&copy);
}

/* ---- copy called on _clone ---- */

static void test_ht_int_vtab_copy_on_clone(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk_full, ht_int_vtabv_full, 0);
    ht_int_insert(col, 10, 100);
    ht_int_insert(col, 20, 200);

    CVX_TEST_COUNTER_COPY_RESET();
    struct ht_int_int *clone = ht_int_clone(col);

    // 2 entries × (1 K + 1 V) = 4 copy calls
    CVX_TEST_COUNTER_COPY(t, 4);
    CVXCHECK(t, ht_int_count(clone) == 2);

    ht_int_drop(col);
    ht_int_drop(clone);
}

/* ---- null copy does not crash ---- */

static void test_ht_int_vtab_null_copy_no_crash(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    ht_int_insert(col, 1, 10);

    struct ht_int_int *clone = ht_int_clone(col);
    CVXCHECK(t, clone != NULL);
    CVXCHECK(t, ht_int_count(clone) == 1);
    CVXCHECK(t, ht_int_get(clone, 1) == 10);

    ht_int_drop(col);
    ht_int_drop(clone);
}

/* ---- drop called on _drop ---- */

static void test_ht_int_vtab_drop_on_drop(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk_full, ht_int_vtabv_full, 0);
    ht_int_insert(col, 1, 10);
    ht_int_insert(col, 2, 20);

    CVX_TEST_COUNTER_DROP_RESET();
    ht_int_drop(col);

    // 2 entries × (1 K + 1 V) = 4 drop calls
    CVX_TEST_COUNTER_DROP(t, 4);
}

/* ---- drop called on _clear ---- */

static void test_ht_int_vtab_drop_on_clear(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk_full, ht_int_vtabv_full, 0);
    ht_int_insert(col, 1, 10);
    ht_int_insert(col, 2, 20);

    CVX_TEST_COUNTER_DROP_RESET();
    ht_int_clear(col);

    // 2 entries × (1 K + 1 V) = 4 drop calls
    CVX_TEST_COUNTER_DROP(t, 4);

    ht_int_drop(col);
}

/* ---- drop called on _remove (no out-param) ---- */

static void test_ht_int_vtab_drop_on_remove(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk_full, ht_int_vtabv_full, 0);
    ht_int_insert(col, 5, 50);

    CVX_TEST_COUNTER_DROP_RESET();
    ht_int_remove(col, 5, NULL);

    // 1 K drop + 1 V drop = 2 drop calls
    CVX_TEST_COUNTER_DROP(t, 2);

    ht_int_drop(col);
}

/* ---- no drop on _remove when out-param is provided ---- */

static void test_ht_int_vtab_no_val_drop_when_out(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk_full, ht_int_vtabv_full, 0);
    ht_int_insert(col, 5, 50);

    CVX_TEST_COUNTER_DROP_RESET();
    int out = 0;
    ht_int_remove(col, 5, &out);

    // key drop only (val ownership transferred to caller)
    CVX_TEST_COUNTER_DROP(t, 1);
    CVXCHECK(t, out == 50);

    ht_int_drop(col);
}

/* ---- null vtab paths do not crash ---- */

static void test_ht_int_vtab_null_vtabv_no_crash(struct cvxtest *t)
{
    (void)t;
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    ht_int_insert(col, 1, 10);

    ht_int_clear(col); // no drop on values: must not crash
    ht_int_drop(col);
}

/* ---- runner ---- */

static struct cvxresult run_hashtable_vtab_tests(void)
{
    struct cvxtest t = { 0 };

    printf("hashtable (vtab)\n");

    CVXRUN(&t, test_ht_int_vtab_copy_on_copy);
    CVXRUN(&t, test_ht_int_vtab_copy_on_clone);
    CVXRUN(&t, test_ht_int_vtab_null_copy_no_crash);

    CVXRUN(&t, test_ht_int_vtab_drop_on_drop);
    CVXRUN(&t, test_ht_int_vtab_drop_on_clear);
    CVXRUN(&t, test_ht_int_vtab_drop_on_remove);
    CVXRUN(&t, test_ht_int_vtab_no_val_drop_when_out);

    CVXRUN(&t, test_ht_int_vtab_null_vtabv_no_crash);

    return CVXSUMMARY(&t);
}

#endif /* HASHTABLE_VTAB_TESTS_H */
