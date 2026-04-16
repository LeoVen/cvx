#ifndef BINARY_HEAP_ALLOC_TESTS_H
#define BINARY_HEAP_ALLOC_TESTS_H

// alloc.h must be included before implementations.h so that the #define
// malloc / realloc macros are in effect when binary_heap.h is compiled.
#include "tests/alloc.h"

#include "tests/cvxtest.h"
#include "tests/implementations.h"

/* ---- init_with (stack-allocated) ---- */

// bh_int_init_with() allocates only the buffer (struct is on the stack).
// Failing that malloc must return a struct with CVX_FLAG_ALLOC and NULL buffer.
static void test_bh_int_alloc_init_with_buffer_fails(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    struct bheap_int h = bh_int_init_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 8);
    CVXCHECK(t, h.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, h.buffer == NULL);
    CVX_MALLOC_RESET();
}

/* ---- copy (stack-allocated) ---- */

// bh_int_copy() allocates only the buffer (struct is on the stack).
// Failing that malloc must return a struct with CVX_FLAG_ALLOC and NULL buffer.
static void test_bh_int_alloc_copy_buffer_fails(struct cvxtest *t)
{
    struct bheap_int *src = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, src != NULL);
    if (!src)
        return;

    bh_int_push(src, 10);
    bh_int_push(src, 20);

    CVX_MALLOC_FAIL_NEXT();
    struct bheap_int copy = bh_int_copy(src);
    CVXCHECK(t, copy.super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, copy.buffer == NULL);

    CVX_MALLOC_RESET();
    bh_int_drop(src);
}

/* ---- new ---- */

// bh_int_new() performs one allocation (the struct). Failing it returns NULL.
static void test_bh_int_alloc_new_struct_fails(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    struct bheap_int *h = bh_int_new();
    CVXCHECK(t, h == NULL);
    CVX_MALLOC_RESET();
}

/* ---- new_with ---- */

// bh_int_new_with() performs two allocations: the struct, then the buffer.
// Failing the struct returns NULL.
static void test_bh_int_alloc_new_with_struct_fails(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_NEXT();
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 4);
    CVXCHECK(t, h == NULL);
    CVX_MALLOC_RESET();
}

// Failing the buffer must free the struct and return NULL (no leak).
static void test_bh_int_alloc_new_with_buffer_fails(struct cvxtest *t)
{
    CVX_MALLOC_FAIL_AFTER(1);
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 4);
    CVXCHECK(t, h == NULL);
    CVX_MALLOC_RESET();
}

/* ---- clone ---- */

// clone() allocates: (1) new struct via _new(), (2) buffer if count > 0.
// Failing the struct returns NULL.
static void test_bh_int_alloc_clone_struct_fails(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, h != NULL);
    if (!h)
        return;
    bh_int_push(h, 10);
    bh_int_push(h, 20);

    CVX_MALLOC_FAIL_NEXT();
    struct bheap_int *clone = bh_int_clone(h);
    CVXCHECK(t, clone == NULL);

    CVX_MALLOC_RESET();
    bh_int_drop(h);
}

// Failing the buffer allocation inside clone must free the cloned struct and
// return NULL.
static void test_bh_int_alloc_clone_buffer_fails(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, h != NULL);
    if (!h)
        return;
    bh_int_push(h, 10);
    bh_int_push(h, 20);

    // Allow 1 alloc (the clone struct), fail the buffer malloc.
    CVX_MALLOC_FAIL_AFTER(1);
    struct bheap_int *clone = bh_int_clone(h);
    CVXCHECK(t, clone == NULL);

    CVX_MALLOC_RESET();
    bh_int_drop(h);
}

/* ---- push: initial buffer allocation ---- */

// The first push on a heap with no buffer triggers the initial malloc.
// Failing it must set CVX_FLAG_ALLOC; count stays 0.
static void test_bh_int_alloc_push_initial_buffer_fails(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, h != NULL);
    if (!h)
        return;

    CVX_MALLOC_FAIL_NEXT();
    bh_int_push(h, 42);
    CVXCHECK(t, h->super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, bh_int_count(h) == 0);

    CVX_MALLOC_RESET();
    bh_int_drop(h);
}

/* ---- push: realloc (buffer growth) ---- */

// Use new_with(capacity=2) so the realloc path is reached after exactly 2
// pushes without any additional mallocs to count.
// On realloc failure the existing elements and capacity must be preserved.
static void test_bh_int_alloc_push_realloc_fails(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 2);
    CVXCHECK(t, h != NULL);
    if (!h)
        return;

    bh_int_push(h, 10);
    bh_int_push(h, 20);

    CVX_MALLOC_FAIL_NEXT();
    bh_int_push(h, 30);
    CVXCHECK(t, h->super.flag == CVX_FLAG_ALLOC);
    CVXCHECK(t, bh_int_count(h) == 2);
    CVXCHECK(t, bh_int_capacity(h) == 2);

    CVX_MALLOC_RESET();
    bh_int_drop(h);
}

/* ---- iter_start / iter_end ---- */

static void test_bh_int_alloc_iter_start_fails(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, h != NULL);
    if (!h)
        return;

    CVX_MALLOC_FAIL_NEXT();
    struct bheap_int_iter *it = bh_int_iter_start(h);
    CVXCHECK(t, it == NULL);

    CVX_MALLOC_RESET();
    bh_int_drop(h);
}

static void test_bh_int_alloc_iter_end_fails(struct cvxtest *t)
{
    struct bheap_int *h = bh_int_new_with(bh_int_vtabv_comp_only, CVX_MAX_HEAP, 0);
    CVXCHECK(t, h != NULL);
    if (!h)
        return;

    CVX_MALLOC_FAIL_NEXT();
    struct bheap_int_iter *it = bh_int_iter_end(h);
    CVXCHECK(t, it == NULL);

    CVX_MALLOC_RESET();
    bh_int_drop(h);
}

/* ---- runner ---- */

static struct cvxresult run_binary_heap_alloc_tests(void)
{
    struct cvxtest t = { 0 };

    printf("binary_heap (alloc errors)\n");

    CVXRUN(&t, test_bh_int_alloc_init_with_buffer_fails);
    CVXRUN(&t, test_bh_int_alloc_copy_buffer_fails);

    CVXRUN(&t, test_bh_int_alloc_new_struct_fails);

    CVXRUN(&t, test_bh_int_alloc_new_with_struct_fails);
    CVXRUN(&t, test_bh_int_alloc_new_with_buffer_fails);

    CVXRUN(&t, test_bh_int_alloc_clone_struct_fails);
    CVXRUN(&t, test_bh_int_alloc_clone_buffer_fails);

    CVXRUN(&t, test_bh_int_alloc_push_initial_buffer_fails);
    CVXRUN(&t, test_bh_int_alloc_push_realloc_fails);

    CVXRUN(&t, test_bh_int_alloc_iter_start_fails);
    CVXRUN(&t, test_bh_int_alloc_iter_end_fails);

    return CVXSUMMARY(&t);
}

#endif /* BINARY_HEAP_ALLOC_TESTS_H */
