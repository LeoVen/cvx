#ifndef SLINKED_LIST_TESTS_H
#define SLINKED_LIST_TESTS_H

#include "cvx/flags.h"
// alloc.h must precede implementations.h so malloc interception is in effect
#include "tests/alloc.h"
#include "tests/cvxtest.h"
#include "tests/cvxtestutils.h"
#include "tests/implementations.h"

static void sll_fill3(struct slinked_int *col)
{
    sll_int_push_back(col, 10);
    sll_int_push_back(col, 20);
    sll_int_push_back(col, 30);
}

static void test_sll_int_init(struct cvxtest *t)
{
    // Default: no vtabv
    struct slinked_int l;
    sll_int_init(&l, NULL);
    CVXCHECK(t, l.super.tag == 77);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, l.head == NULL);
    CVXCHECK(t, l.tail == NULL);
    CVXCHECK(t, l.count == 0);
    CVXCHECK(t, l.vtabv == NULL);
    sll_int_drop(&l);
    // With vtabv
    sll_int_init(&l, sll_int_vtabv_full);
    CVXCHECK(t, l.vtabv == sll_int_vtabv_full);
    sll_int_drop(&l);
}

static void test_sll_int_clone(struct cvxtest *t)
{
    // Clone empty list
    struct slinked_int orig, clone;
    sll_int_init(&orig, NULL);
    sll_int_clone(&orig, &clone);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, clone.super.tag == 77);
    CVXCHECK(t, clone.count == 0);
    sll_int_drop(&orig);
    sll_int_drop(&clone);
    // Clone with values (direct assignment path)
    sll_int_init(&orig, NULL);
    sll_int_push_back(&orig, 10);
    sll_int_push_back(&orig, 20);
    sll_int_push_back(&orig, 30);
    sll_int_clone(&orig, &clone);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, clone.count == 3);
    CVXCHECK(t, sll_int_get(&clone, 0) == 10);
    CVXCHECK(t, sll_int_get(&clone, 1) == 20);
    CVXCHECK(t, sll_int_get(&clone, 2) == 30);
    sll_int_push_back(&orig, 40);
    CVXCHECK(t, clone.count == 3);
    sll_int_drop(&orig);
    sll_int_drop(&clone);
    // Clone with vtabv->clone
    CVX_TEST_COUNTER_CLONE_RESET();
    sll_int_init(&orig, sll_int_vtabv_full);
    sll_int_push_back(&orig, 10);
    sll_int_push_back(&orig, 20);
    sll_int_push_back(&orig, 30);
    sll_int_clone(&orig, &clone);
    CVX_TEST_COUNTER_CLONE(t, 3);
    CVXCHECK(t, orig.vtabv == clone.vtabv);
    sll_int_drop(&orig);
    sll_int_drop(&clone);
    // Alloc failure: first node fails
    sll_int_init(&orig, NULL);
    sll_int_push_back(&orig, 10);
    sll_int_push_back(&orig, 20);
    CVX_MALLOC_FAIL_NEXT();
    sll_int_clone(&orig, &clone);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, orig.super.flag == CVX_FLAG_ALLOC);
    CVX_MALLOC_RESET();
    sll_int_drop(&orig);
    sll_int_drop(&clone);
    // Alloc failure: second node fails
    sll_int_init(&orig, NULL);
    sll_int_push_back(&orig, 10);
    sll_int_push_back(&orig, 20);
    CVX_MALLOC_FAIL_AFTER(1);
    sll_int_clone(&orig, &clone);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_ALLOC);
    CVX_MALLOC_RESET();
    sll_int_drop(&orig);
    sll_int_drop(&clone);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    struct slinked_int dummy;
    sll_int_init(&dummy, NULL);
    sll_int__proxy_clone(col, (cvx_container *)&dummy);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
    sll_int_drop(&dummy);
}

static void test_sll_int_drop(struct cvxtest *t)
{
    // Drop empty list
    struct slinked_int l;
    sll_int_init(&l, NULL);
    sll_int_drop(&l);
    // Drop with elements; vtabv->drop called for each
    CVX_TEST_COUNTER_DROP_RESET();
    sll_int_init(&l, sll_int_vtabv_full);
    sll_int_push_back(&l, 1);
    sll_int_push_back(&l, 2);
    sll_int_push_back(&l, 3);
    sll_int_drop(&l);
    CVX_TEST_COUNTER_DROP(t, 3);
    // Drop with NULL vtabv — no crash
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 1);
    sll_int_push_back(&l, 2);
    sll_int_push_back(&l, 3);
    sll_int_drop(&l);
    // Drop NULL pointer — no crash
    sll_int_drop(NULL);
    // Random sequence of _init / _drop / _clone must always work
    struct slinked_int b;
    sll_int_init(&l, NULL);
    for (int i = 0; i < 100; i++)
    {
        int choice = rand() % 3;
        if (choice == 0)
            sll_int_init(&l, NULL);
        else if (choice == 1)
            sll_int_drop(&l);
        else
        {
            sll_int_clone(&l, &b);
            sll_int_drop(&b);
        }
    }
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    sll_int__proxy_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_count(struct cvxtest *t)
{
    struct slinked_int l;
    sll_int_init(&l, NULL);
    CVXCHECK(t, sll_int_count(&l) == 0);
    sll_int_push_back(&l, 1);
    CVXCHECK(t, sll_int_count(&l) == 1);
    sll_int_push_back(&l, 2);
    CVXCHECK(t, sll_int_count(&l) == 2);
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    size_t r = sll_int__proxy_count(col);
    CVXCHECK(t, r == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_empty(struct cvxtest *t)
{
    struct slinked_int l;
    sll_int_init(&l, NULL);
    CVXCHECK(t, sll_int_empty(&l) == true);
    sll_int_push_back(&l, 1);
    CVXCHECK(t, sll_int_empty(&l) == false);
    sll_int_pop_front(&l);
    CVXCHECK(t, sll_int_empty(&l) == true);
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    bool r = sll_int__proxy_empty(col);
    CVXCHECK(t, r == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_front(struct cvxtest *t)
{
    struct slinked_int l;
    sll_int_init(&l, NULL);
    sll_fill3(&l);
    CVXCHECK(t, sll_int_front(&l) == 10);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    sll_int_drop(&l);
    // Empty
    sll_int_init(&l, NULL);
    sll_int_front(&l);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    int r = sll_int__proxy_front(col);
    CVXCHECK(t, r == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_back(struct cvxtest *t)
{
    struct slinked_int l;
    sll_int_init(&l, NULL);
    sll_fill3(&l);
    CVXCHECK(t, sll_int_back(&l) == 30);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    sll_int_drop(&l);
    // Empty
    sll_int_init(&l, NULL);
    sll_int_back(&l);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    int r = sll_int__proxy_back(col);
    CVXCHECK(t, r == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_get(struct cvxtest *t)
{
    struct slinked_int l;
    sll_int_init(&l, NULL);
    sll_fill3(&l);
    CVXCHECK(t, sll_int_get(&l, 0) == 10);
    CVXCHECK(t, sll_int_get(&l, 1) == 20);
    CVXCHECK(t, sll_int_get(&l, 2) == 30);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    sll_int_drop(&l);
    // Out of range
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 1);
    sll_int_get(&l, 1);
    CVXCHECK(t, l.super.flag == CVX_FLAG_RANGE);
    sll_int_drop(&l);
    // Empty
    sll_int_init(&l, NULL);
    sll_int_get(&l, 0);
    CVXCHECK(t, l.super.flag == CVX_FLAG_RANGE);
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    int r = sll_int__proxy_get(col, 0);
    CVXCHECK(t, r == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_push_front(struct cvxtest *t)
{
    struct slinked_int l;
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 2);
    sll_int_push_back(&l, 3);
    sll_int_push_front(&l, 1);
    CVXCHECK(t, sll_int_count(&l) == 3);
    CVXCHECK(t, sll_int_front(&l) == 1);
    CVXCHECK(t, sll_int_back(&l) == 3);
    CVXCHECK(t, sll_int_get(&l, 1) == 2);
    sll_int_drop(&l);
    // Alloc failure on empty list
    sll_int_init(&l, NULL);
    CVX_MALLOC_FAIL_NEXT();
    sll_int_push_front(&l, 42);
    CVXCHECK(t, l.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, sll_int_count(&l) == 0);
    CVX_MALLOC_RESET();
    sll_int_drop(&l);
    // Alloc failure with existing elements
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 10);
    sll_int_push_back(&l, 20);
    CVX_MALLOC_FAIL_NEXT();
    sll_int_push_front(&l, 30);
    CVXCHECK(t, l.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, sll_int_count(&l) == 2);
    CVXCHECK(t, sll_int_front(&l) == 10);
    CVX_MALLOC_RESET();
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    sll_int__proxy_push_front(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_push_back(struct cvxtest *t)
{
    struct slinked_int l;
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 10);
    sll_int_push_back(&l, 20);
    sll_int_push_back(&l, 30);
    CVXCHECK(t, sll_int_count(&l) == 3);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, sll_int_front(&l) == 10);
    CVXCHECK(t, sll_int_back(&l) == 30);
    sll_int_drop(&l);
    // Alloc failure on empty list
    sll_int_init(&l, NULL);
    CVX_MALLOC_FAIL_NEXT();
    sll_int_push_back(&l, 42);
    CVXCHECK(t, l.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, sll_int_count(&l) == 0);
    CVX_MALLOC_RESET();
    sll_int_drop(&l);
    // Alloc failure with existing elements
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 10);
    sll_int_push_back(&l, 20);
    CVX_MALLOC_FAIL_NEXT();
    sll_int_push_back(&l, 30);
    CVXCHECK(t, l.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, sll_int_count(&l) == 2);
    CVXCHECK(t, sll_int_back(&l) == 20);
    CVX_MALLOC_RESET();
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    sll_int__proxy_push_back(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_push_at(struct cvxtest *t)
{
    struct slinked_int l;
    // Middle insertion
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 1);
    sll_int_push_back(&l, 3);
    sll_int_push_at(&l, 2, 1);
    CVXCHECK(t, sll_int_count(&l) == 3);
    CVXCHECK(t, sll_int_get(&l, 0) == 1);
    CVXCHECK(t, sll_int_get(&l, 1) == 2);
    CVXCHECK(t, sll_int_get(&l, 2) == 3);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    sll_int_drop(&l);
    // Head insertion
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 2);
    sll_int_push_at(&l, 1, 0);
    CVXCHECK(t, sll_int_front(&l) == 1);
    CVXCHECK(t, sll_int_back(&l) == 2);
    sll_int_drop(&l);
    // Tail insertion
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 1);
    sll_int_push_at(&l, 2, 1);
    CVXCHECK(t, sll_int_back(&l) == 2);
    sll_int_drop(&l);
    // Deep traversal (index > 1)
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 10);
    sll_int_push_back(&l, 20);
    sll_int_push_back(&l, 30);
    sll_int_push_at(&l, 99, 2);
    CVXCHECK(t, sll_int_count(&l) == 4);
    CVXCHECK(t, sll_int_get(&l, 2) == 99);
    CVXCHECK(t, sll_int_get(&l, 3) == 30);
    sll_int_drop(&l);
    // Out of range
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 1);
    sll_int_push_at(&l, 99, 5);
    CVXCHECK(t, l.super.flag == CVX_FLAG_RANGE);
    sll_int_drop(&l);
    // Alloc failure (middle path)
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 10);
    sll_int_push_back(&l, 20);
    CVX_MALLOC_FAIL_NEXT();
    sll_int_push_at(&l, 99, 1);
    CVXCHECK(t, l.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, sll_int_count(&l) == 2);
    CVX_MALLOC_RESET();
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    sll_int__proxy_push_at(col, 1, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_pop_front(struct cvxtest *t)
{
    struct slinked_int l;
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 10);
    sll_int_push_back(&l, 20);
    int out = sll_int_pop_front(&l);
    CVXCHECK(t, out == 10);
    CVXCHECK(t, sll_int_count(&l) == 1);
    CVXCHECK(t, sll_int_front(&l) == 20);
    sll_int_drop(&l);
    // Pop to empty: head and tail must both be NULL
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 1);
    sll_int_pop_front(&l);
    CVXCHECK(t, sll_int_count(&l) == 0);
    CVXCHECK(t, l.head == NULL);
    CVXCHECK(t, l.tail == NULL);
    sll_int_drop(&l);
    // Empty
    sll_int_init(&l, NULL);
    sll_int_pop_front(&l);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    sll_int__proxy_pop_front(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_pop_back(struct cvxtest *t)
{
    struct slinked_int l;
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 10);
    sll_int_push_back(&l, 20);
    int out = sll_int_pop_back(&l);
    CVXCHECK(t, out == 20);
    CVXCHECK(t, sll_int_count(&l) == 1);
    CVXCHECK(t, sll_int_back(&l) == 10);
    sll_int_drop(&l);
    // Pop to empty: head and tail must both be NULL
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 1);
    sll_int_pop_back(&l);
    CVXCHECK(t, sll_int_count(&l) == 0);
    CVXCHECK(t, l.head == NULL);
    CVXCHECK(t, l.tail == NULL);
    sll_int_drop(&l);
    // Multi-element: exercises traversal loop
    sll_int_init(&l, NULL);
    sll_fill3(&l);
    int v = sll_int_pop_back(&l);
    CVXCHECK(t, v == 30);
    CVXCHECK(t, sll_int_count(&l) == 2);
    CVXCHECK(t, sll_int_back(&l) == 20);
    sll_int_drop(&l);
    // Empty
    sll_int_init(&l, NULL);
    sll_int_pop_back(&l);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    sll_int__proxy_pop_back(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_pop_at(struct cvxtest *t)
{
    struct slinked_int l;
    // Middle
    sll_int_init(&l, NULL);
    sll_fill3(&l);
    int out = sll_int_pop_at(&l, 1);
    CVXCHECK(t, out == 20);
    CVXCHECK(t, sll_int_count(&l) == 2);
    CVXCHECK(t, sll_int_get(&l, 0) == 10);
    CVXCHECK(t, sll_int_get(&l, 1) == 30);
    sll_int_drop(&l);
    // Front (delegates to pop_front)
    sll_int_init(&l, NULL);
    sll_fill3(&l);
    int v = sll_int_pop_at(&l, 0);
    CVXCHECK(t, v == 10);
    CVXCHECK(t, sll_int_count(&l) == 2);
    CVXCHECK(t, sll_int_front(&l) == 20);
    sll_int_drop(&l);
    // Back (delegates to pop_back)
    sll_int_init(&l, NULL);
    sll_fill3(&l);
    v = sll_int_pop_at(&l, 2);
    CVXCHECK(t, v == 30);
    CVXCHECK(t, sll_int_count(&l) == 2);
    CVXCHECK(t, sll_int_back(&l) == 20);
    sll_int_drop(&l);
    // Deep traversal (index > 1)
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 10);
    sll_int_push_back(&l, 20);
    sll_int_push_back(&l, 30);
    sll_int_push_back(&l, 40);
    v = sll_int_pop_at(&l, 2);
    CVXCHECK(t, v == 30);
    CVXCHECK(t, sll_int_count(&l) == 3);
    CVXCHECK(t, sll_int_get(&l, 2) == 40);
    sll_int_drop(&l);
    // Out of range
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 1);
    sll_int_pop_at(&l, 5);
    CVXCHECK(t, l.super.flag == CVX_FLAG_RANGE);
    sll_int_drop(&l);
    // Empty
    sll_int_init(&l, NULL);
    sll_int_pop_at(&l, 0);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    sll_int__proxy_pop_at(col, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_replace_front(struct cvxtest *t)
{
    struct slinked_int l;
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 10);
    sll_int_push_back(&l, 20);
    int old = sll_int_replace_front(&l, 99);
    CVXCHECK(t, old == 10);
    CVXCHECK(t, sll_int_front(&l) == 99);
    CVXCHECK(t, sll_int_count(&l) == 2);
    sll_int_drop(&l);
    // When empty
    sll_int_init(&l, NULL);
    sll_int_replace_front(&l, 42);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, sll_int_count(&l) == 0);
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    sll_int__proxy_replace_front(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_sll_int_replace_back(struct cvxtest *t)
{
    struct slinked_int l;
    sll_int_init(&l, NULL);
    sll_int_push_back(&l, 5);
    sll_int_push_back(&l, 10);
    int old = sll_int_replace_back(&l, 99);
    CVXCHECK(t, old == 10);
    CVXCHECK(t, sll_int_back(&l) == 99);
    CVXCHECK(t, sll_int_count(&l) == 2);
    sll_int_drop(&l);
    // When empty
    sll_int_init(&l, NULL);
    sll_int_replace_back(&l, 42);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, sll_int_count(&l) == 0);
    sll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    sll_int__proxy_replace_back(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static struct cvxresult run_slinked_list_tests(void)
{
    struct cvxtest t = { 0 };

    printf("slinked_list\n");

    CVXRUN(&t, test_sll_int_init);
    CVXRUN(&t, test_sll_int_clone);
    CVXRUN(&t, test_sll_int_drop);
    CVXRUN(&t, test_sll_int_count);
    CVXRUN(&t, test_sll_int_empty);
    CVXRUN(&t, test_sll_int_front);
    CVXRUN(&t, test_sll_int_back);
    CVXRUN(&t, test_sll_int_get);
    CVXRUN(&t, test_sll_int_push_front);
    CVXRUN(&t, test_sll_int_push_back);
    CVXRUN(&t, test_sll_int_push_at);
    CVXRUN(&t, test_sll_int_pop_front);
    CVXRUN(&t, test_sll_int_pop_back);
    CVXRUN(&t, test_sll_int_pop_at);
    CVXRUN(&t, test_sll_int_replace_front);
    CVXRUN(&t, test_sll_int_replace_back);

    return CVXSUMMARY(&t);
}

#endif /* SLINKED_LIST_TESTS_H */
