#ifndef INTERVAL_SET_TESTS_H
#define INTERVAL_SET_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

/* ---- init ---- */

static void test_is_int_init(struct cvxtest *t)
{
    struct iset_int s = is_int_init(is_int_vtabv_comp_only);
    cvx_container *col = cvx_col(s);

    CVXCHECK(t, col->tag == 55);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, s.count == 0);
    CVXCHECK(t, s.capacity == 0);
    CVXCHECK(t, s.buffer == NULL);
    CVXCHECK(t, s.vtabv == is_int_vtabv_comp_only);
}

static void test_is_int_init_null_vtabv(struct cvxtest *t)
{
    struct iset_int s = is_int_init(NULL);
    cvx_container *col = cvx_col(s);

    CVXCHECK(t, col->flag == CVX_FLAG_VTAB);
    CVXCHECK(t, col->tag == 0);
}

static void test_is_int_init_no_comp(struct cvxtest *t)
{
    struct iset_int_vtabv vtabv = { 0 };
    struct iset_int s = is_int_init(&vtabv);
    cvx_container *col = cvx_col(s);

    CVXCHECK(t, col->flag == CVX_FLAG_VTAB);
    CVXCHECK(t, col->tag == 0);
}

/* ---- new / new_with ---- */

static void test_is_int_new(struct cvxtest *t)
{
    cvx_container *col = is_int_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->tag == 55);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(col) == 0);

    is_int_drop(col);
}

static void test_is_int_new_with(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->tag == 55);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(col) == 0);
    CVXCHECK(t, ((struct iset_int *)col)->vtabv == is_int_vtabv_comp_only);

    is_int_drop(col);
}

/* ---- copy ---- */

static void test_is_int_copy_empty(struct cvxtest *t)
{
    struct iset_int orig = is_int_init(is_int_vtabv_comp_only);
    struct iset_int copy = is_int_copy(&orig);

    CVXCHECK(t, ((cvx_container *)&copy)->flag == CVX_FLAG_OK);
    CVXCHECK(t, copy.count == 0);
    CVXCHECK(t, copy.buffer == NULL);
}

static void test_is_int_copy_values(struct cvxtest *t)
{
    struct iset_int orig = is_int_init(is_int_vtabv_comp_only);
    cvx_container *col = cvx_col(orig);

    is_int_add(col, 1, 5);
    is_int_add(col, 10, 15);

    struct iset_int copy = is_int_copy(&orig);
    cvx_container *ccol = cvx_col(copy);

    CVXCHECK(t, copy.count == 2);
    CVXCHECK(t, copy.buffer != orig.buffer);
    CVXCHECK(t, is_int_contains(ccol, 3) == true);
    CVXCHECK(t, is_int_contains(ccol, 12) == true);
    CVXCHECK(t, is_int_contains(ccol, 7) == false);

    is_int_clear(col);
    is_int_clear(ccol);
}

/* ---- clone ---- */

static void test_is_int_clone_empty(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);
    cvx_container *clone = is_int_clone(col);

    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        is_int_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(clone) == 0);

    is_int_drop(col);
    is_int_drop(clone);
}

static void test_is_int_clone_values(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);
    is_int_add(col, 1, 5);
    is_int_add(col, 10, 15);

    cvx_container *clone = is_int_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        is_int_drop(col);
        return;
    }

    CVXCHECK(t, is_int_count(clone) == 2);
    CVXCHECK(t, is_int_contains(clone, 3) == true);
    CVXCHECK(t, is_int_contains(clone, 12) == true);
    CVXCHECK(t, is_int_contains(clone, 7) == false);

    // Clones must be independent.
    is_int_add(col, 6, 9);
    CVXCHECK(t, is_int_count(clone) == 2);

    is_int_drop(col);
    is_int_drop(clone);
}

/* ---- clear ---- */

static void test_is_int_clear(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 5);
    is_int_add(col, 10, 15);
    is_int_clear(col);

    CVXCHECK(t, is_int_count(col) == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    // Container must be reusable after clear.
    is_int_add(col, 20, 25);
    CVXCHECK(t, is_int_count(col) == 1);

    is_int_drop(col);
}

/* ---- empty ---- */

static void test_is_int_empty(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    CVXCHECK(t, is_int_empty(col) == true);
    is_int_add(col, 1, 3);
    CVXCHECK(t, is_int_empty(col) == false);
    is_int_clear(col);
    CVXCHECK(t, is_int_empty(col) == true);

    is_int_drop(col);
}

/* ---- add: basic insert, no merge ---- */

static void test_is_int_add_single(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 3, 7);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(col) == 1);
    CVXCHECK(t, is_int_contains(col, 3) == true);
    CVXCHECK(t, is_int_contains(col, 6) == true);
    CVXCHECK(t, is_int_contains(col, 7) == false);

    is_int_drop(col);
}

static void test_is_int_add_two_disjoint(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 4);
    is_int_add(col, 8, 12);

    CVXCHECK(t, is_int_count(col) == 2);
    CVXCHECK(t, is_int_contains(col, 2) == true);
    CVXCHECK(t, is_int_contains(col, 10) == true);
    CVXCHECK(t, is_int_contains(col, 5) == false);
    CVXCHECK(t, is_int_contains(col, 7) == false);

    is_int_drop(col);
}

/* ---- add: merging overlapping intervals ---- */

static void test_is_int_add_overlap_merge(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 5);
    is_int_add(col, 3, 8);

    CVXCHECK(t, is_int_count(col) == 1);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    struct iset_int_iter it = is_int_iter_init_start(col);
    CVXCHECK(t, it.target->buffer[0].lo == 1);
    CVXCHECK(t, it.target->buffer[0].hi == 8);

    is_int_drop(col);
}

static void test_is_int_add_touching_merge(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 10, 12);
    is_int_add(col, 12, 15);

    // [10,12) and [12,15) touch: hi of first == lo of second → merged to [10,15).
    CVXCHECK(t, is_int_count(col) == 1);
    CVXCHECK(t, is_int_contains(col, 10) == true);
    CVXCHECK(t, is_int_contains(col, 14) == true);
    CVXCHECK(t, is_int_contains(col, 15) == false);

    is_int_drop(col);
}

static void test_is_int_add_merge_many(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 4);
    is_int_add(col, 5, 8);
    is_int_add(col, 9, 12);
    // Three separate intervals.
    CVXCHECK(t, is_int_count(col) == 3);

    // Adding [3, 10) should merge all three.
    is_int_add(col, 3, 10);
    CVXCHECK(t, is_int_count(col) == 1);
    CVXCHECK(t, is_int_contains(col, 1) == true);
    CVXCHECK(t, is_int_contains(col, 11) == true);
    CVXCHECK(t, is_int_contains(col, 0) == false);

    is_int_drop(col);
}

static void test_is_int_add_superset_interval(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 3, 7);
    // Add a strictly larger interval.
    is_int_add(col, 1, 10);

    CVXCHECK(t, is_int_count(col) == 1);
    CVXCHECK(t, is_int_contains(col, 1) == true);
    CVXCHECK(t, is_int_contains(col, 9) == true);
    CVXCHECK(t, is_int_contains(col, 0) == false);
    CVXCHECK(t, is_int_contains(col, 10) == false);

    is_int_drop(col);
}

static void test_is_int_add_subset_interval(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 10);
    // Add a strictly smaller interval (no-op on structure).
    is_int_add(col, 3, 7);

    CVXCHECK(t, is_int_count(col) == 1);
    CVXCHECK(t, is_int_contains(col, 1) == true);
    CVXCHECK(t, is_int_contains(col, 9) == true);

    is_int_drop(col);
}

static void test_is_int_add_invalid_interval(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    // lo == hi: empty interval, invalid for right-open.
    is_int_add(col, 5, 5);
    CVXCHECK(t, col->flag == CVX_FLAG_INVALID);
    CVXCHECK(t, is_int_count(col) == 0);

    // lo > hi: also invalid.
    is_int_add(col, 9, 3);
    CVXCHECK(t, col->flag == CVX_FLAG_INVALID);
    CVXCHECK(t, is_int_count(col) == 0);

    is_int_drop(col);
}

static void test_is_int_add_no_vtabv(struct cvxtest *t)
{
    cvx_container *col = is_int_new();

    is_int_add(col, 1, 5);
    CVXCHECK(t, col->flag == CVX_FLAG_VTAB);

    is_int_drop(col);
}

/* ---- remove ---- */

static void test_is_int_remove_exact(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 3, 7);
    is_int_remove(col, 3, 7);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(col) == 0);

    is_int_drop(col);
}

static void test_is_int_remove_split(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 10);
    is_int_remove(col, 3, 7);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(col) == 2);
    CVXCHECK(t, is_int_contains(col, 1) == true);
    CVXCHECK(t, is_int_contains(col, 2) == true);
    CVXCHECK(t, is_int_contains(col, 3) == false);
    CVXCHECK(t, is_int_contains(col, 6) == false);
    CVXCHECK(t, is_int_contains(col, 7) == true);
    CVXCHECK(t, is_int_contains(col, 9) == true);
    CVXCHECK(t, is_int_contains(col, 10) == false);

    is_int_drop(col);
}

static void test_is_int_remove_trim_left(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 10);
    is_int_remove(col, 1, 5);

    CVXCHECK(t, is_int_count(col) == 1);
    CVXCHECK(t, is_int_contains(col, 1) == false);
    CVXCHECK(t, is_int_contains(col, 4) == false);
    CVXCHECK(t, is_int_contains(col, 5) == true);
    CVXCHECK(t, is_int_contains(col, 9) == true);

    is_int_drop(col);
}

static void test_is_int_remove_trim_right(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 10);
    is_int_remove(col, 6, 10);

    CVXCHECK(t, is_int_count(col) == 1);
    CVXCHECK(t, is_int_contains(col, 5) == true);
    CVXCHECK(t, is_int_contains(col, 6) == false);
    CVXCHECK(t, is_int_contains(col, 9) == false);

    is_int_drop(col);
}

static void test_is_int_remove_spanning_multiple(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 5);
    is_int_add(col, 8, 12);
    is_int_add(col, 15, 20);
    is_int_remove(col, 3, 17);

    CVXCHECK(t, is_int_count(col) == 2);
    CVXCHECK(t, is_int_contains(col, 1) == true);
    CVXCHECK(t, is_int_contains(col, 2) == true);
    CVXCHECK(t, is_int_contains(col, 3) == false);
    CVXCHECK(t, is_int_contains(col, 16) == false);
    CVXCHECK(t, is_int_contains(col, 17) == true);
    CVXCHECK(t, is_int_contains(col, 19) == true);

    is_int_drop(col);
}

static void test_is_int_remove_no_overlap(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 5);
    is_int_remove(col, 10, 15);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(col) == 1);
    CVXCHECK(t, is_int_contains(col, 3) == true);

    is_int_drop(col);
}

static void test_is_int_remove_touching_not_removed(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 3);
    is_int_add(col, 7, 10);
    // Remove [3, 7): touches [1,3) at its hi and [7,10) at its lo.
    // Right-open semantics: these do NOT overlap, so nothing changes.
    is_int_remove(col, 3, 7);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(col) == 2);
    CVXCHECK(t, is_int_contains(col, 2) == true);
    CVXCHECK(t, is_int_contains(col, 3) == false);
    CVXCHECK(t, is_int_contains(col, 7) == true);

    is_int_drop(col);
}

static void test_is_int_remove_empty_range(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 5);
    is_int_remove(col, 3, 3);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, is_int_count(col) == 1);

    is_int_drop(col);
}

/* ---- contains ---- */

static void test_is_int_contains_in_interval(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 5, 10);
    CVXCHECK(t, is_int_contains(col, 5) == true);
    CVXCHECK(t, is_int_contains(col, 9) == true);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    is_int_drop(col);
}

static void test_is_int_contains_boundary_excluded(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 5, 10);
    CVXCHECK(t, is_int_contains(col, 10) == false);

    is_int_drop(col);
}

static void test_is_int_contains_between_intervals(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 3);
    is_int_add(col, 7, 10);
    CVXCHECK(t, is_int_contains(col, 5) == false);

    is_int_drop(col);
}

static void test_is_int_contains_empty_set(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    CVXCHECK(t, is_int_contains(col, 0) == false);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    is_int_drop(col);
}

/* ---- contains_interval ---- */

static void test_is_int_contains_interval_full(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 10);
    CVXCHECK(t, is_int_contains_interval(col, 3, 7) == true);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    is_int_drop(col);
}

static void test_is_int_contains_interval_partial(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 5);
    is_int_add(col, 8, 12);
    CVXCHECK(t, is_int_contains_interval(col, 3, 10) == false);

    is_int_drop(col);
}

static void test_is_int_contains_interval_empty(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 10);
    CVXCHECK(t, is_int_contains_interval(col, 5, 5) == false);

    is_int_drop(col);
}

/* ---- overlaps ---- */

static void test_is_int_overlaps_yes(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 5);
    CVXCHECK(t, is_int_overlaps(col, 3, 8) == true);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    is_int_drop(col);
}

static void test_is_int_overlaps_no(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 5);
    CVXCHECK(t, is_int_overlaps(col, 6, 10) == false);

    is_int_drop(col);
}

static void test_is_int_overlaps_touching_not_overlap(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    is_int_add(col, 1, 5);
    // [5, 8) touches [1,5) but does NOT overlap (right-open semantics).
    CVXCHECK(t, is_int_overlaps(col, 5, 8) == false);

    is_int_drop(col);
}

static void test_is_int_overlaps_empty_set(struct cvxtest *t)
{
    cvx_container *col = is_int_new_with(is_int_vtabv_comp_only);

    CVXCHECK(t, is_int_overlaps(col, 1, 5) == false);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    is_int_drop(col);
}

/* ---- runner ---- */

static int run_interval_set_tests(void)
{
    struct cvxtest t = { 0 };

    printf("interval_set\n");

    CVXRUN(&t, test_is_int_init);
    CVXRUN(&t, test_is_int_init_null_vtabv);
    CVXRUN(&t, test_is_int_init_no_comp);

    CVXRUN(&t, test_is_int_copy_empty);
    CVXRUN(&t, test_is_int_copy_values);

    CVXRUN(&t, test_is_int_new);
    CVXRUN(&t, test_is_int_new_with);

    CVXRUN(&t, test_is_int_clone_empty);
    CVXRUN(&t, test_is_int_clone_values);

    CVXRUN(&t, test_is_int_clear);

    CVXRUN(&t, test_is_int_empty);

    CVXRUN(&t, test_is_int_add_single);
    CVXRUN(&t, test_is_int_add_two_disjoint);
    CVXRUN(&t, test_is_int_add_overlap_merge);
    CVXRUN(&t, test_is_int_add_touching_merge);
    CVXRUN(&t, test_is_int_add_merge_many);
    CVXRUN(&t, test_is_int_add_superset_interval);
    CVXRUN(&t, test_is_int_add_subset_interval);
    CVXRUN(&t, test_is_int_add_invalid_interval);
    CVXRUN(&t, test_is_int_add_no_vtabv);

    CVXRUN(&t, test_is_int_remove_exact);
    CVXRUN(&t, test_is_int_remove_split);
    CVXRUN(&t, test_is_int_remove_trim_left);
    CVXRUN(&t, test_is_int_remove_trim_right);
    CVXRUN(&t, test_is_int_remove_spanning_multiple);
    CVXRUN(&t, test_is_int_remove_no_overlap);
    CVXRUN(&t, test_is_int_remove_touching_not_removed);
    CVXRUN(&t, test_is_int_remove_empty_range);

    CVXRUN(&t, test_is_int_contains_in_interval);
    CVXRUN(&t, test_is_int_contains_boundary_excluded);
    CVXRUN(&t, test_is_int_contains_between_intervals);
    CVXRUN(&t, test_is_int_contains_empty_set);

    CVXRUN(&t, test_is_int_contains_interval_full);
    CVXRUN(&t, test_is_int_contains_interval_partial);
    CVXRUN(&t, test_is_int_contains_interval_empty);

    CVXRUN(&t, test_is_int_overlaps_yes);
    CVXRUN(&t, test_is_int_overlaps_no);
    CVXRUN(&t, test_is_int_overlaps_touching_not_overlap);
    CVXRUN(&t, test_is_int_overlaps_empty_set);

    return CVXSUMMARY(&t);
}

#endif /* INTERVAL_SET_TESTS_H */
