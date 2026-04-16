#ifndef DLINKED_LIST_ALLOC_TESTS_H
#define DLINKED_LIST_ALLOC_TESTS_H

// alloc.h must be included before implementations.h so that the #define
// malloc / calloc macros are in effect when dlinked_list.h is compiled.
#include "tests/alloc.h"

#include "tests/cvxtest.h"
#include "tests/implementations.h"

/* ---- copy (stack-allocated) ---- */

// dll_int_copy() allocates one node per element. Failing the second node must
// roll back the first (calling vtabv->drop on it) and return a struct flagged
// CVX_FLAG_ALLOC with head/tail/count all zeroed.
static void test_dll_int_alloc_copy_node_fails_with_drop(struct cvxtest *t)
{
    CVX_TEST_COUNTER_DROP_RESET();

    struct dlinked_int *src = dll_int_new_with(dll_int_vtabv_full);
    CVXCHECK(t, src != NULL);
    if (!src)
        return;

    dll_int_push_back(src, 10);
    dll_int_push_back(src, 20);

    // 1 node alloc succeeds (first node copied to result), then second fails.
    CVX_MALLOC_FAIL_AFTER(1);
    struct dlinked_int result = dll_int_copy(src);

    CVXCHECK(t, result.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, result.head == NULL);
    CVXCHECK(t, result.tail == NULL);
    CVXCHECK(t, result.count == 0);
    // vtabv->drop must have been called once for the rolled-back first node.
    CVX_TEST_COUNTER_DROP(t, 1);

    CVX_MALLOC_RESET();
    dll_int_drop(src);
}

/* ---- new ---- */

// dll_int_new() performs one allocation (the struct itself).
// Failing it must return NULL.
static void test_dll_int_alloc_new(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    struct dlinked_int *d = dll_int_new();
    CVXCHECK(t, d == NULL);
    CVX_MALLOC_RESET();
}

/* ---- clone ---- */

// clone() needs: 1 alloc for the new container + 1 per node.
// Failing the container alloc must return NULL.
static void test_dll_int_alloc_clone_new_fails(struct cvxtest *t)
{
    struct dlinked_int *d = dll_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    dll_int_push_back(d, 10);
    dll_int_push_back(d, 20);

    // Allocs so far: 3 (1 struct + 2 nodes).
    // Fail the very next one (the clone's container).
    CVX_MALLOC_FAIL_NEXT();
    struct dlinked_int *copy = dll_int_clone(d);
    CVXCHECK(t, copy == NULL);

    CVX_MALLOC_RESET();
    dll_int_drop(d);
}

// Failing the first node copy must return NULL and not leak the container.
static void test_dll_int_alloc_clone_node_fails(struct cvxtest *t)
{
    struct dlinked_int *d = dll_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    dll_int_push_back(d, 10);
    dll_int_push_back(d, 20);

    // Allow 1 alloc (clone's container struct), then fail the first node.
    CVX_MALLOC_FAIL_AFTER(1);
    struct dlinked_int *copy = dll_int_clone(d);
    CVXCHECK(t, copy == NULL);

    CVX_MALLOC_RESET();
    dll_int_drop(d);
}

// Failing the second node must return NULL; the first copied node must be freed
// (run with AddressSanitizer to verify no leak).
static void test_dll_int_alloc_clone_second_node_fails(struct cvxtest *t)
{
    struct dlinked_int *d = dll_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    dll_int_push_back(d, 10);
    dll_int_push_back(d, 20);

    // Allow 2 allocs (container + first node), then fail the second node.
    CVX_MALLOC_FAIL_AFTER(2);
    struct dlinked_int *copy = dll_int_clone(d);
    CVXCHECK(t, copy == NULL);

    CVX_MALLOC_RESET();
    dll_int_drop(d);
}

// Successful clone must be independent of the original after allocation.
static void test_dll_int_alloc_clone_success(struct cvxtest *t)
{
    struct dlinked_int *d = dll_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    dll_int_push_back(d, 10);
    dll_int_push_back(d, 20);

    struct dlinked_int *copy = dll_int_clone(d);
    CVXCHECK(t, copy != NULL);
    if (!copy)
    {
        dll_int_drop(d);
        return;
    }

    // Mutation of the original must not affect the clone.
    dll_int_push_back(d, 30);
    CVXCHECK(t, dll_int_count(copy) == 2);
    CVXCHECK(t, dll_int_front(copy) == 10);
    CVXCHECK(t, dll_int_back(copy) == 20);

    dll_int_drop(copy);
    dll_int_drop(d);
}

/* ---- push_front ---- */

// The list is created successfully (1 alloc), then push_front fails (2nd alloc).
// The list must remain consistent and droppable.
static void test_dll_int_alloc_push_front(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_AFTER(1);
    struct dlinked_int *d = dll_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
    {
        CVX_MALLOC_RESET();
        return;
    }

    dll_int_push_front(d, 42);
    CVXCHECK(t, d->super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, dll_int_count(d) == 0);

    CVX_MALLOC_RESET();
    dll_int_drop(d);
}

// Two elements pushed successfully, then push_front fails.
// Existing elements must be untouched.
static void test_dll_int_alloc_push_front_partial(struct cvxtest *t)
{
    struct dlinked_int *d = dll_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    dll_int_push_back(d, 10);
    dll_int_push_back(d, 20);
    CVXCHECK(t, dll_int_count(d) == 2);

    CVX_MALLOC_FAIL_NEXT();
    dll_int_push_front(d, 30);
    CVXCHECK(t, d->super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, dll_int_count(d) == 2);
    CVXCHECK(t, dll_int_front(d) == 10);
    CVXCHECK(t, dll_int_back(d) == 20);

    CVX_MALLOC_RESET();
    dll_int_drop(d);
}

/* ---- push_back ---- */

static void test_dll_int_alloc_push_back(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_AFTER(1);
    struct dlinked_int *d = dll_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
    {
        CVX_MALLOC_RESET();
        return;
    }

    dll_int_push_back(d, 42);
    CVXCHECK(t, d->super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, dll_int_count(d) == 0);

    CVX_MALLOC_RESET();
    dll_int_drop(d);
}

/* ---- push_at (middle) ---- */

// push_at with a middle index (not 0, not count) allocates a node.
// Failing it must set CVX_FLAG_ALLOC and leave the list unchanged.
static void test_dll_int_alloc_push_at_middle(struct cvxtest *t)
{
    struct dlinked_int *d = dll_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    dll_int_push_back(d, 10);
    dll_int_push_back(d, 20);

    CVX_MALLOC_FAIL_NEXT();
    dll_int_push_at(d, 99, 1);
    CVXCHECK(t, d->super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, dll_int_count(d) == 2);
    CVXCHECK(t, dll_int_front(d) == 10);
    CVXCHECK(t, dll_int_back(d) == 20);

    CVX_MALLOC_RESET();
    dll_int_drop(d);
}

/* ---- iter_start / iter_end ---- */

static void test_dll_int_alloc_iter_start_fails(struct cvxtest *t)
{
    struct dlinked_int *d = dll_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    CVX_MALLOC_FAIL_NEXT();
    struct dlinked_int_iter *it = dll_int_iter_start(d);
    CVXCHECK(t, it == NULL);

    CVX_MALLOC_RESET();
    dll_int_drop(d);
}

static void test_dll_int_alloc_iter_end_fails(struct cvxtest *t)
{
    struct dlinked_int *d = dll_int_new();
    CVXCHECK(t, d != NULL);
    if (!d)
        return;

    CVX_MALLOC_FAIL_NEXT();
    struct dlinked_int_iter *it = dll_int_iter_end(d);
    CVXCHECK(t, it == NULL);

    CVX_MALLOC_RESET();
    dll_int_drop(d);
}

/* ---- runner ---- */

static struct cvxresult run_dlinked_list_alloc_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dlinked_list (alloc errors)\n");

    CVXRUN(&t, test_dll_int_alloc_copy_node_fails_with_drop);

    CVXRUN(&t, test_dll_int_alloc_new);

    CVXRUN(&t, test_dll_int_alloc_clone_new_fails);
    CVXRUN(&t, test_dll_int_alloc_clone_node_fails);
    CVXRUN(&t, test_dll_int_alloc_clone_second_node_fails);
    CVXRUN(&t, test_dll_int_alloc_clone_success);

    CVXRUN(&t, test_dll_int_alloc_push_front);
    CVXRUN(&t, test_dll_int_alloc_push_front_partial);

    CVXRUN(&t, test_dll_int_alloc_push_back);

    CVXRUN(&t, test_dll_int_alloc_push_at_middle);

    CVXRUN(&t, test_dll_int_alloc_iter_start_fails);
    CVXRUN(&t, test_dll_int_alloc_iter_end_fails);

    return CVXSUMMARY(&t);
}

#endif /* DLINKED_LIST_ALLOC_TESTS_H */
