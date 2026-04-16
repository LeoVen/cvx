#ifndef INTERVAL_MAP_VTAB_TESTS_H
#define INTERVAL_MAP_VTAB_TESTS_H

#include "cvxtest.h"

#include "implementations.h"

// K=int and V=int both use int_copy / int_drop, so copy and drop counters
// accumulate calls from both K boundaries and V values together.
// Per entry: 2 K calls (lo + hi) + 1 V call = 3 calls.

/* ---- copy called on _copy ---- */

static void test_im_int_vtab_copy_on_copy(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    struct imap_int_int orig = im_int_init(im_int_vtabk_full, im_int_vtabv_full);
    im_int_add(&orig, 1, 5, 10);
    im_int_add(&orig, 10, 15, 20);

    CVX_TEST_COUNTER_COPY_RESET(); // reset after puts (no copy on put)
    struct imap_int_int copy = im_int_copy(&orig);

    // 2 entries × (2 K + 1 V) = 6 copy calls.
    CVX_TEST_COUNTER_COPY(t, 6);
    CVXCHECK(t, copy.count == 2);

    im_int_clear(&orig);
    im_int_clear(&copy);
}

/* ---- copy called on _clone ---- */

static void test_im_int_vtab_copy_on_clone(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    struct imap_int_int *col = im_int_new_with(im_int_vtabk_full, im_int_vtabv_full);
    im_int_add(col, 1, 5, 10);
    im_int_add(col, 10, 15, 20);

    CVX_TEST_COUNTER_COPY_RESET();
    struct imap_int_int *clone = im_int_clone(col);

    // 2 entries × (2 K + 1 V) = 6 copy calls.
    CVX_TEST_COUNTER_COPY(t, 6);
    CVXCHECK(t, im_int_count(clone) == 2);

    im_int_drop(col);
    im_int_drop(clone);
}

/* ---- null copy does not crash ---- */

static void test_im_int_vtab_null_copy_no_crash(struct cvxtest *t)
{
    struct imap_int_int *col = im_int_new_with(im_int_vtabk, NULL);
    im_int_add(col, 1, 5, 10);

    struct imap_int_int *clone = im_int_clone(col);
    CVXCHECK(t, clone != NULL);
    CVXCHECK(t, im_int_count(clone) == 1);

    im_int_drop(col);
    im_int_drop(clone);
}

/* ---- drop called on _drop ---- */

static void test_im_int_vtab_drop_on_drop(struct cvxtest *t)
{
    struct imap_int_int *col = im_int_new_with(im_int_vtabk_full, im_int_vtabv_full);
    im_int_add(col, 1, 5, 10);
    im_int_add(col, 10, 15, 20);

    CVX_TEST_COUNTER_DROP_RESET();
    im_int_drop(col);

    // 2 entries × (2 K + 1 V) = 6 drop calls.
    CVX_TEST_COUNTER_DROP(t, 6);
}

/* ---- drop called on _clear ---- */

static void test_im_int_vtab_drop_on_clear(struct cvxtest *t)
{
    struct imap_int_int *col = im_int_new_with(im_int_vtabk_full, im_int_vtabv_full);
    im_int_add(col, 1, 5, 10);
    im_int_add(col, 10, 15, 20);

    CVX_TEST_COUNTER_DROP_RESET();
    im_int_clear(col);

    // 2 entries × (2 K + 1 V) = 6 drop calls.
    CVX_TEST_COUNTER_DROP(t, 6);

    im_int_drop(col);
}

/* ---- drop called when _add overwrites ---- */

static void test_im_int_vtab_drop_on_add_overwrite(struct cvxtest *t)
{
    struct imap_int_int *col = im_int_new_with(im_int_vtabk_full, im_int_vtabv_full);
    im_int_add(col, 1, 10, 10);

    CVX_TEST_COUNTER_DROP_RESET();
    im_int_add(col, 1, 10, 20); // exact overwrite

    // Drops buffer[0].lo(1), buffer[0].hi(10), buffer[0].val(10) = 3 drops.
    CVX_TEST_COUNTER_DROP(t, 3);
    CVXCHECK(t, im_int_count(col) == 1);
    CVXCHECK(t, im_int_get(col, 5) == 20);

    im_int_drop(col);
}

/* ---- drop + copy called when _remove splits ---- */

static void test_im_int_vtab_drop_on_remove_split(struct cvxtest *t)
{
    struct imap_int_int *col = im_int_new_with(im_int_vtabk_full, im_int_vtabv_full);
    im_int_add(col, 1, 10, 42);

    CVX_TEST_COUNTER_DROP_RESET();
    CVX_TEST_COUNTER_COPY_RESET();
    im_int_remove(col, 3, 7); // splits into [1,3)→42 and [7,10)→42

    // Residuals are built by copying K boundaries and V value before dropping.
    // Left residual:  copy(lo=1) + copy(hi_new=3) + copy(val=42) = 3 copies
    // Right residual: copy(lo_new=7) + copy(hi=10) + copy(val=42) = 3 copies
    // Total copies: 6.
    CVX_TEST_COUNTER_COPY(t, 6);

    // Drop the original entry: drop(lo=1) + drop(hi=10) + drop(val=42) = 3 drops.
    CVX_TEST_COUNTER_DROP(t, 3);

    CVXCHECK(t, im_int_count(col) == 2);

    im_int_drop(col);
}

/* ---- joining: same-value adjacent intervals merge on _add ---- */

static void test_im_int_vtab_join_on_add(struct cvxtest *t)
{
    // Use vtabk_full so boundary copies during absorb are counted.
    // Use vtabv_with_comp so joining is enabled (no vtabv->copy/drop).
    struct imap_int_int *col = im_int_new_with(im_int_vtabk_full, im_int_vtabv_with_comp);
    im_int_add(col, 1, 5, 10);
    im_int_add(col, 10, 15, 10); // same value, gap between them

    CVX_TEST_COUNTER_COPY_RESET();
    CVX_TEST_COUNTER_DROP_RESET();

    // Fill the gap: [5,10)→10.  Same value on both sides → merge to [1,15)→10.
    im_int_add(col, 5, 10, 10);

    // effective_lo absorbed from buffer[0].lo via vtabk->copy(1) → 1 copy
    // effective_hi absorbed from buffer[1].hi via vtabk->copy(15) → 1 copy
    // No vtabv->copy (vtabv_with_comp has no copy fn).
    CVX_TEST_COUNTER_COPY(t, 2);

    // Drop buffer[0]: vtabk->drop(lo=1) + vtabk->drop(hi=5) → 2 drops
    //                 vtabv->drop is NULL → 0 drops
    // Drop buffer[1]: vtabk->drop(lo=10) + vtabk->drop(hi=15) → 2 drops
    //                 vtabv->drop is NULL → 0 drops
    CVX_TEST_COUNTER_DROP(t, 4);

    CVXCHECK(t, im_int_count(col) == 1);
    CVXCHECK(t, im_int_get(col, 1) == 10);
    CVXCHECK(t, im_int_get(col, 14) == 10);
    CVXCHECK(t, im_int_contains_key(col, 15) == false);

    im_int_drop(col);
}

/* ---- null drop does not crash ---- */

static void test_im_int_vtab_null_drop_no_crash(struct cvxtest *t)
{
    struct imap_int_int *col = im_int_new_with(im_int_vtabk, NULL);
    im_int_add(col, 1, 5, 10);

    im_int_drop(col);

    CVXCHECK(t, true);
}

/* ---- runner ---- */

static struct cvxresult run_interval_map_vtab_tests(void)
{
    struct cvxtest t = { 0 };

    printf("interval_map (vtab)\n");

    CVXRUN(&t, test_im_int_vtab_copy_on_copy);
    CVXRUN(&t, test_im_int_vtab_copy_on_clone);
    CVXRUN(&t, test_im_int_vtab_null_copy_no_crash);

    CVXRUN(&t, test_im_int_vtab_drop_on_drop);
    CVXRUN(&t, test_im_int_vtab_drop_on_clear);
    CVXRUN(&t, test_im_int_vtab_drop_on_add_overwrite);
    CVXRUN(&t, test_im_int_vtab_drop_on_remove_split);
    CVXRUN(&t, test_im_int_vtab_join_on_add);
    CVXRUN(&t, test_im_int_vtab_null_drop_no_crash);

    return CVXSUMMARY(&t);
}

#endif /* INTERVAL_MAP_VTAB_TESTS_H */
