#ifndef BINARY_HEAP_VTAB_TESTS_H
#define BINARY_HEAP_VTAB_TESTS_H

#include "cvxtest.h"
#include "implementations.h"

static void test_bh_int_vtabv_copy_called_on_copy(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_full, CVX_MAX_HEAP, 0);
    bh_int_push(h, 1);
    bh_int_push(h, 2);
    bh_int_push(h, 3);

    struct bheap_int copy = bh_int_copy(h);

    CVX_TEST_COUNTER_COPY(t, 3);
    CVXCHECK(t, copy.count == 3);

    free(copy.buffer);
    bh_int_drop(h);
}

static void test_bh_int_vtabv_copy_called_on_clone(struct cvxtest *t)
{
    CVX_TEST_COUNTER_COPY_RESET();

    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_full, CVX_MAX_HEAP, 0);
    bh_int_push(h, 10);
    bh_int_push(h, 20);
    bh_int_push(h, 30);

    struct bheap_int *clone = bh_int_clone(h);

    CVX_TEST_COUNTER_COPY(t, 3);
    CVXCHECK(t, bh_int_count(clone) == 3);

    bh_int_drop(h);
    bh_int_drop(clone);
}

static void test_bh_int_vtabv_null_copy_no_crash(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(h, 7);
    bh_int_push(h, 8);
    bh_int_push(h, 9);

    struct bheap_int *clone = bh_int_clone(h);

    CVXCHECK(t, clone != NULL);
    CVXCHECK(t, bh_int_count(clone) == 3);

    bh_int_drop(h);
    bh_int_drop(clone);
}

static void test_bh_int_vtabv_drop_called_on_drop(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_full, CVX_MAX_HEAP, 0);
    bh_int_push(h, 1);
    bh_int_push(h, 2);
    bh_int_push(h, 3);

    bh_int_drop(h);

    CVX_TEST_COUNTER_DROP(t, 3);
}

static void test_bh_int_vtabv_null_drop_no_crash(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    bh_int_push(h, 1);
    bh_int_push(h, 2);
    bh_int_push(h, 3);

    bh_int_drop(h);

    CVXCHECK(t, true);
}

static void test_bh_int_vtabv_drop_called_on_clear(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_full, CVX_MAX_HEAP, 0);
    bh_int_push(h, 1);
    bh_int_push(h, 2);
    bh_int_push(h, 3);

    bh_int_clear(h);

    CVX_TEST_COUNTER_DROP(t, 3);

    bh_int_drop(h);
}

/* ---- runner ---- */

static struct cvxresult run_binary_heap_vtab_tests(void)
{
    struct cvxtest t = { 0 };

    printf("binary_heap vtabv\n");

    CVXRUN(&t, test_bh_int_vtabv_copy_called_on_copy);
    CVXRUN(&t, test_bh_int_vtabv_copy_called_on_clone);
    CVXRUN(&t, test_bh_int_vtabv_null_copy_no_crash);

    CVXRUN(&t, test_bh_int_vtabv_drop_called_on_drop);
    CVXRUN(&t, test_bh_int_vtabv_null_drop_no_crash);
    CVXRUN(&t, test_bh_int_vtabv_drop_called_on_clear);

    return CVXSUMMARY(&t);
}

#endif /* BINARY_HEAP_VTAB_TESTS_H */
