#ifndef INTERVAL_SET_VTAB_TESTS_H
#define INTERVAL_SET_VTAB_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

/* ---- copy called on copy ---- */

static void test_is_vtabv_copy_on_copy(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    struct iset_int orig = is_int_init(is_int_vtabv_full);
    is_int_add(&orig, 1, 5);
    is_int_add(&orig, 10, 15);

    struct iset_int copy = is_int_copy(&orig);

    // 2 intervals × 2 bounds each = 4 copy calls.
    CVX_TEST_COUNTER_COPY(t, 4);
    CVXCHECK(t, copy.count == 2);

    is_int_clear(&orig);
    is_int_clear(&copy);
}

/* ---- copy called on clone ---- */

static void test_is_vtabv_copy_on_clone(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    struct iset_int *col = is_int_new_with(is_int_vtabv_full);
    is_int_add(col, 1, 5);
    is_int_add(col, 10, 15);

    struct iset_int *clone = is_int_clone(col);

    // 2 intervals × 2 bounds each = 4 copy calls.
    CVX_TEST_COUNTER_COPY(t, 4);
    CVXCHECK(t, is_int_count(clone) == 2);

    is_int_drop(col);
    is_int_drop(clone);
}

/* ---- null copy does not crash ---- */

static void test_is_vtabv_null_copy_no_crash(struct cvxtest *t)
{
    struct iset_int *col = is_int_new_with(is_int_vtabv_comp_only);
    is_int_add(col, 1, 5);

    struct iset_int *clone = is_int_clone(col);
    CVXCHECK(t, clone != NULL);
    CVXCHECK(t, is_int_count(clone) == 1);

    is_int_drop(col);
    is_int_drop(clone);
}

/* ---- drop called on drop ---- */

static void test_is_vtabv_drop_on_drop(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    struct iset_int *col = is_int_new_with(is_int_vtabv_full);
    is_int_add(col, 1, 5);
    is_int_add(col, 10, 15);

    is_int_drop(col);

    // 2 intervals × 2 bounds each = 4 drop calls.
    CVX_TEST_COUNTER_DROP(t, 4);
}

/* ---- drop called on clear ---- */

static void test_is_vtabv_drop_on_clear(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    struct iset_int *col = is_int_new_with(is_int_vtabv_full);
    is_int_add(col, 1, 5);
    is_int_add(col, 10, 15);

    is_int_clear(col);
    CVX_TEST_COUNTER_DROP(t, 4);

    is_int_drop(col);
}

/* ---- drop called when merging in _add ---- */

static void test_is_vtabv_drop_on_add_merge(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    struct iset_int *col = is_int_new_with(is_int_vtabv_full);
    is_int_add(col, 1, 5);
    is_int_add(col, 10, 15);
    // Two intervals, drop count = 0 so far.
    CVX_TEST_COUNTER_DROP(t, 0);

    // Add [3, 12): overlaps both. Merges into [1, 15).
    // Drops: buffer[0].hi(5), buffer[1].lo(10), buffer[1].hi(15 consumed by hi
    // comparison → existing 15 wins, so incoming 12 is NOT stored, but 15 was
    // already kept).
    // Specifically:
    //   merged_lo: comp(3, 1) > 0 → existing lo=1 wins, incoming 3 ignored (no drop since it came from caller by value)
    //   merged_hi: comp(12, 15) < 0 → existing hi=15 wins, incoming 12 ignored
    //   drop buffer[0].hi = 5  (inner boundary)
    //   drop buffer[1].lo = 10 (inner boundary)
    //   → 2 drop calls total
    CVX_TEST_COUNTER_DROP_RESET();
    is_int_add(col, 3, 12);
    CVX_TEST_COUNTER_DROP(t, 2);

    CVXCHECK(t, is_int_count(col) == 1);

    is_int_drop(col);
}

/* ---- drop called when removing ---- */

static void test_is_vtabv_drop_on_remove(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    struct iset_int *col = is_int_new_with(is_int_vtabv_full);
    is_int_add(col, 1, 10);
    CVX_TEST_COUNTER_DROP_RESET();

    // Remove [3, 7): splits [1,10) → [1,3) and [7,10).
    // The original entry [1,10) is dropped (drop lo=1, drop hi=10): 2 drops.
    // Residuals are stored with copies of the boundary values.
    is_int_remove(col, 3, 7);
    CVX_TEST_COUNTER_DROP(t, 2);

    CVXCHECK(t, is_int_count(col) == 2);

    is_int_drop(col);
}

/* ---- null drop does not crash ---- */

static void test_is_vtabv_null_drop_no_crash(struct cvxtest *t)
{
    struct iset_int *col = is_int_new_with(is_int_vtabv_comp_only);
    is_int_add(col, 1, 5);

    is_int_drop(col);

    CVXCHECK(t, true);
}

/* ---- runner ---- */

static int run_interval_set_vtab_tests(void)
{
    struct cvxtest t = { 0 };

    printf("interval_set (vtabv)\n");

    CVXRUN(&t, test_is_vtabv_copy_on_copy);
    CVXRUN(&t, test_is_vtabv_copy_on_clone);
    CVXRUN(&t, test_is_vtabv_null_copy_no_crash);

    CVXRUN(&t, test_is_vtabv_drop_on_drop);
    CVXRUN(&t, test_is_vtabv_drop_on_clear);
    CVXRUN(&t, test_is_vtabv_drop_on_add_merge);
    CVXRUN(&t, test_is_vtabv_drop_on_remove);
    CVXRUN(&t, test_is_vtabv_null_drop_no_crash);

    return CVXSUMMARY(&t);
}

#endif /* INTERVAL_SET_VTAB_TESTS_H */
