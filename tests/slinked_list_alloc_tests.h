#ifndef SLINKED_LIST_ALLOC_TESTS_H
#define SLINKED_LIST_ALLOC_TESTS_H

// alloc.h must be included before implementations.h so that the #define
// malloc / calloc macros are in effect when slinked_list.h is compiled.
#include "tests/alloc.h"

#include "tests/cvxtest.h"
#include "tests/implementations.h"

/* ---- copy (stack-allocated) ---- */

// sll_int_copy() allocates one node per element. Failing the second node must
// roll back the first (calling vtabv->drop on it) and return a struct flagged
// CVX_FLAG_ALLOC with head/tail/count all zeroed.
static void test_sll_int_alloc_copy_node_fails_with_drop(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    cvx_container *src = sll_int_new_with(sll_int_vtabv_full);
    CVXCHECK(t, src != NULL);
    if (!src)
        return;

    sll_int_push_back(src, 10);
    sll_int_push_back(src, 20);

    struct slinked_int *self = (struct slinked_int *)src;

    // 1 node alloc succeeds (first node copied), then second fails.
    CVX_MALLOC_FAIL_AFTER(1);
    struct slinked_int result = sll_int_copy(self);

    CVXCHECK(t, result.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, result.head == NULL);
    CVXCHECK(t, result.tail == NULL);
    CVXCHECK(t, result.count == 0);
    // vtabv->drop must have been called once for the rolled-back first node.
    CVX_TEST_COUNTER_DROP(t, 1);

    CVX_MALLOC_RESET();
    sll_int_drop(src);
}

/* ---- new ---- */

// sll_int_new() performs one allocation (the struct). Failing it returns NULL.
static void test_sll_int_alloc_new(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    cvx_container *s = sll_int_new();
    CVXCHECK(t, s == NULL);
    CVX_MALLOC_RESET();
}

/* ---- new_with ---- */

// sll_int_new_with() also performs one allocation. Failing it returns NULL.
static void test_sll_int_alloc_new_with(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    cvx_container *s = sll_int_new_with(NULL);
    CVXCHECK(t, s == NULL);
    CVX_MALLOC_RESET();
}

/* ---- clone ---- */

// clone() allocates: (1) a new container struct, then (2+) one node per element.
// Failing the container struct returns NULL.
static void test_sll_int_alloc_clone_struct_fails(struct cvxtest *t)
{
    cvx_container *s = sll_int_new();
    CVXCHECK(t, s != NULL);
    if (!s)
        return;

    sll_int_push_back(s, 10);
    sll_int_push_back(s, 20);

    CVX_MALLOC_FAIL_NEXT();
    cvx_container *copy = sll_int_clone(s);
    CVXCHECK(t, copy == NULL);

    CVX_MALLOC_RESET();
    sll_int_drop(s);
}

// Failing the first node must return NULL and not leak the container.
static void test_sll_int_alloc_clone_first_node_fails(struct cvxtest *t)
{
    cvx_container *s = sll_int_new();
    CVXCHECK(t, s != NULL);
    if (!s)
        return;

    sll_int_push_back(s, 10);
    sll_int_push_back(s, 20);

    // 1 alloc succeeds (clone struct), then node alloc fails.
    CVX_MALLOC_FAIL_AFTER(1);
    cvx_container *copy = sll_int_clone(s);
    CVXCHECK(t, copy == NULL);

    CVX_MALLOC_RESET();
    sll_int_drop(s);
}

// Failing the second node must return NULL; the first copied node must be freed
// (run with AddressSanitizer to verify no leak).
static void test_sll_int_alloc_clone_second_node_fails(struct cvxtest *t)
{
    cvx_container *s = sll_int_new();
    CVXCHECK(t, s != NULL);
    if (!s)
        return;

    sll_int_push_back(s, 10);
    sll_int_push_back(s, 20);

    // 2 allocs succeed (clone struct + first node), then second node fails.
    CVX_MALLOC_FAIL_AFTER(2);
    cvx_container *copy = sll_int_clone(s);
    CVXCHECK(t, copy == NULL);

    CVX_MALLOC_RESET();
    sll_int_drop(s);
}

// A successful clone must be independent of the original.
static void test_sll_int_alloc_clone_success(struct cvxtest *t)
{
    cvx_container *s = sll_int_new();
    CVXCHECK(t, s != NULL);
    if (!s)
        return;

    sll_int_push_back(s, 10);
    sll_int_push_back(s, 20);

    cvx_container *copy = sll_int_clone(s);
    CVXCHECK(t, copy != NULL);
    if (!copy)
    {
        sll_int_drop(s);
        return;
    }

    sll_int_push_back(s, 30);
    CVXCHECK(t, sll_int_count(copy) == 2);
    CVXCHECK(t, sll_int_front(copy) == 10);
    CVXCHECK(t, sll_int_back(copy) == 20);

    sll_int_drop(copy);
    sll_int_drop(s);
}

/* ---- push_front ---- */

// Container created successfully, then push_front fails to alloc the node.
// List must remain consistent (empty and droppable).
static void test_sll_int_alloc_push_front_empty(struct cvxtest *t)
{
    cvx_container *s = sll_int_new();
    CVXCHECK(t, s != NULL);
    if (!s)
        return;

    CVX_MALLOC_FAIL_NEXT();
    sll_int_push_front(s, 42);
    CVXCHECK(t, s->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, sll_int_count(s) == 0);

    CVX_MALLOC_RESET();
    sll_int_drop(s);
}

// Two elements already in the list; push_front fails.
// Existing elements must be untouched.
static void test_sll_int_alloc_push_front_partial(struct cvxtest *t)
{
    cvx_container *s = sll_int_new();
    CVXCHECK(t, s != NULL);
    if (!s)
        return;

    sll_int_push_back(s, 10);
    sll_int_push_back(s, 20);
    CVXCHECK(t, sll_int_count(s) == 2);

    CVX_MALLOC_FAIL_NEXT();
    sll_int_push_front(s, 30);
    CVXCHECK(t, s->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, sll_int_count(s) == 2);
    CVXCHECK(t, sll_int_front(s) == 10);
    CVXCHECK(t, sll_int_back(s) == 20);

    CVX_MALLOC_RESET();
    sll_int_drop(s);
}

/* ---- push_back ---- */

static void test_sll_int_alloc_push_back_empty(struct cvxtest *t)
{
    cvx_container *s = sll_int_new();
    CVXCHECK(t, s != NULL);
    if (!s)
        return;

    CVX_MALLOC_FAIL_NEXT();
    sll_int_push_back(s, 42);
    CVXCHECK(t, s->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, sll_int_count(s) == 0);

    CVX_MALLOC_RESET();
    sll_int_drop(s);
}

static void test_sll_int_alloc_push_back_partial(struct cvxtest *t)
{
    cvx_container *s = sll_int_new();
    CVXCHECK(t, s != NULL);
    if (!s)
        return;

    sll_int_push_back(s, 10);
    sll_int_push_back(s, 20);

    CVX_MALLOC_FAIL_NEXT();
    sll_int_push_back(s, 30);
    CVXCHECK(t, s->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, sll_int_count(s) == 2);
    CVXCHECK(t, sll_int_front(s) == 10);
    CVXCHECK(t, sll_int_back(s) == 20);

    CVX_MALLOC_RESET();
    sll_int_drop(s);
}

/* ---- push_at (middle) ---- */

// push_at for index 0 delegates to push_front and push_at for the last index
// delegates to push_back; the middle path has its own malloc.
static void test_sll_int_alloc_push_at_middle(struct cvxtest *t)
{
    cvx_container *s = sll_int_new();
    CVXCHECK(t, s != NULL);
    if (!s)
        return;

    sll_int_push_back(s, 10);
    sll_int_push_back(s, 20);

    CVX_MALLOC_FAIL_NEXT();
    sll_int_push_at(s, 99, 1); // middle insertion
    CVXCHECK(t, s->flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, sll_int_count(s) == 2);
    CVXCHECK(t, sll_int_front(s) == 10);
    CVXCHECK(t, sll_int_back(s) == 20);

    CVX_MALLOC_RESET();
    sll_int_drop(s);
}

/* ---- iter_start ---- */

static void test_sll_int_alloc_iter_start_fails(struct cvxtest *t)
{
    cvx_container *s = sll_int_new();
    CVXCHECK(t, s != NULL);
    if (!s)
        return;

    CVX_MALLOC_FAIL_NEXT();
    cvx_container *it = sll_int_iter_start(s);
    CVXCHECK(t, it == NULL);

    CVX_MALLOC_RESET();
    sll_int_drop(s);
}

/* ---- runner ---- */

static int run_slinked_list_alloc_tests(void)
{
    struct cvxtest t = { 0 };

    printf("slinked_list (alloc errors)\n");

    CVXRUN(&t, test_sll_int_alloc_copy_node_fails_with_drop);

    CVXRUN(&t, test_sll_int_alloc_new);
    CVXRUN(&t, test_sll_int_alloc_new_with);

    CVXRUN(&t, test_sll_int_alloc_clone_struct_fails);
    CVXRUN(&t, test_sll_int_alloc_clone_first_node_fails);
    CVXRUN(&t, test_sll_int_alloc_clone_second_node_fails);
    CVXRUN(&t, test_sll_int_alloc_clone_success);

    CVXRUN(&t, test_sll_int_alloc_push_front_empty);
    CVXRUN(&t, test_sll_int_alloc_push_front_partial);

    CVXRUN(&t, test_sll_int_alloc_push_back_empty);
    CVXRUN(&t, test_sll_int_alloc_push_back_partial);

    CVXRUN(&t, test_sll_int_alloc_push_at_middle);

    CVXRUN(&t, test_sll_int_alloc_iter_start_fails);

    return CVXSUMMARY(&t);
}

#endif /* SLINKED_LIST_ALLOC_TESTS_H */
