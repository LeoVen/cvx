#ifndef DYNAMIC_ARRAY_TESTS_H
#define DYNAMIC_ARRAY_TESTS_H

#include "cvx/flags.h"
#include "cvxtest.h"
#include "implementations.h"
#include <stdlib.h>

static void test_da_init(struct cvxtest *t)
{
    // Default init
    struct dynamic_array arr;
    da_init(&arr, NULL, 0);
    CVXCHECK(t, arr.super.tag == 99);
    CVXCHECK(t, arr.buffer == NULL);
    CVXCHECK(t, arr.capacity == 0);
    CVXCHECK(t, arr.count == 0);
    // Capacity init
    da_init(&arr, da_vtabv_full, 16);
    CVXCHECK(t, arr.buffer != NULL);
    CVXCHECK(t, arr.vtabv == da_vtabv_full);
    CVXCHECK(t, da_capacity(&arr) == 16);
    CVXCHECK(t, da_count(&arr) == 0);
    da_drop(&arr);
}

static void test_da_drop(struct cvxtest *t)
{
    // Drop empty
    struct dynamic_array arr;
    da_init(&arr, NULL, 0);
    da_drop(&arr);
    // Random combination of _init, _drop, _clone on empty _init should always work
    struct dynamic_array clone;
    da_init(&arr, NULL, 0);
    for (int i = 0; i < 100; i++)
    {
        int choice = rand() % 3;
        if (choice == 0)
            da_init(&arr, NULL, 0);
        if (choice == 1)
            da_drop(&arr);
        if (choice == 2)
            da_clone(&arr, &clone);
    }
}

static void test_da_clone(struct cvxtest *t)
{
    // Clone and drop empty
    struct dynamic_array arr, clone;
    da_init(&arr, NULL, 0);
    da_clone(&arr, &clone);
    CVXCHECK(t, clone.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, clone.super.tag == 99);
    CVXCHECK(t, clone.count == 0);
    da_drop(&arr);
    da_drop(&clone);
    // Clone with memcpy
    da_init(&arr, NULL, 1);
    for (int i = 0; i < 20; i++)
        da_push_back(&arr, i);
    da_clone(&arr, &clone);
    CVXCHECK(t, clone.count == 20);
    CVXCHECK(t, clone.buffer != NULL);
    for (int i = 0; i < 20; i++)
        CVXCHECK(t, clone.buffer[i] == i);
    da_drop(&arr);
    da_drop(&clone);
    // Clone with vtabv->clone
    CVX_TEST_COUNTER_CLONE_RESET();
    da_init(&arr, da_vtabv_full, 1);
    for (int i = 0; i < 20; i++)
        da_push_back(&arr, i);
    CVX_TEST_COUNTER_CLONE(t, 0);
    da_clone(&arr, &clone);
    CVX_TEST_COUNTER_CLONE(t, 20);
    CVXCHECK(t, arr.vtabv == clone.vtabv);
    for (int i = 0; i < 20; i++)
        CVXCHECK(t, clone.buffer[i] == i);
    da_drop(&arr);
    da_drop(&clone);
}

static void test_da_flag(struct cvxtest *t)
{
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 16);
    CVXCHECK(t, da_flag(&arr) == CVX_FLAG_OK);
    da_drop(&arr);
}

static void test_da_count(struct cvxtest *t)
{
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 16);
    CVXCHECK(t, da_count(&arr) == 0);
    da_push_back(&arr, 10);
    CVXCHECK(t, da_count(&arr) == 1);
    da_drop(&arr);
}

static void test_da_capacity(struct cvxtest *t)
{
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 16);
    CVXCHECK(t, da_capacity(&arr) == 16);
    for (int i = 0; i < 20; i++)
        da_push_at(&arr, i, 0);
    CVXCHECK(t, da_capacity(&arr) == (size_t)(16 * CVX_BUFFER_GROWTH_RATE));
    da_drop(&arr);
}

static void test_da_empty(struct cvxtest *t)
{
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 0);
    CVXCHECK(t, da_empty(&arr) == true);
    da_push_back(&arr, 1);
    da_drop(&arr);
}

static void test_da_full(struct cvxtest *t)
{
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 2);
    CVXCHECK(t, da_full(&arr) == false);
    da_push_back(&arr, 1);
    da_push_back(&arr, 2);
    CVXCHECK(t, da_full(&arr) == true);
    da_drop(&arr);
}

static void test_da_front(struct cvxtest *t)
{
    // Regular case
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 11);
    da_push_back(&arr, 22);
    CVXCHECK(t, da_front(&arr) == 11);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_OK);
    da_drop(&arr);
    // Empty
    da_init(&arr, da_vtabv_full, 0);
    da_front(&arr);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_EMPTY);
    da_drop(&arr);
}

static void test_da_back(struct cvxtest *t)
{
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 11);
    da_push_back(&arr, 22);
    CVXCHECK(t, da_back(&arr) == 22);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_OK);
    da_drop(&arr);
    // Empty
    da_init(&arr, da_vtabv_full, 0);
    da_back(&arr);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_EMPTY);
    da_drop(&arr);
}

static void test_da_get(struct cvxtest *t)
{
    // Regular case
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 10);
    da_push_back(&arr, 20);
    da_push_back(&arr, 30);
    CVXCHECK(t, da_get(&arr, 0) == 10);
    CVXCHECK(t, da_get(&arr, 1) == 20);
    CVXCHECK(t, da_get(&arr, 2) == 30);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_OK);
    da_drop(&arr);
    // Out of range
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 1);
    da_get(&arr, 1);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_RANGE);
    da_drop(&arr);
    // Empty
    da_init(&arr, da_vtabv_full, 0);
    da_get(&arr, 0);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_RANGE);
    da_drop(&arr);
}

static void test_da_push_front(struct cvxtest *t)
{
    // Regular case
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 1);
    da_push_back(&arr, 1);
    da_push_back(&arr, 2);
    da_push_front(&arr, 0);
    CVXCHECK(t, arr.buffer[0] == 0);
    CVXCHECK(t, arr.buffer[1] == 1);
    CVXCHECK(t, arr.buffer[2] == 2);
    CVXCHECK(t, da_count(&arr) == 3);
    da_drop(&arr);
}

static void test_da_push_at(struct cvxtest *t)
{
    // Regular case
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 1);
    da_push_back(&arr, 3);
    da_push_at(&arr, 2, 1);
    CVXCHECK(t, arr.buffer[0] == 1);
    CVXCHECK(t, arr.buffer[1] == 2);
    CVXCHECK(t, arr.buffer[2] == 3);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_OK);
    CVXCHECK(t, da_count(&arr) == 3);
    da_drop(&arr);
    // Out of range
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 1);
    da_push_at(&arr, 99, 5);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_RANGE);
    da_drop(&arr);
}

static void test_da_push_back(struct cvxtest *t)
{
    // Regular case
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 1);
    da_push_back(&arr, 10);
    da_push_back(&arr, 20);
    da_push_back(&arr, 30);
    CVXCHECK(t, arr.buffer[0] == 10);
    CVXCHECK(t, arr.buffer[1] == 20);
    CVXCHECK(t, arr.buffer[2] == 30);
    da_drop(&arr);
    // Buffer growth
    da_init(&arr, da_vtabv_full, 1);
    for (int i = 0; i < 20; i++)
        da_push_back(&arr, i);
    CVXCHECK(t, da_count(&arr) == 20);
    CVXCHECK(t, da_capacity(&arr) >= 20);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_OK);
    da_drop(&arr);
}

static void test_da_pop_front(struct cvxtest *t)
{
    // Regular case
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 10);
    da_push_back(&arr, 20);
    int out = da_pop_front(&arr);
    CVXCHECK(t, out == 10);
    CVXCHECK(t, da_count(&arr) == 1);
    CVXCHECK(t, arr.buffer[0] == 20);
    da_drop(&arr);
    // Empty case
    da_init(&arr, da_vtabv_full, 0);
    da_pop_front(&arr);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_EMPTY);
    da_drop(&arr);
}

static void test_da_pop_at(struct cvxtest *t)
{
    // Pop at middle
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 1);
    da_push_back(&arr, 2);
    da_push_back(&arr, 3);
    int out = da_pop_at(&arr, 1);
    CVXCHECK(t, out == 2);
    CVXCHECK(t, da_count(&arr) == 2);
    CVXCHECK(t, arr.buffer[0] == 1);
    CVXCHECK(t, arr.buffer[1] == 3);
    da_drop(&arr);
    // Out of range
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 1);
    da_pop_at(&arr, 5);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_RANGE);
    da_drop(&arr);
    // Empty
    da_init(&arr, da_vtabv_full, 0);
    da_pop_at(&arr, 0);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_EMPTY);
    da_drop(&arr);
}

static void test_da_pop_back(struct cvxtest *t)
{
    // Regular case
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 42);
    da_push_back(&arr, 7);
    int out = da_pop_back(&arr);
    CVXCHECK(t, out == 7);
    CVXCHECK(t, da_count(&arr) == 1);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_OK);
    da_drop(&arr);
    // Case empty
    da_init(&arr, da_vtabv_full, 0);
    da_pop_back(&arr);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_EMPTY);
    da_drop(&arr);
}

static void test_da_replace_front(struct cvxtest *t)
{
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 10);
    da_push_back(&arr, 20);
    int old = da_replace_front(&arr, 99);
    CVXCHECK(t, old == 10);
    CVXCHECK(t, da_front(&arr) == 99);
    CVXCHECK(t, da_count(&arr) == 2);
    da_drop(&arr);
    // When empty
    da_init(&arr, NULL, 0);
    da_replace_front(&arr, 42);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, da_count(&arr) == 0);
    da_drop(&arr);
}
static void test_da_replace_back(struct cvxtest *t)
{
    // Regular case
    struct dynamic_array arr;
    da_init(&arr, da_vtabv_full, 0);
    da_push_back(&arr, 5);
    da_push_back(&arr, 10);
    int old = da_replace_back(&arr, 99);
    CVXCHECK(t, old == 10);
    CVXCHECK(t, da_back(&arr) == 99);
    CVXCHECK(t, da_count(&arr) == 2);
    da_drop(&arr);
    // When empty
    da_init(&arr, da_vtabv_full, 0);
    da_replace_back(&arr, 42);
    CVXCHECK(t, arr.super.flag == CVX_FLAG_EMPTY);
    CVXCHECK(t, da_count(&arr) == 0);
    da_drop(&arr);
}

static void test_da_swap(struct cvxtest *t)
{
}
static void test_da_compare(struct cvxtest *t)
{
}
static void test_da_sort(struct cvxtest *t)
{
}

static struct cvxresult run_dynamic_array_tests(void)
{
    struct cvxtest t = { 0 };

    printf("dynamic_array\n");

    CVXRUN(&t, test_da_init);
    CVXRUN(&t, test_da_drop);
    CVXRUN(&t, test_da_clone);
    CVXRUN(&t, test_da_flag);
    CVXRUN(&t, test_da_count);
    CVXRUN(&t, test_da_capacity);
    CVXRUN(&t, test_da_empty);
    CVXRUN(&t, test_da_full);
    CVXRUN(&t, test_da_front);
    CVXRUN(&t, test_da_back);
    CVXRUN(&t, test_da_get);
    CVXRUN(&t, test_da_push_front);
    CVXRUN(&t, test_da_push_at);
    CVXRUN(&t, test_da_push_back);
    CVXRUN(&t, test_da_pop_front);
    CVXRUN(&t, test_da_pop_at);
    CVXRUN(&t, test_da_pop_back);
    CVXRUN(&t, test_da_replace_front);
    CVXRUN(&t, test_da_replace_back);
    CVXRUN(&t, test_da_swap);
    CVXRUN(&t, test_da_compare);
    CVXRUN(&t, test_da_sort);

    return CVXSUMMARY(&t);
}

#endif /* DYNAMIC_ARRAY_TESTS_H */
