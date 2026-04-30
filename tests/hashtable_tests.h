#ifndef HASHTABLE_TESTS_H
#define HASHTABLE_TESTS_H

#include "cvxtest.h"

#include "implementations.h"

/* ---- init ---- */

static void test_ht_int_init(struct cvxtest *t)
{
    struct ht_int_int s = ht_int_init(ht_int_vtabk, NULL);

    CVXCHECK(t, s.super.tag == 22);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, s.count == 0);
    CVXCHECK(t, s.capacity == 0);
    CVXCHECK(t, s.buffer == NULL);
    CVXCHECK(t, s.vtabk == ht_int_vtabk);
    CVXCHECK(t, s.vtabv == NULL);
    CVXCHECK(t, s.load == 0.7);
}

static void test_ht_int_init_null_vtabk(struct cvxtest *t)
{
    struct ht_int_int s = ht_int_init(NULL, NULL);

    CVXCHECK(t, s.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, s.super.tag == 0);
}

static void test_ht_int_init_no_hash(struct cvxtest *t)
{
    struct ht_int_int_vtabk vtabk = { .comp = int_comp };
    struct ht_int_int s = ht_int_init(&vtabk, NULL);

    CVXCHECK(t, s.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, s.super.tag == 0);
}

static void test_ht_int_init_no_comp(struct cvxtest *t)
{
    struct ht_int_int_vtabk vtabk = { .hash = int_hash };
    struct ht_int_int s = ht_int_init(&vtabk, NULL);

    CVXCHECK(t, s.super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, s.super.tag == 0);
}

/* ---- init_with ---- */

static void test_ht_int_init_with(struct cvxtest *t)
{
    struct ht_int_int s = ht_int_init_with(ht_int_vtabk, NULL, 10);

    CVXCHECK(t, s.super.tag == 22);
    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, s.count == 0);
    CVXCHECK(t, s.capacity >= 10);
    CVXCHECK(t, s.buffer != NULL);

    ht_int_clear(&s);
}

static void test_ht_int_init_with_zero(struct cvxtest *t)
{
    struct ht_int_int s = ht_int_init_with(ht_int_vtabk, NULL, 0);

    CVXCHECK(t, s.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, s.capacity == 0);
    CVXCHECK(t, s.buffer == NULL);
}

/* ---- new / new_with ---- */

static void test_ht_int_new(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new();
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->super.tag == 22);
    CVXCHECK(t, col->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, col->count == 0);
    CVXCHECK(t, col->capacity == 0);
    CVXCHECK(t, col->vtabk == NULL);
    CVXCHECK(t, col->vtabv == NULL);

    ht_int_drop(col);
}

static void test_ht_int_new_with(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    CVXCHECK(t, col != NULL);
    if (!col)
        return;

    CVXCHECK(t, col->super.tag == 22);
    CVXCHECK(t, col->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, col->count == 0);
    CVXCHECK(t, col->vtabk == ht_int_vtabk);
    CVXCHECK(t, col->vtabv == NULL);

    ht_int_drop(col);
}

/* ---- copy ---- */

static void test_ht_int_copy_empty(struct cvxtest *t)
{
    struct ht_int_int orig = ht_int_init(ht_int_vtabk, NULL);
    struct ht_int_int copy = ht_int_copy(&orig);

    CVXCHECK(t, copy.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, copy.super.tag == 22);
    CVXCHECK(t, copy.count == 0);
    CVXCHECK(t, copy.buffer == NULL);
    CVXCHECK(t, copy.vtabk == ht_int_vtabk);
}

static void test_ht_int_copy_nonempty(struct cvxtest *t)
{
    struct ht_int_int orig = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&orig, 1, 10);
    ht_int_insert(&orig, 2, 20);

    struct ht_int_int copy = ht_int_copy(&orig);

    CVXCHECK(t, copy.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, copy.count == 2);
    CVXCHECK(t, ht_int_get(&copy, 1) == 10);
    CVXCHECK(t, ht_int_get(&copy, 2) == 20);
    CVXCHECK(t, copy.buffer != orig.buffer);

    ht_int_clear(&orig);
    ht_int_clear(&copy);
}

/* ---- clone ---- */

static void test_ht_int_clone_empty(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    struct ht_int_int *clone = ht_int_clone(col);

    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        ht_int_drop(col);
        return;
    }

    CVXCHECK(t, clone->super.tag == 22);
    CVXCHECK(t, clone->count == 0);
    CVXCHECK(t, clone->vtabk == ht_int_vtabk);

    ht_int_drop(col);
    ht_int_drop(clone);
}

static void test_ht_int_clone_nonempty(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    ht_int_insert(col, 10, 100);
    ht_int_insert(col, 20, 200);

    struct ht_int_int *clone = ht_int_clone(col);

    CVXCHECK(t, clone != NULL);
    if (!clone)
    {
        ht_int_drop(col);
        return;
    }

    CVXCHECK(t, clone->count == 2);
    CVXCHECK(t, ht_int_get(clone, 10) == 100);
    CVXCHECK(t, ht_int_get(clone, 20) == 200);
    CVXCHECK(t, clone->buffer != col->buffer);

    ht_int_drop(col);
    ht_int_drop(clone);
}

/* ---- drop ---- */

static void test_ht_int_drop_empty(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    CVXCHECK(t, col != NULL);
    ht_int_drop(col); // must not crash
}

static void test_ht_int_drop_nonempty(struct cvxtest *t)
{
    (void)t;
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    ht_int_insert(col, 1, 10);
    ht_int_insert(col, 2, 20);
    ht_int_drop(col); // must not crash
}

/* ---- clear ---- */

static void test_ht_int_clear_empty(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_clear(&col);

    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, col.count == 0);
    CVXCHECK(t, col.capacity == 0);
    CVXCHECK(t, col.buffer == NULL);
}

static void test_ht_int_clear_nonempty(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 1, 10);
    ht_int_clear(&col);

    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, col.count == 0);
    CVXCHECK(t, col.buffer == NULL);

    // Container is still valid for re-use after clear
    ht_int_insert(&col, 5, 50);
    CVXCHECK(t, ht_int_count(&col) == 1);
    CVXCHECK(t, ht_int_get(&col, 5) == 50);

    ht_int_clear(&col);
}

/* ---- getters ---- */

static void test_ht_int_flag(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    CVXCHECK(t, ht_int_flag(&col) == CVX_FLAG_OK);
}

static void test_ht_int_count(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    CVXCHECK(t, ht_int_count(&col) == 0);

    ht_int_insert(&col, 1, 10);
    CVXCHECK(t, ht_int_count(&col) == 1);

    ht_int_clear(&col);
}

static void test_ht_int_capacity(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    CVXCHECK(t, ht_int_capacity(&col) == 0);

    ht_int_insert(&col, 1, 10);
    CVXCHECK(t, ht_int_capacity(&col) > 0);

    ht_int_clear(&col);
}

static void test_ht_int_load(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    CVXCHECK(t, ht_int_load(&col) == 0.7);
}

static void test_ht_int_empty(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    CVXCHECK(t, ht_int_empty(&col) == true);

    ht_int_insert(&col, 1, 10);
    CVXCHECK(t, ht_int_empty(&col) == false);

    ht_int_clear(&col);
}

/* ---- insert ---- */

static void test_ht_int_insert_basic(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);

    bool ok = ht_int_insert(&col, 42, 100);

    CVXCHECK(t, ok == true);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, ht_int_count(&col) == 1);

    ht_int_clear(&col);
}

static void test_ht_int_insert_multiple(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);

    for (int i = 0; i < 10; i++)
        ht_int_insert(&col, i, i * 10);

    CVXCHECK(t, ht_int_count(&col) == 10);

    for (int i = 0; i < 10; i++)
        CVXCHECK(t, ht_int_get(&col, i) == i * 10);

    ht_int_clear(&col);
}

static void test_ht_int_insert_duplicate(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);

    ht_int_insert(&col, 1, 10);
    bool ok = ht_int_insert(&col, 1, 99);

    CVXCHECK(t, ok == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_DUPLICATE);
    CVXCHECK(t, ht_int_count(&col) == 1);
    CVXCHECK(t, ht_int_get(&col, 1) == 10); // original value preserved

    ht_int_clear(&col);
}

static void test_ht_int_insert_triggers_resize(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);

    // Insert enough entries to force at least one resize
    for (int i = 0; i < 60; i++)
        ht_int_insert(&col, i, i);

    CVXCHECK(t, ht_int_count(&col) == 60);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);

    // All entries must still be retrievable after resize
    for (int i = 0; i < 60; i++)
        CVXCHECK(t, ht_int_get(&col, i) == i);

    ht_int_clear(&col);
}

static void test_ht_int_insert_no_vtab(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new();
    bool ok = ht_int_insert(col, 1, 10);

    CVXCHECK(t, ok == false);
    CVXCHECK(t, col->super.flag == CVX_FLAG_VTAB);

    ht_int_drop(col);
}

/* ---- update ---- */

static void test_ht_int_update_hit(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 5, 50);

    int old = 0;
    bool ok = ht_int_update(&col, 5, 99, &old);

    CVXCHECK(t, ok == true);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, old == 50);
    CVXCHECK(t, ht_int_get(&col, 5) == 99);

    ht_int_clear(&col);
}

static void test_ht_int_update_miss(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);

    bool ok = ht_int_update(&col, 5, 99, NULL);

    CVXCHECK(t, ok == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_NOT_FOUND);
}

static void test_ht_int_update_no_vtab(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new();
    bool ok = ht_int_update(col, 1, 10, NULL);

    CVXCHECK(t, ok == false);
    CVXCHECK(t, col->super.flag == CVX_FLAG_VTAB);

    ht_int_drop(col);
}

/* ---- remove ---- */

static void test_ht_int_remove_hit(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 7, 70);

    int out = 0;
    bool ok = ht_int_remove(&col, 7, &out);

    CVXCHECK(t, ok == true);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, out == 70);
    CVXCHECK(t, ht_int_count(&col) == 0);
    CVXCHECK(t, ht_int_contains(&col, 7) == false);

    ht_int_clear(&col);
}

static void test_ht_int_remove_miss(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 1, 10);

    bool ok = ht_int_remove(&col, 999, NULL);

    CVXCHECK(t, ok == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_NOT_FOUND);
    CVXCHECK(t, ht_int_count(&col) == 1);

    ht_int_clear(&col);
}

static void test_ht_int_remove_empty(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);

    bool ok = ht_int_remove(&col, 1, NULL);

    CVXCHECK(t, ok == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_EMPTY);
}

static void test_ht_int_remove_tombstone_then_find(struct cvxtest *t)
{
    // Insert multiple keys, remove one in the middle, verify others still findable.
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);

    for (int i = 0; i < 10; i++)
        ht_int_insert(&col, i, i * 10);

    ht_int_remove(&col, 5, NULL);

    CVXCHECK(t, ht_int_count(&col) == 9);
    CVXCHECK(t, ht_int_contains(&col, 5) == false);

    // Keys that may have been probed past the tombstone must still be found.
    for (int i = 0; i < 10; i++)
    {
        if (i == 5)
            continue;
        CVXCHECK(t, ht_int_get(&col, i) == i * 10);
    }

    ht_int_clear(&col);
}

static void test_ht_int_remove_no_vtab(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new();
    bool ok = ht_int_remove(col, 1, NULL);

    CVXCHECK(t, ok == false);
    CVXCHECK(t, col->super.flag == CVX_FLAG_VTAB);

    ht_int_drop(col);
}

/* ---- get ---- */

static void test_ht_int_get_hit(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 3, 33);

    int val = ht_int_get(&col, 3);

    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, val == 33);

    ht_int_clear(&col);
}

static void test_ht_int_get_miss(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);

    int val = ht_int_get(&col, 999);

    CVXCHECK(t, col.super.flag == CVX_FLAG_NOT_FOUND);
    CVXCHECK(t, val == 0);
}

static void test_ht_int_get_no_vtab(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new();
    int val = ht_int_get(col, 1);

    CVXCHECK(t, col->super.flag == CVX_FLAG_VTAB);
    CVXCHECK(t, val == 0);

    ht_int_drop(col);
}

/* ---- get_ref ---- */

static void test_ht_int_get_ref_hit(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 8, 80);

    int *ref = ht_int_get_ref(&col, 8);

    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, ref != NULL);
    if (!ref)
    {
        ht_int_clear(&col);
        return;
    }
    CVXCHECK(t, *ref == 80);

    *ref = 88;
    CVXCHECK(t, ht_int_get(&col, 8) == 88);

    ht_int_clear(&col);
}

static void test_ht_int_get_ref_miss(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);

    int *ref = ht_int_get_ref(&col, 999);

    CVXCHECK(t, col.super.flag == CVX_FLAG_NOT_FOUND);
    CVXCHECK(t, ref == NULL);
}

/* ---- contains ---- */

static void test_ht_int_contains_hit(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 4, 40);

    CVXCHECK(t, ht_int_contains(&col, 4) == true);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);

    ht_int_clear(&col);
}

static void test_ht_int_contains_miss(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);

    CVXCHECK(t, ht_int_contains(&col, 999) == false);
    CVXCHECK(t, col.super.flag == CVX_FLAG_OK);
}

static void test_ht_int_contains_no_vtab(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new();
    bool found = ht_int_contains(col, 1);

    CVXCHECK(t, found == false);
    CVXCHECK(t, col->super.flag == CVX_FLAG_VTAB);

    ht_int_drop(col);
}

/* ---- runner ---- */

static struct cvxresult run_hashtable_tests(void)
{
    struct cvxtest t = { 0 };

    printf("hashtable\n");

    CVXRUN(&t, test_ht_int_init);
    CVXRUN(&t, test_ht_int_init_null_vtabk);
    CVXRUN(&t, test_ht_int_init_no_hash);
    CVXRUN(&t, test_ht_int_init_no_comp);

    CVXRUN(&t, test_ht_int_init_with);
    CVXRUN(&t, test_ht_int_init_with_zero);

    CVXRUN(&t, test_ht_int_new);
    CVXRUN(&t, test_ht_int_new_with);

    CVXRUN(&t, test_ht_int_copy_empty);
    CVXRUN(&t, test_ht_int_copy_nonempty);

    CVXRUN(&t, test_ht_int_clone_empty);
    CVXRUN(&t, test_ht_int_clone_nonempty);

    CVXRUN(&t, test_ht_int_drop_empty);
    CVXRUN(&t, test_ht_int_drop_nonempty);

    CVXRUN(&t, test_ht_int_clear_empty);
    CVXRUN(&t, test_ht_int_clear_nonempty);

    CVXRUN(&t, test_ht_int_flag);
    CVXRUN(&t, test_ht_int_count);
    CVXRUN(&t, test_ht_int_capacity);
    CVXRUN(&t, test_ht_int_load);
    CVXRUN(&t, test_ht_int_empty);

    CVXRUN(&t, test_ht_int_insert_basic);
    CVXRUN(&t, test_ht_int_insert_multiple);
    CVXRUN(&t, test_ht_int_insert_duplicate);
    CVXRUN(&t, test_ht_int_insert_triggers_resize);
    CVXRUN(&t, test_ht_int_insert_no_vtab);

    CVXRUN(&t, test_ht_int_update_hit);
    CVXRUN(&t, test_ht_int_update_miss);
    CVXRUN(&t, test_ht_int_update_no_vtab);

    CVXRUN(&t, test_ht_int_remove_hit);
    CVXRUN(&t, test_ht_int_remove_miss);
    CVXRUN(&t, test_ht_int_remove_empty);
    CVXRUN(&t, test_ht_int_remove_tombstone_then_find);
    CVXRUN(&t, test_ht_int_remove_no_vtab);

    CVXRUN(&t, test_ht_int_get_hit);
    CVXRUN(&t, test_ht_int_get_miss);
    CVXRUN(&t, test_ht_int_get_no_vtab);

    CVXRUN(&t, test_ht_int_get_ref_hit);
    CVXRUN(&t, test_ht_int_get_ref_miss);

    CVXRUN(&t, test_ht_int_contains_hit);
    CVXRUN(&t, test_ht_int_contains_miss);
    CVXRUN(&t, test_ht_int_contains_no_vtab);

    return CVXSUMMARY(&t);
}

#endif /* HASHTABLE_TESTS_H */
