#ifndef HASHTABLE_ITER_TESTS_H
#define HASHTABLE_ITER_TESTS_H

#include "cvxtest.h"

#include "implementations.h"

/* ---- iter_init_start / iter_start ---- */

static void test_ht_int_iter_init_start_empty(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    struct ht_int_int_iter iter = ht_int_iter_init_start(&col);

    CVXCHECK(t, iter.super.tag == HT_ITER_TAG);
    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter.index == 0);
    CVXCHECK(t, iter.target == &col);
    CVXCHECK(t, ht_int_iter_at_start(&iter) == true);
    CVXCHECK(t, ht_int_iter_at_end(&iter) == true); // empty: start == end
}

static void test_ht_int_iter_init_start_nonempty(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 1, 10);
    ht_int_insert(&col, 2, 20);
    ht_int_insert(&col, 3, 30);

    struct ht_int_int_iter iter = ht_int_iter_init_start(&col);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter.index == 0);
    CVXCHECK(t, ht_int_iter_at_start(&iter) == true);
    CVXCHECK(t, ht_int_iter_at_end(&iter) == false);

    ht_int_clear(&col);
}

static void test_ht_int_iter_init_end_empty(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    struct ht_int_int_iter iter = ht_int_iter_init_end(&col);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, ht_int_iter_at_end(&iter) == true);
    CVXCHECK(t, ht_int_iter_at_start(&iter) == true);
}

static void test_ht_int_iter_init_end_nonempty(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 5, 50);

    struct ht_int_int_iter iter = ht_int_iter_init_end(&col);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter.index == 1);
    CVXCHECK(t, ht_int_iter_at_end(&iter) == true);
    CVXCHECK(t, ht_int_iter_at_start(&iter) == false);

    ht_int_clear(&col);
}

static void test_ht_int_iter_start_heap(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    ht_int_insert(col, 42, 420);

    struct ht_int_int_iter *iter = ht_int_iter_start(col);
    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        ht_int_drop(col);
        return;
    }

    CVXCHECK(t, iter->super.tag == HT_ITER_TAG);
    CVXCHECK(t, ht_int_iter_at_start(iter) == true);

    ht_int_iter_drop(iter);
    ht_int_drop(col);
}

static void test_ht_int_iter_end_heap(struct cvxtest *t)
{
    struct ht_int_int *col = ht_int_new_with(ht_int_vtabk, NULL, 0);
    ht_int_insert(col, 42, 420);

    struct ht_int_int_iter *iter = ht_int_iter_end(col);
    CVXCHECK(t, iter != NULL);
    if (!iter)
    {
        ht_int_drop(col);
        return;
    }

    CVXCHECK(t, iter->super.tag == HT_ITER_TAG);
    CVXCHECK(t, ht_int_iter_at_end(iter) == true);

    ht_int_iter_drop(iter);
    ht_int_drop(col);
}

/* ---- iter_count ---- */

static void test_ht_int_iter_count(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 1, 10);
    ht_int_insert(&col, 2, 20);

    struct ht_int_int_iter iter = ht_int_iter_init_start(&col);
    CVXCHECK(t, ht_int_iter_count(&iter) == 2);

    ht_int_clear(&col);
}

/* ---- iter_next / iter_key / iter_value / iter_index ---- */

static void test_ht_int_iter_forward_traversal(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    for (int i = 1; i <= 5; i++)
        ht_int_insert(&col, i, i * 100);

    struct ht_int_int_iter iter = ht_int_iter_init_start(&col);

    size_t visited = 0;
    while (!ht_int_iter_at_end(&iter))
    {
        int k = ht_int_iter_key(&iter);
        int v = ht_int_iter_value(&iter);
        CVXCHECK(t, v == k * 100);
        CVXCHECK(t, ht_int_iter_index(&iter) == visited);
        visited++;
        ht_int_iter_next(&iter);
    }

    CVXCHECK(t, visited == 5);

    ht_int_clear(&col);
}

static void test_ht_int_iter_next_at_end(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 1, 10);

    struct ht_int_int_iter iter = ht_int_iter_init_end(&col);
    ht_int_iter_next(&iter);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_RANGE);

    ht_int_clear(&col);
}

static void test_ht_int_iter_key_at_end(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    struct ht_int_int_iter iter = ht_int_iter_init_end(&col);

    int k = ht_int_iter_key(&iter);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_RANGE);
    CVXCHECK(t, k == 0);
}

static void test_ht_int_iter_value_at_end(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    struct ht_int_int_iter iter = ht_int_iter_init_end(&col);

    int v = ht_int_iter_value(&iter);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_RANGE);
    CVXCHECK(t, v == 0);
}

/* ---- iter_prev ---- */

static void test_ht_int_iter_backward_traversal(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    for (int i = 1; i <= 5; i++)
        ht_int_insert(&col, i, i * 100);

    struct ht_int_int_iter iter = ht_int_iter_init_end(&col);

    size_t visited = 0;
    while (!ht_int_iter_at_start(&iter))
    {
        ht_int_iter_prev(&iter);
        int k = ht_int_iter_key(&iter);
        int v = ht_int_iter_value(&iter);
        CVXCHECK(t, v == k * 100);
        visited++;
    }

    CVXCHECK(t, visited == 5);

    ht_int_clear(&col);
}

static void test_ht_int_iter_prev_at_start(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 1, 10);

    struct ht_int_int_iter iter = ht_int_iter_init_start(&col);
    ht_int_iter_prev(&iter);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_RANGE);

    ht_int_clear(&col);
}

/* ---- iter_to_start / iter_to_end ---- */

static void test_ht_int_iter_to_start(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 1, 10);
    ht_int_insert(&col, 2, 20);

    struct ht_int_int_iter iter = ht_int_iter_init_end(&col);
    ht_int_iter_to_start(&iter);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, ht_int_iter_at_start(&iter) == true);
    CVXCHECK(t, ht_int_iter_at_end(&iter) == false);

    ht_int_clear(&col);
}

static void test_ht_int_iter_to_end(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 1, 10);

    struct ht_int_int_iter iter = ht_int_iter_init_start(&col);
    ht_int_iter_to_end(&iter);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, ht_int_iter_at_end(&iter) == true);

    ht_int_clear(&col);
}

/* ---- iter_forward / iter_backward ---- */

static void test_ht_int_iter_forward(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    for (int i = 0; i < 5; i++)
        ht_int_insert(&col, i, i);

    struct ht_int_int_iter iter = ht_int_iter_init_start(&col);
    ht_int_iter_forward(&iter, 3);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter.index == 3);

    ht_int_clear(&col);
}

static void test_ht_int_iter_forward_clamps(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 1, 10);

    struct ht_int_int_iter iter = ht_int_iter_init_start(&col);
    ht_int_iter_forward(&iter, 100);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, ht_int_iter_at_end(&iter) == true);

    ht_int_clear(&col);
}

static void test_ht_int_iter_backward(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    for (int i = 0; i < 5; i++)
        ht_int_insert(&col, i, i);

    struct ht_int_int_iter iter = ht_int_iter_init_end(&col);
    ht_int_iter_backward(&iter, 3);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter.index == 2);

    ht_int_clear(&col);
}

static void test_ht_int_iter_backward_clamps(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    ht_int_insert(&col, 1, 10);

    struct ht_int_int_iter iter = ht_int_iter_init_end(&col);
    ht_int_iter_backward(&iter, 100);

    CVXCHECK(t, iter.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, ht_int_iter_at_start(&iter) == true);

    ht_int_clear(&col);
}

/* ---- round-trip: forward then backward visits same elements ---- */

static void test_ht_int_iter_roundtrip(struct cvxtest *t)
{
    struct ht_int_int col = ht_int_init(ht_int_vtabk, NULL);
    for (int i = 1; i <= 4; i++)
        ht_int_insert(&col, i, i);

    // Collect all keys going forward
    int fwd[4];
    struct ht_int_int_iter iter = ht_int_iter_init_start(&col);
    for (int i = 0; i < 4; i++)
    {
        fwd[i] = ht_int_iter_key(&iter);
        ht_int_iter_next(&iter);
    }

    // Collect all keys going backward from end
    int bwd[4];
    ht_int_iter_to_end(&iter);
    for (int i = 3; i >= 0; i--)
    {
        ht_int_iter_prev(&iter);
        bwd[i] = ht_int_iter_key(&iter);
    }

    for (int i = 0; i < 4; i++)
        CVXCHECK(t, fwd[i] == bwd[i]);

    ht_int_clear(&col);
}

/* ---- runner ---- */

static struct cvxresult run_hashtable_iter_tests(void)
{
    struct cvxtest t = { 0 };

    printf("hashtable (iter)\n");

    CVXRUN(&t, test_ht_int_iter_init_start_empty);
    CVXRUN(&t, test_ht_int_iter_init_start_nonempty);
    CVXRUN(&t, test_ht_int_iter_init_end_empty);
    CVXRUN(&t, test_ht_int_iter_init_end_nonempty);
    CVXRUN(&t, test_ht_int_iter_start_heap);
    CVXRUN(&t, test_ht_int_iter_end_heap);

    CVXRUN(&t, test_ht_int_iter_count);

    CVXRUN(&t, test_ht_int_iter_forward_traversal);
    CVXRUN(&t, test_ht_int_iter_next_at_end);
    CVXRUN(&t, test_ht_int_iter_key_at_end);
    CVXRUN(&t, test_ht_int_iter_value_at_end);

    CVXRUN(&t, test_ht_int_iter_backward_traversal);
    CVXRUN(&t, test_ht_int_iter_prev_at_start);

    CVXRUN(&t, test_ht_int_iter_to_start);
    CVXRUN(&t, test_ht_int_iter_to_end);

    CVXRUN(&t, test_ht_int_iter_forward);
    CVXRUN(&t, test_ht_int_iter_forward_clamps);
    CVXRUN(&t, test_ht_int_iter_backward);
    CVXRUN(&t, test_ht_int_iter_backward_clamps);

    CVXRUN(&t, test_ht_int_iter_roundtrip);

    return CVXSUMMARY(&t);
}

#endif /* HASHTABLE_ITER_TESTS_H */
