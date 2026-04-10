#ifndef INTERVAL_MAP_TESTS_H
#define INTERVAL_MAP_TESTS_H

#include "cvxtest.h"
#include "cvxtestutils.h"

#include "implementations.h"

/* ---- init ---- */

static void test_im_int_init(struct cvxtest *t)
{
    struct imap_int_int s = im_int_init(im_int_vtabk, NULL);
    cvx_container *col = cvx_col(s);

    CVXCHECK(t, col->tag == 44);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, s.count == 0);
    CVXCHECK(t, s.capacity == 0);
    CVXCHECK(t, s.buffer == NULL);
    CVXCHECK(t, s.vtabk == im_int_vtabk);
    CVXCHECK(t, s.vtabv == NULL);
}

static void test_im_int_init_null_vtabk(struct cvxtest *t)
{
    struct imap_int_int s = im_int_init(NULL, NULL);
    cvx_container *col = cvx_col(s);

    CVXCHECK(t, col->flag == CVX_FLAG_VTAB);
    CVXCHECK(t, col->tag == 0);
}

static void test_im_int_init_no_comp(struct cvxtest *t)
{
    struct imap_int_int_vtabk vtabk = { 0 };
    struct imap_int_int s = im_int_init(&vtabk, NULL);
    cvx_container *col = cvx_col(s);

    CVXCHECK(t, col->flag == CVX_FLAG_VTAB);
    CVXCHECK(t, col->tag == 0);
}

/* ---- new / new_with ---- */

static void test_im_int_new(struct cvxtest *t)
{
    cvx_container *col = im_int_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->tag == 44);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 0);

    im_int_drop(col);
}

static void test_im_int_new_with(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->tag == 44);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 0);
    CVXCHECK(t, ((struct imap_int_int *)col)->vtabk == im_int_vtabk);

    im_int_drop(col);
}

/* ---- copy ---- */

static void test_im_int_copy_empty(struct cvxtest *t)
{
    struct imap_int_int orig = im_int_init(im_int_vtabk, NULL);
    struct imap_int_int copy = im_int_copy(&orig);

    CVXCHECK(t, ((cvx_container *)&copy)->flag == CVX_FLAG_OK);
    CVXCHECK(t, copy.count == 0);
    CVXCHECK(t, copy.buffer == NULL);
}

static void test_im_int_copy_values(struct cvxtest *t)
{
    struct imap_int_int orig = im_int_init(im_int_vtabk, NULL);
    cvx_container *col = cvx_col(orig);

    im_int_add(col, 1, 5, 10);
    im_int_add(col, 10, 15, 20);

    struct imap_int_int copy = im_int_copy(&orig);
    cvx_container *ccol = cvx_col(copy);

    CVXCHECK(t, copy.count == 2);
    CVXCHECK(t, copy.buffer != orig.buffer);
    CVXCHECK(t, im_int_get(ccol, 3) == 10);
    CVXCHECK(t, im_int_get(ccol, 12) == 20);
    CVXCHECK(t, im_int_contains_key(ccol, 7) == false);

    im_int_clear(col);
    im_int_clear(ccol);
}

/* ---- clone ---- */

static void test_im_int_clone_empty(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);
    cvx_container *clone = im_int_clone(col);

    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        im_int_drop(col);
        return;
    }

    CVXCHECK(t, clone->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(clone) == 0);

    im_int_drop(col);
    im_int_drop(clone);
}

static void test_im_int_clone_values(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);
    im_int_add(col, 1, 5, 10);
    im_int_add(col, 10, 15, 20);

    cvx_container *clone = im_int_clone(col);
    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        im_int_drop(col);
        return;
    }

    CVXCHECK(t, im_int_count(clone) == 2);
    CVXCHECK(t, im_int_get(clone, 3) == 10);
    CVXCHECK(t, im_int_get(clone, 12) == 20);

    // Clones must be independent.
    im_int_add(col, 6, 9, 99);
    CVXCHECK(t, im_int_count(clone) == 2);

    im_int_drop(col);
    im_int_drop(clone);
}

/* ---- clear / empty ---- */

static void test_im_int_clear(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 5, 10);
    im_int_add(col, 10, 15, 20);
    im_int_clear(col);

    CVXCHECK(t, im_int_count(col) == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    // Container must be reusable after clear.
    im_int_add(col, 20, 25, 30);
    CVXCHECK(t, im_int_count(col) == 1);

    im_int_drop(col);
}

static void test_im_int_empty(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    CVXCHECK(t, im_int_empty(col) == true);
    im_int_add(col, 1, 3, 99);
    CVXCHECK(t, im_int_empty(col) == false);
    im_int_clear(col);
    CVXCHECK(t, im_int_empty(col) == true);

    im_int_drop(col);
}

/* ---- put: basic inserts ---- */

static void test_im_int_add_single(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 3, 7, 42);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 1);
    CVXCHECK(t, im_int_get(col, 3) == 42);
    CVXCHECK(t, im_int_get(col, 6) == 42);
    CVXCHECK(t, im_int_contains_key(col, 7) == false);

    im_int_drop(col);
}

static void test_im_int_add_two_disjoint(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 4, 10);
    im_int_add(col, 8, 12, 20);

    CVXCHECK(t, im_int_count(col) == 2);
    CVXCHECK(t, im_int_get(col, 2) == 10);
    CVXCHECK(t, im_int_get(col, 10) == 20);
    CVXCHECK(t, im_int_contains_key(col, 5) == false);

    im_int_drop(col);
}

static void test_im_int_add_overwrite_exact(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 5, 10, 10);
    im_int_add(col, 5, 10, 20);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 1);
    CVXCHECK(t, im_int_get(col, 7) == 20);

    im_int_drop(col);
}

static void test_im_int_add_partial_left_overlap(struct cvxtest *t)
{
    // [1,10)→10 then put [5,15)→20: produces [1,5)→10 and [5,15)→20.
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 10, 10);
    im_int_add(col, 5, 15, 20);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 2);
    CVXCHECK(t, im_int_get(col, 2) == 10);
    CVXCHECK(t, im_int_get(col, 5) == 20);
    CVXCHECK(t, im_int_get(col, 14) == 20);

    im_int_drop(col);
}

static void test_im_int_add_partial_right_overlap(struct cvxtest *t)
{
    // [10,20)→20 then put [5,15)→10: produces [5,15)→10 and [15,20)→20.
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 10, 20, 20);
    im_int_add(col, 5, 15, 10);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 2);
    CVXCHECK(t, im_int_get(col, 7) == 10);
    CVXCHECK(t, im_int_get(col, 14) == 10);
    CVXCHECK(t, im_int_get(col, 15) == 20);
    CVXCHECK(t, im_int_get(col, 19) == 20);

    im_int_drop(col);
}

static void test_im_int_add_full_containment(struct cvxtest *t)
{
    // [1,20)→10 then put [5,15)→20: produces [1,5)→10, [5,15)→20, [15,20)→10.
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 20, 10);
    im_int_add(col, 5, 15, 20);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 3);
    CVXCHECK(t, im_int_get(col, 2) == 10);
    CVXCHECK(t, im_int_get(col, 7) == 20);
    CVXCHECK(t, im_int_get(col, 17) == 10);

    im_int_drop(col);
}

static void test_im_int_add_spanning_multiple(struct cvxtest *t)
{
    // [1,5)→10, [8,12)→20, [15,20)→30; put [3,17)→50
    // → [1,3)→10, [3,17)→50, [17,20)→30.
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 5, 10);
    im_int_add(col, 8, 12, 20);
    im_int_add(col, 15, 20, 30);
    im_int_add(col, 3, 17, 50);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 3);
    CVXCHECK(t, im_int_get(col, 1) == 10);
    CVXCHECK(t, im_int_get(col, 2) == 10);
    CVXCHECK(t, im_int_contains_key(col, 3) == true);
    CVXCHECK(t, im_int_get(col, 10) == 50);
    CVXCHECK(t, im_int_get(col, 17) == 30);
    CVXCHECK(t, im_int_get(col, 19) == 30);

    im_int_drop(col);
}

static void test_im_int_add_adjacent_same_value_join(struct cvxtest *t)
{
    // vtabv->comp set: touching intervals with same value are merged.
    cvx_container *col = im_int_new_with(im_int_vtabk, im_int_vtabv_with_comp);

    im_int_add(col, 1, 5, 10);
    im_int_add(col, 5, 10, 10); // touches [1,5) with same value → merge to [1,10).

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 1);
    CVXCHECK(t, im_int_get(col, 1) == 10);
    CVXCHECK(t, im_int_get(col, 9) == 10);
    CVXCHECK(t, im_int_contains_key(col, 10) == false);

    im_int_drop(col);
}

static void test_im_int_add_adjacent_different_value_no_join(struct cvxtest *t)
{
    // vtabv->comp set but values differ: touching intervals must NOT merge.
    cvx_container *col = im_int_new_with(im_int_vtabk, im_int_vtabv_with_comp);

    im_int_add(col, 1, 5, 10);
    im_int_add(col, 5, 10, 20); // touches [1,5) but different value → keep separate.

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 2);
    CVXCHECK(t, im_int_get(col, 3) == 10);
    CVXCHECK(t, im_int_get(col, 7) == 20);

    im_int_drop(col);
}

static void test_im_int_add_invalid_range(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 5, 5, 1); // lo == hi → invalid
    CVXCHECK(t, col->flag == CVX_FLAG_INVALID);
    CVXCHECK(t, im_int_count(col) == 0);

    im_int_add(col, 9, 3, 1); // lo > hi → invalid
    CVXCHECK(t, col->flag == CVX_FLAG_INVALID);
    CVXCHECK(t, im_int_count(col) == 0);

    im_int_drop(col);
}

static void test_im_int_add_no_vtabk(struct cvxtest *t)
{
    cvx_container *col = im_int_new();

    im_int_add(col, 1, 5, 10);
    CVXCHECK(t, col->flag == CVX_FLAG_VTAB);

    im_int_drop(col);
}

/* ---- remove ---- */

static void test_im_int_remove_exact(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 3, 7, 10);
    im_int_remove(col, 3, 7);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 0);

    im_int_drop(col);
}

static void test_im_int_remove_split(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 10, 42);
    im_int_remove(col, 3, 7);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 2);
    CVXCHECK(t, im_int_get(col, 1) == 42);
    CVXCHECK(t, im_int_get(col, 2) == 42);
    CVXCHECK(t, im_int_contains_key(col, 3) == false);
    CVXCHECK(t, im_int_contains_key(col, 6) == false);
    CVXCHECK(t, im_int_get(col, 7) == 42);
    CVXCHECK(t, im_int_get(col, 9) == 42);

    im_int_drop(col);
}

static void test_im_int_remove_trim_left(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 10, 42);
    im_int_remove(col, 1, 5);

    CVXCHECK(t, im_int_count(col) == 1);
    CVXCHECK(t, im_int_contains_key(col, 1) == false);
    CVXCHECK(t, im_int_get(col, 5) == 42);
    CVXCHECK(t, im_int_get(col, 9) == 42);

    im_int_drop(col);
}

static void test_im_int_remove_trim_right(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 10, 42);
    im_int_remove(col, 6, 10);

    CVXCHECK(t, im_int_count(col) == 1);
    CVXCHECK(t, im_int_get(col, 5) == 42);
    CVXCHECK(t, im_int_contains_key(col, 6) == false);

    im_int_drop(col);
}

static void test_im_int_remove_spanning_multiple(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 5, 10);
    im_int_add(col, 8, 12, 20);
    im_int_add(col, 15, 20, 30);
    im_int_remove(col, 3, 17);

    CVXCHECK(t, im_int_count(col) == 2);
    CVXCHECK(t, im_int_get(col, 1) == 10);
    CVXCHECK(t, im_int_get(col, 2) == 10);
    CVXCHECK(t, im_int_contains_key(col, 3) == false);
    CVXCHECK(t, im_int_contains_key(col, 16) == false);
    CVXCHECK(t, im_int_get(col, 17) == 30);
    CVXCHECK(t, im_int_get(col, 19) == 30);

    im_int_drop(col);
}

static void test_im_int_remove_no_overlap(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 5, 10);
    im_int_remove(col, 10, 15);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 1);
    CVXCHECK(t, im_int_get(col, 3) == 10);

    im_int_drop(col);
}

static void test_im_int_remove_empty_range(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 5, 10);
    im_int_remove(col, 3, 3); // empty range → no-op

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, im_int_count(col) == 1);

    im_int_drop(col);
}

/* ---- get ---- */

static void test_im_int_get_covered_key(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 5, 10, 42);
    int val = im_int_get(col, 7);

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, val == 42);

    im_int_drop(col);
}

static void test_im_int_get_lo_boundary(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 5, 10, 42);
    int val = im_int_get(col, 5); // lo is included

    CVXCHECK(t, col->flag == CVX_FLAG_OK);
    CVXCHECK(t, val == 42);

    im_int_drop(col);
}

static void test_im_int_get_hi_boundary(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 5, 10, 42);
    im_int_get(col, 10); // hi is excluded → NOT_FOUND

    CVXCHECK(t, col->flag == CVX_FLAG_NOT_FOUND);

    im_int_drop(col);
}

static void test_im_int_get_gap(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 5, 10);
    im_int_add(col, 10, 15, 20);
    im_int_get(col, 7); // gap between entries → NOT_FOUND

    CVXCHECK(t, col->flag == CVX_FLAG_NOT_FOUND);

    im_int_drop(col);
}

static void test_im_int_get_empty(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_get(col, 5);
    CVXCHECK(t, col->flag == CVX_FLAG_NOT_FOUND);

    im_int_drop(col);
}

/* ---- contains_key ---- */

static void test_im_int_contains_key_yes(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 5, 10, 42);
    CVXCHECK(t, im_int_contains_key(col, 7) == true);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    im_int_drop(col);
}

static void test_im_int_contains_key_no(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 5, 10, 42);
    CVXCHECK(t, im_int_contains_key(col, 10) == false);
    CVXCHECK(t, im_int_contains_key(col, 3) == false);

    im_int_drop(col);
}

/* ---- contains_interval ---- */

static void test_im_int_contains_interval_full(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 10, 42);
    CVXCHECK(t, im_int_contains_interval(col, 3, 7) == true);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    im_int_drop(col);
}

static void test_im_int_contains_interval_partial(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 5, 10);
    im_int_add(col, 8, 12, 20);
    CVXCHECK(t, im_int_contains_interval(col, 3, 10) == false);

    im_int_drop(col);
}

static void test_im_int_contains_interval_empty(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 10, 42);
    CVXCHECK(t, im_int_contains_interval(col, 5, 5) == false);

    im_int_drop(col);
}

/* ---- overlaps ---- */

static void test_im_int_overlaps_yes(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 5, 42);
    CVXCHECK(t, im_int_overlaps(col, 3, 8) == true);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    im_int_drop(col);
}

static void test_im_int_overlaps_no(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 5, 42);
    CVXCHECK(t, im_int_overlaps(col, 6, 10) == false);

    im_int_drop(col);
}

static void test_im_int_overlaps_touching_not_overlap(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    im_int_add(col, 1, 5, 42);
    CVXCHECK(t, im_int_overlaps(col, 5, 8) == false);

    im_int_drop(col);
}

static void test_im_int_overlaps_empty(struct cvxtest *t)
{
    cvx_container *col = im_int_new_with(im_int_vtabk, NULL);

    CVXCHECK(t, im_int_overlaps(col, 1, 5) == false);
    CVXCHECK(t, col->flag == CVX_FLAG_OK);

    im_int_drop(col);
}

/* ---- runner ---- */

static int run_interval_map_tests(void)
{
    struct cvxtest t = { 0 };

    printf("interval_map\n");

    CVXRUN(&t, test_im_int_init);
    CVXRUN(&t, test_im_int_init_null_vtabk);
    CVXRUN(&t, test_im_int_init_no_comp);

    CVXRUN(&t, test_im_int_new);
    CVXRUN(&t, test_im_int_new_with);

    CVXRUN(&t, test_im_int_copy_empty);
    CVXRUN(&t, test_im_int_copy_values);

    CVXRUN(&t, test_im_int_clone_empty);
    CVXRUN(&t, test_im_int_clone_values);

    CVXRUN(&t, test_im_int_clear);
    CVXRUN(&t, test_im_int_empty);

    CVXRUN(&t, test_im_int_add_single);
    CVXRUN(&t, test_im_int_add_two_disjoint);
    CVXRUN(&t, test_im_int_add_overwrite_exact);
    CVXRUN(&t, test_im_int_add_partial_left_overlap);
    CVXRUN(&t, test_im_int_add_partial_right_overlap);
    CVXRUN(&t, test_im_int_add_full_containment);
    CVXRUN(&t, test_im_int_add_spanning_multiple);
    CVXRUN(&t, test_im_int_add_adjacent_same_value_join);
    CVXRUN(&t, test_im_int_add_adjacent_different_value_no_join);
    CVXRUN(&t, test_im_int_add_invalid_range);
    CVXRUN(&t, test_im_int_add_no_vtabk);

    CVXRUN(&t, test_im_int_remove_exact);
    CVXRUN(&t, test_im_int_remove_split);
    CVXRUN(&t, test_im_int_remove_trim_left);
    CVXRUN(&t, test_im_int_remove_trim_right);
    CVXRUN(&t, test_im_int_remove_spanning_multiple);
    CVXRUN(&t, test_im_int_remove_no_overlap);
    CVXRUN(&t, test_im_int_remove_empty_range);

    CVXRUN(&t, test_im_int_get_covered_key);
    CVXRUN(&t, test_im_int_get_lo_boundary);
    CVXRUN(&t, test_im_int_get_hi_boundary);
    CVXRUN(&t, test_im_int_get_gap);
    CVXRUN(&t, test_im_int_get_empty);

    CVXRUN(&t, test_im_int_contains_key_yes);
    CVXRUN(&t, test_im_int_contains_key_no);

    CVXRUN(&t, test_im_int_contains_interval_full);
    CVXRUN(&t, test_im_int_contains_interval_partial);
    CVXRUN(&t, test_im_int_contains_interval_empty);

    CVXRUN(&t, test_im_int_overlaps_yes);
    CVXRUN(&t, test_im_int_overlaps_no);
    CVXRUN(&t, test_im_int_overlaps_touching_not_overlap);
    CVXRUN(&t, test_im_int_overlaps_empty);

    return CVXSUMMARY(&t);
}

#endif /* INTERVAL_MAP_TESTS_H */
