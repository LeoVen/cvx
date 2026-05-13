#ifndef DLINKED_LIST_TESTS_H
#define DLINKED_LIST_TESTS_H

#include "cvx/flags.h"
#include "tests/alloc.h"
#include "tests/cvxtest.h"
#include "tests/cvxtestutils.h"
#include "tests/implementations.h"

static void dll_fill3(struct dlinked_int *col)
{
    dll_int_push_back(col, 10);
    dll_int_push_back(col, 20);
    dll_int_push_back(col, 30);
}

static void test_dll_int_init(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    CVXCHECK(t, l.super.tag == 88);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, l.head == NULL);
    CVXCHECK(t, l.tail == NULL);
    CVXCHECK(t, l.count == 0);
    CVXCHECK(t, l.vtabv == NULL);
    dll_int_drop(&l);
    // With vtabv
    dll_int_init(&l, dll_int_vtabv_full);
    CVXCHECK(t, l.vtabv == dll_int_vtabv_full);
    dll_int_drop(&l);
}

static void test_dll_int_clone(struct cvxtest *t)
{
    // Clone empty list
    struct dlinked_int orig, clone;
    dll_int_init(&orig, NULL);
    dll_int_clone(&orig, &clone);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, clone.super.tag == 88);
    CVXCHECK(t, clone.count == 0);
    dll_int_drop(&orig);
    dll_int_drop(&clone);
    // Clone with values — nodes distinct, prev links correct
    dll_int_init(&orig, NULL);
    dll_fill3(&orig);
    dll_int_clone(&orig, &clone);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, clone.count == 3);
    CVXCHECK(t, dll_int_get(&clone, 0) == 10);
    CVXCHECK(t, dll_int_get(&clone, 1) == 20);
    CVXCHECK(t, dll_int_get(&clone, 2) == 30);
    CVXCHECK(t, clone.head != orig.head);
    CVXCHECK(t, clone.head->prev == NULL);
    CVXCHECK(t, clone.head->next->prev == clone.head);
    CVXCHECK(t, clone.tail->next == NULL);
    dll_int_push_back(&orig, 40);
    CVXCHECK(t, clone.count == 3);
    dll_int_drop(&orig);
    dll_int_drop(&clone);
    // vtabv->clone called during clone
    CVX_TEST_COUNTER_CLONE_RESET();
    dll_int_init(&orig, dll_int_vtabv_full);
    dll_fill3(&orig);
    dll_int_clone(&orig, &clone);
    CVX_TEST_COUNTER_CLONE(t, 3);
    CVXCHECK(t, orig.vtabv == clone.vtabv);
    dll_int_drop(&orig);
    dll_int_drop(&clone);
    // Null vtabv — no crash
    dll_int_init(&orig, NULL);
    dll_fill3(&orig);
    dll_int_clone(&orig, &clone);
    CVXCHECK(t, clone.count == 3);
    dll_int_drop(&orig);
    dll_int_drop(&clone);
    // Alloc failure: first node fails
    dll_int_init(&orig, NULL);
    dll_int_push_back(&orig, 10);
    dll_int_push_back(&orig, 20);
    CVX_MALLOC_FAIL_NEXT();
    dll_int_clone(&orig, &clone);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, orig.super.flag == CVX_FLAG_ALLOC);
    CVX_MALLOC_RESET();
    dll_int_drop(&orig);
    dll_int_drop(&clone);
    // Alloc failure: second node fails
    dll_int_init(&orig, NULL);
    dll_int_push_back(&orig, 10);
    dll_int_push_back(&orig, 20);
    CVX_MALLOC_FAIL_AFTER(1);
    dll_int_clone(&orig, &clone);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_ALLOC);
    CVX_MALLOC_RESET();
    dll_int_drop(&orig);
    dll_int_drop(&clone);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    struct dlinked_int dummy;
    dll_int_init(&dummy, NULL);
    dll_int__proxy_clone(col, (cvx_container *)&dummy);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
    dll_int_drop(&dummy);
}

static void test_dll_int_drop(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_int_drop(&l);
    // Drop with elements; vtabv->drop called for each
    CVX_TEST_COUNTER_DROP_RESET();
    dll_int_init(&l, dll_int_vtabv_full);
    dll_int_push_back(&l, 1);
    dll_int_push_back(&l, 2);
    dll_int_push_back(&l, 3);
    dll_int_drop(&l);
    CVX_TEST_COUNTER_DROP(t, 3);
    // Drop with NULL vtabv — no crash
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 1);
    dll_int_push_back(&l, 2);
    dll_int_push_back(&l, 3);
    dll_int_drop(&l);
    // Drop NULL pointer — no crash
    dll_int_drop(NULL);
    // Random sequence of init / drop / clone must always work
    struct dlinked_int b;
    dll_int_init(&l, NULL);
    for (int i = 0; i < 100; i++)
    {
        int choice = rand() % 3;
        if (choice == 0)
            dll_int_init(&l, NULL);
        else if (choice == 1)
            dll_int_drop(&l);
        else
        {
            dll_int_clone(&l, &b);
            dll_int_drop(&b);
        }
    }
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_count(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    CVXCHECK(t, dll_int_count(&l) == 0);
    dll_int_push_back(&l, 1);
    CVXCHECK(t, dll_int_count(&l) == 1);
    dll_int_push_back(&l, 2);
    CVXCHECK(t, dll_int_count(&l) == 2);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    size_t r = dll_int__proxy_count(col);
    CVXCHECK(t, r == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_empty(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    CVXCHECK(t, dll_int_empty(&l) == true);
    dll_int_push_back(&l, 1);
    CVXCHECK(t, dll_int_empty(&l) == false);
    dll_int_pop_front(&l);
    CVXCHECK(t, dll_int_empty(&l) == true);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    bool r = dll_int__proxy_empty(col);
    CVXCHECK(t, r == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_front(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    CVXCHECK(t, dll_int_front(&l) == 10);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    dll_int_drop(&l);
    // Empty → CVX_FLAG_EMPTY
    dll_int_init(&l, NULL);
    dll_int_front(&l);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    int r = dll_int__proxy_front(col);
    CVXCHECK(t, r == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_back(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    CVXCHECK(t, dll_int_back(&l) == 30);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    dll_int_drop(&l);
    // Empty → CVX_FLAG_EMPTY
    dll_int_init(&l, NULL);
    dll_int_back(&l);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    int r = dll_int__proxy_back(col);
    CVXCHECK(t, r == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_get(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    CVXCHECK(t, dll_int_get(&l, 0) == 10);
    CVXCHECK(t, dll_int_get(&l, 1) == 20);
    CVXCHECK(t, dll_int_get(&l, 2) == 30);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    dll_int_drop(&l);
    // Out of range
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 1);
    dll_int_get(&l, 1);
    CVXCHECK(t, l.super.flag == CVX_FLAG_RANGE);
    dll_int_drop(&l);
    // Empty → CVX_FLAG_RANGE
    dll_int_init(&l, NULL);
    dll_int_get(&l, 0);
    CVXCHECK(t, l.super.flag == CVX_FLAG_RANGE);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    int r = dll_int__proxy_get(col, 0);
    CVXCHECK(t, r == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_push_front(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 2);
    dll_int_push_back(&l, 3);
    dll_int_push_front(&l, 1);
    CVXCHECK(t, dll_int_count(&l) == 3);
    CVXCHECK(t, dll_int_front(&l) == 1);
    CVXCHECK(t, dll_int_back(&l) == 3);
    CVXCHECK(t, dll_int_get(&l, 1) == 2);
    CVXCHECK(t, l.head->prev == NULL);
    CVXCHECK(t, l.head->next->prev == l.head);
    dll_int_drop(&l);
    // Alloc failure
    dll_int_init(&l, NULL);
    CVX_MALLOC_FAIL_NEXT();
    dll_int_push_front(&l, 1);
    CVXCHECK(t, l.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, dll_int_count(&l) == 0);
    CVX_MALLOC_RESET();
    dll_int_drop(&l);
    // Alloc failure with existing elements
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 10);
    dll_int_push_back(&l, 20);
    CVX_MALLOC_FAIL_NEXT();
    dll_int_push_front(&l, 30);
    CVXCHECK(t, l.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, dll_int_count(&l) == 2);
    CVXCHECK(t, dll_int_front(&l) == 10);
    CVXCHECK(t, dll_int_back(&l) == 20);
    CVX_MALLOC_RESET();
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_push_front(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_push_back(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    CVXCHECK(t, dll_int_count(&l) == 3);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_front(&l) == 10);
    CVXCHECK(t, dll_int_back(&l) == 30);
    // Prev links
    CVXCHECK(t, l.head->prev == NULL);
    CVXCHECK(t, l.head->next->prev == l.head);
    CVXCHECK(t, l.tail->prev == l.head->next);
    dll_int_drop(&l);
    // Many pushes
    dll_int_init(&l, NULL);
    for (int i = 0; i < 100; i++)
        dll_int_push_back(&l, i);
    CVXCHECK(t, dll_int_count(&l) == 100);
    CVXCHECK(t, dll_int_front(&l) == 0);
    CVXCHECK(t, dll_int_back(&l) == 99);
    dll_int_drop(&l);
    // Alloc failure
    dll_int_init(&l, NULL);
    CVX_MALLOC_FAIL_NEXT();
    dll_int_push_back(&l, 42);
    CVXCHECK(t, l.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, dll_int_count(&l) == 0);
    CVX_MALLOC_RESET();
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_push_back(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_push_at(struct cvxtest *t)
{
    struct dlinked_int l;
    // Middle
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 1);
    dll_int_push_back(&l, 3);
    dll_int_push_at(&l, 2, 1);
    CVXCHECK(t, dll_int_count(&l) == 3);
    CVXCHECK(t, dll_int_get(&l, 0) == 1);
    CVXCHECK(t, dll_int_get(&l, 1) == 2);
    CVXCHECK(t, dll_int_get(&l, 2) == 3);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, l.head->next->prev == l.head);
    CVXCHECK(t, l.tail->prev->next == l.tail);
    dll_int_drop(&l);
    // Head (index 0)
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 2);
    dll_int_push_at(&l, 1, 0);
    CVXCHECK(t, dll_int_front(&l) == 1);
    CVXCHECK(t, dll_int_back(&l) == 2);
    dll_int_drop(&l);
    // Tail (index == count)
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 1);
    dll_int_push_at(&l, 2, 1);
    CVXCHECK(t, dll_int_back(&l) == 2);
    dll_int_drop(&l);
    // Out of range
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 1);
    dll_int_push_at(&l, 99, 5);
    CVXCHECK(t, l.super.flag == CVX_FLAG_RANGE);
    dll_int_drop(&l);
    // Alloc failure (middle)
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 10);
    dll_int_push_back(&l, 20);
    CVX_MALLOC_FAIL_NEXT();
    dll_int_push_at(&l, 99, 1);
    CVXCHECK(t, l.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, dll_int_count(&l) == 2);
    CVXCHECK(t, dll_int_front(&l) == 10);
    CVXCHECK(t, dll_int_back(&l) == 20);
    CVX_MALLOC_RESET();
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_push_at(col, 1, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_pop_front(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 10);
    dll_int_push_back(&l, 20);
    int out = dll_int_pop_front(&l);
    CVXCHECK(t, out == 10);
    CVXCHECK(t, dll_int_count(&l) == 1);
    CVXCHECK(t, dll_int_front(&l) == 20);
    CVXCHECK(t, l.head->prev == NULL);
    dll_int_drop(&l);
    // To empty: head/tail reset
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 1);
    dll_int_pop_front(&l);
    CVXCHECK(t, dll_int_count(&l) == 0);
    CVXCHECK(t, l.head == NULL);
    CVXCHECK(t, l.tail == NULL);
    dll_int_drop(&l);
    // Empty → CVX_FLAG_EMPTY
    dll_int_init(&l, NULL);
    dll_int_pop_front(&l);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_pop_front(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_pop_back(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 10);
    dll_int_push_back(&l, 20);
    int out = dll_int_pop_back(&l);
    CVXCHECK(t, out == 20);
    CVXCHECK(t, dll_int_count(&l) == 1);
    CVXCHECK(t, dll_int_back(&l) == 10);
    CVXCHECK(t, l.tail->next == NULL);
    dll_int_drop(&l);
    // To empty
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 1);
    dll_int_pop_back(&l);
    CVXCHECK(t, dll_int_count(&l) == 0);
    CVXCHECK(t, l.head == NULL);
    CVXCHECK(t, l.tail == NULL);
    dll_int_drop(&l);
    // Empty → CVX_FLAG_EMPTY
    dll_int_init(&l, NULL);
    dll_int_pop_back(&l);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_pop_back(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_pop_at(struct cvxtest *t)
{
    struct dlinked_int l;
    // Middle
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    int out = dll_int_pop_at(&l, 1);
    CVXCHECK(t, out == 20);
    CVXCHECK(t, dll_int_count(&l) == 2);
    CVXCHECK(t, dll_int_get(&l, 0) == 10);
    CVXCHECK(t, dll_int_get(&l, 1) == 30);
    CVXCHECK(t, l.tail->prev == l.head);
    CVXCHECK(t, l.head->next == l.tail);
    dll_int_drop(&l);
    // Front (delegates to pop_front)
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    int v = dll_int_pop_at(&l, 0);
    CVXCHECK(t, v == 10);
    CVXCHECK(t, dll_int_count(&l) == 2);
    CVXCHECK(t, dll_int_front(&l) == 20);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    dll_int_drop(&l);
    // Back (delegates to pop_back)
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    v = dll_int_pop_at(&l, 2);
    CVXCHECK(t, v == 30);
    CVXCHECK(t, dll_int_count(&l) == 2);
    CVXCHECK(t, dll_int_back(&l) == 20);
    CVXCHECK(t, l.super.flag == CVX_FLAG_OK);
    dll_int_drop(&l);
    // Out of range
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 1);
    dll_int_pop_at(&l, 5);
    CVXCHECK(t, l.super.flag == CVX_FLAG_RANGE);
    dll_int_drop(&l);
    // Empty → CVX_FLAG_EMPTY
    dll_int_init(&l, NULL);
    dll_int_pop_at(&l, 0);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_pop_at(col, 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_replace_front(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 10);
    dll_int_push_back(&l, 20);
    int old = dll_int_replace_front(&l, 99);
    CVXCHECK(t, old == 10);
    CVXCHECK(t, dll_int_front(&l) == 99);
    CVXCHECK(t, dll_int_count(&l) == 2);
    dll_int_drop(&l);
    // Empty → CVX_FLAG_EMPTY
    dll_int_init(&l, NULL);
    dll_int_replace_front(&l, 42);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, dll_int_count(&l) == 0);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_replace_front(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_replace_back(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_int_push_back(&l, 5);
    dll_int_push_back(&l, 10);
    int old = dll_int_replace_back(&l, 99);
    CVXCHECK(t, old == 10);
    CVXCHECK(t, dll_int_back(&l) == 99);
    CVXCHECK(t, dll_int_count(&l) == 2);
    dll_int_drop(&l);
    // Empty → CVX_FLAG_EMPTY
    dll_int_init(&l, NULL);
    dll_int_replace_back(&l, 42);
    CVXCHECK(t, l.super.flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, dll_int_count(&l) == 0);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_replace_back(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_init_start(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter it = dll_int_iter_init_start(&l);
    CVXCHECK(t, it.super.tag == (size_t)DLL_ITER_TAG);
    CVXCHECK(t, it.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 0);
    CVXCHECK(t, it.cursor == l.head);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    struct dlinked_int_iter err = dll_int__proxy_iter_init_start(col);
    CVXCHECK(t, err.super.flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_init_end(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter it = dll_int_iter_init_end(&l);
    CVXCHECK(t, it.super.tag == (size_t)DLL_ITER_TAG);
    CVXCHECK(t, it.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, it.index == 3);
    CVXCHECK(t, it.cursor == NULL);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    struct dlinked_int_iter err = dll_int__proxy_iter_init_end(col);
    CVXCHECK(t, err.super.flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_start(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_start(&l);
    CVXCHECK(t, iter != NULL);
    if (iter)
    {
        CVXCHECK(t, iter->super.tag == (size_t)DLL_ITER_TAG);
        CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
        CVXCHECK(t, iter->index == 0);
        dll_int_iter_drop(iter);
    }
    dll_int_drop(&l);
    // Alloc failure → NULL
    dll_int_init(&l, NULL);
    CVX_MALLOC_FAIL_NEXT();
    iter = dll_int_iter_start(&l);
    CVXCHECK(t, iter == NULL);
    CVX_MALLOC_RESET();
    dll_int_drop(&l);
    // Guard: wrong tag → NULL
    MAKE_INVALID_CONTAINER(col);
    cvx_container *it = dll_int__proxy_iter_start(col);
    CVXCHECK(t, it == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_end(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_end(&l);
    CVXCHECK(t, iter != NULL);
    if (iter)
    {
        CVXCHECK(t, iter->super.tag == (size_t)DLL_ITER_TAG);
        CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
        CVXCHECK(t, iter->index == 3);
        CVXCHECK(t, iter->cursor == NULL);
        dll_int_iter_drop(iter);
    }
    dll_int_drop(&l);
    // Alloc failure → NULL
    dll_int_init(&l, NULL);
    CVX_MALLOC_FAIL_NEXT();
    iter = dll_int_iter_end(&l);
    CVXCHECK(t, iter == NULL);
    CVX_MALLOC_RESET();
    dll_int_drop(&l);
    // Guard: wrong tag → NULL
    MAKE_INVALID_CONTAINER(col);
    cvx_container *it = dll_int__proxy_iter_end(col);
    CVXCHECK(t, it == NULL);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_drop(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_start(&l);
    CVXCHECK(t, iter != NULL);
    if (iter)
        dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_drop(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_at_start(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_start(&l);
    CVXCHECK(t, dll_int_iter_at_start(iter) == true);
    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_at_start(iter) == false);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Empty list: at_start and at_end simultaneously
    dll_int_init(&l, NULL);
    iter = dll_int_iter_start(&l);
    CVXCHECK(t, dll_int_iter_at_start(iter) == true);
    CVXCHECK(t, dll_int_iter_at_end(iter) == true);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    bool r = dll_int__proxy_iter_at_start(col);
    CVXCHECK(t, r == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_at_end(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_end(&l);
    CVXCHECK(t, dll_int_iter_at_end(iter) == true);
    dll_int_iter_drop(iter);
    iter = dll_int_iter_start(&l);
    CVXCHECK(t, dll_int_iter_at_end(iter) == false);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    bool r = dll_int__proxy_iter_at_end(col);
    CVXCHECK(t, r == false);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_count(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    for (int i = 0; i < 5; i++)
        dll_int_push_back(&l, i);
    struct dlinked_int_iter *iter = dll_int_iter_start(&l);
    CVXCHECK(t, dll_int_iter_count(iter) == 5);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    size_t r = dll_int__proxy_iter_count(col);
    CVXCHECK(t, r == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_to_start(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_start(&l);
    dll_int_iter_next(iter);
    dll_int_iter_next(iter);
    dll_int_iter_to_start(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_value(iter) == 10);
    CVXCHECK(t, iter->cursor == l.head);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_to_start(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_to_end(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_start(&l);
    dll_int_iter_to_end(iter);
    CVXCHECK(t, dll_int_iter_at_end(iter) == true);
    CVXCHECK(t, dll_int_iter_index(iter) == 3);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, iter->cursor == NULL);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_to_end(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_next(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_start(&l);
    CVXCHECK(t, dll_int_iter_index(iter) == 0);
    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 2);
    dll_int_iter_drop(iter);
    // At end → CVX_FLAG_RANGE
    iter = dll_int_iter_end(&l);
    dll_int_iter_next(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_next(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_prev(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    // From end: cursor=NULL → tail
    struct dlinked_int_iter *iter = dll_int_iter_end(&l);
    CVXCHECK(t, dll_int_iter_index(iter) == 3);
    dll_int_iter_prev(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 2);
    CVXCHECK(t, dll_int_iter_value(iter) == 30);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    dll_int_iter_prev(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    CVXCHECK(t, dll_int_iter_value(iter) == 20);
    dll_int_iter_drop(iter);
    // Round-trip: next/next/prev
    iter = dll_int_iter_start(&l);
    dll_int_iter_next(iter);
    dll_int_iter_next(iter);
    dll_int_iter_prev(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    CVXCHECK(t, dll_int_iter_value(iter) == 20);
    dll_int_iter_drop(iter);
    // At start → CVX_FLAG_RANGE
    iter = dll_int_iter_start(&l);
    dll_int_iter_prev(iter);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_prev(col);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_forward(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_start(&l);
    dll_int_iter_forward(iter, 2);
    CVXCHECK(t, dll_int_iter_index(iter) == 2);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_value(iter) == 30);
    dll_int_iter_drop(iter);
    // Clamp: more steps than remaining
    iter = dll_int_iter_start(&l);
    dll_int_iter_forward(iter, 100);
    CVXCHECK(t, dll_int_iter_index(iter) == 3);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_at_end(iter) == true);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_forward(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_backward(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_end(&l);
    dll_int_iter_backward(iter, 2);
    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_value(iter) == 20);
    dll_int_iter_drop(iter);
    // Clamp: more steps than available
    iter = dll_int_iter_end(&l);
    dll_int_iter_backward(iter, 100);
    CVXCHECK(t, dll_int_iter_index(iter) == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_OK);
    CVXCHECK(t, dll_int_iter_at_start(iter) == true);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    dll_int__proxy_iter_backward(col, 1);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_value(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_start(&l);
    CVXCHECK(t, dll_int_iter_value(iter) == 10);
    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_value(iter) == 20);
    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_value(iter) == 30);
    dll_int_iter_drop(iter);
    // At end → CVX_FLAG_RANGE
    iter = dll_int_iter_end(&l);
    int val = dll_int_iter_value(iter);
    CVXCHECK(t, val == 0);
    CVXCHECK(t, iter->super.flag == CVX_FLAG_RANGE);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    int r = dll_int__proxy_iter_value(col);
    CVXCHECK(t, r == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static void test_dll_int_iter_index(struct cvxtest *t)
{
    struct dlinked_int l;
    dll_int_init(&l, NULL);
    dll_fill3(&l);
    struct dlinked_int_iter *iter = dll_int_iter_start(&l);
    CVXCHECK(t, dll_int_iter_index(iter) == 0);
    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 1);
    dll_int_iter_next(iter);
    CVXCHECK(t, dll_int_iter_index(iter) == 2);
    dll_int_iter_drop(iter);
    dll_int_drop(&l);
    // Guard: wrong tag
    MAKE_INVALID_CONTAINER(col);
    size_t r = dll_int__proxy_iter_index(col);
    CVXCHECK(t, r == 0);
    CVXCHECK(t, col->flag == CVX_FLAG_WRONG_TAG);
}

static struct cvxresult run_dlinked_list_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dlinked_list\n");

    CVXRUN(&t, test_dll_int_init);
    CVXRUN(&t, test_dll_int_clone);
    CVXRUN(&t, test_dll_int_drop);
    CVXRUN(&t, test_dll_int_count);
    CVXRUN(&t, test_dll_int_empty);
    CVXRUN(&t, test_dll_int_front);
    CVXRUN(&t, test_dll_int_back);
    CVXRUN(&t, test_dll_int_get);
    CVXRUN(&t, test_dll_int_push_front);
    CVXRUN(&t, test_dll_int_push_back);
    CVXRUN(&t, test_dll_int_push_at);
    CVXRUN(&t, test_dll_int_pop_front);
    CVXRUN(&t, test_dll_int_pop_back);
    CVXRUN(&t, test_dll_int_pop_at);
    CVXRUN(&t, test_dll_int_replace_front);
    CVXRUN(&t, test_dll_int_replace_back);
    CVXRUN(&t, test_dll_int_iter_init_start);
    CVXRUN(&t, test_dll_int_iter_init_end);
    CVXRUN(&t, test_dll_int_iter_start);
    CVXRUN(&t, test_dll_int_iter_end);
    CVXRUN(&t, test_dll_int_iter_drop);
    CVXRUN(&t, test_dll_int_iter_at_start);
    CVXRUN(&t, test_dll_int_iter_at_end);
    CVXRUN(&t, test_dll_int_iter_count);
    CVXRUN(&t, test_dll_int_iter_to_start);
    CVXRUN(&t, test_dll_int_iter_to_end);
    CVXRUN(&t, test_dll_int_iter_next);
    CVXRUN(&t, test_dll_int_iter_prev);
    CVXRUN(&t, test_dll_int_iter_forward);
    CVXRUN(&t, test_dll_int_iter_backward);
    CVXRUN(&t, test_dll_int_iter_value);
    CVXRUN(&t, test_dll_int_iter_index);

    return CVXSUMMARY(&t);
}

#endif /* DLINKED_LIST_TESTS_H */
