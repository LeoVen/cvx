// Exercises the generated output from cvx2/examples/config.json: one
// dynamic_array instantiation, one hashtable instantiation per collision
// variant (proving the variant mechanism actually selects between two
// different real implementations, not just a config flag), and two
// dynamic_array instantiations over "complex" declarator types (a
// fixed-size array wrapped in a typedef'd struct, and a typedef'd function
// pointer) that only work because of their "includes": ["types.h"] config
// -- see types.h for why that's required.
#include <assert.h>
#include <stdio.h>

#include "cvx2/flags.h"
#include "generated/int_array.h"
#include "generated/int_map_oa.h"
#include "generated/int_map_sc.h"
#include "generated/int10_array.h"
#include "generated/binop_list.h"
#include "types.h" // int10_t / binop_fn, used directly below

static int int_comp(int a, int b)
{
    return a - b;
}

static size_t int_hash(int v)
{
    return (size_t)v;
}

static void test_dynamic_array(void)
{
    struct int_array_vtabv vtabv = { .comp = int_comp };
    struct int_array arr;

    assert(ia_init(&arr, &vtabv, 0) == CVX2_FLAG_OK);
    assert(ia_empty(&arr));

    assert(ia_push_back(&arr, 10) == CVX2_FLAG_OK);
    assert(ia_push_back(&arr, 20) == CVX2_FLAG_OK);
    assert(ia_push_front(&arr, 5) == CVX2_FLAG_OK);
    assert(ia_count(&arr) == 3);

    int out = 0;
    assert(ia_get(&arr, 0, &out) == CVX2_FLAG_OK && out == 5);
    assert(ia_get(&arr, 2, &out) == CVX2_FLAG_OK && out == 20);
    assert(ia_get(&arr, 99, &out) == CVX2_FLAG_RANGE);

    assert(ia_pop_front(&arr, &out) == CVX2_FLAG_OK && out == 5);
    assert(ia_count(&arr) == 2);

    ia_drop(&arr);
    assert(ia_pop_back(&arr, &out) == CVX2_FLAG_EMPTY);

    printf("dynamic_array: OK\n");
}

static void test_hashtable_open_addressing(void)
{
    struct intMapOaVtabk vtabk = { .hash = int_hash, .comp = int_comp };
    struct intMapOa map;

    assert(moaInit(&map, &vtabk, NULL, 0) == CVX2_FLAG_OK);

    for (int i = 0; i < 200; i++)
        assert(moaInsert(&map, i, i * 10) == CVX2_FLAG_OK);
    assert(moaCount(&map) == 200);

    assert(moaInsert(&map, 5, 999) == CVX2_FLAG_DUPLICATE);

    int out = 0;
    assert(moaGet(&map, 42, &out) == CVX2_FLAG_OK && out == 420);
    assert(moaGet(&map, 99999, &out) == CVX2_FLAG_NOT_FOUND);
    assert(moaContains(&map, 7));
    assert(!moaContains(&map, 99999));

    int old = 0;
    assert(moaUpdate(&map, 7, 7000, &old) == CVX2_FLAG_OK && old == 70);
    assert(moaGet(&map, 7, &out) == CVX2_FLAG_OK && out == 7000);

    assert(moaRemove(&map, 7, &out) == CVX2_FLAG_OK && out == 7000);
    assert(!moaContains(&map, 7));
    assert(moaRemove(&map, 7, &out) == CVX2_FLAG_NOT_FOUND);
    assert(moaCount(&map) == 199);

    moaDrop(&map);
    printf("hashtable (open_addressing): OK\n");
}

static void test_hashtable_separate_chaining(void)
{
    struct int_map_sc_vtabk vtabk = { .hash = int_hash, .comp = int_comp };
    struct int_map_sc map;

    assert(msc_init(&map, &vtabk, NULL, 0) == CVX2_FLAG_OK);

    for (int i = 0; i < 200; i++)
        assert(msc_insert(&map, i, i * 10) == CVX2_FLAG_OK);
    assert(msc_count(&map) == 200);

    assert(msc_insert(&map, 5, 999) == CVX2_FLAG_DUPLICATE);

    int out = 0;
    assert(msc_get(&map, 42, &out) == CVX2_FLAG_OK && out == 420);
    assert(msc_get(&map, 99999, &out) == CVX2_FLAG_NOT_FOUND);
    assert(msc_contains(&map, 7));
    assert(!msc_contains(&map, 99999));

    int old = 0;
    assert(msc_update(&map, 7, 7000, &old) == CVX2_FLAG_OK && old == 70);
    assert(msc_get(&map, 7, &out) == CVX2_FLAG_OK && out == 7000);

    assert(msc_remove(&map, 7, &out) == CVX2_FLAG_OK && out == 7000);
    assert(!msc_contains(&map, 7));
    assert(msc_remove(&map, 7, &out) == CVX2_FLAG_NOT_FOUND);
    assert(msc_count(&map) == 199);

    msc_drop(&map);
    printf("hashtable (separate_chaining): OK\n");
}

static void test_dynamic_array_of_fixed_array(void)
{
    struct int10_array arr;
    assert(i10a_init(&arr, NULL, 0) == CVX2_FLAG_OK);

    int10_t a = { .items = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } };
    int10_t b = { .items = { 0 } };
    assert(i10a_push_back(&arr, a) == CVX2_FLAG_OK);
    assert(i10a_push_back(&arr, b) == CVX2_FLAG_OK);
    assert(i10a_count(&arr) == 2);

    int10_t out;
    assert(i10a_get(&arr, 0, &out) == CVX2_FLAG_OK);
    for (int i = 0; i < 10; i++)
        assert(out.items[i] == a.items[i]);

    assert(i10a_pop_back(&arr, &out) == CVX2_FLAG_OK);
    for (int i = 0; i < 10; i++)
        assert(out.items[i] == b.items[i]);

    i10a_drop(&arr);
    printf("dynamic_array<int10_t> (fixed-size array): OK\n");
}

static int add(int a, int b)
{
    return a + b;
}

static int mul(int a, int b)
{
    return a * b;
}

static void test_dynamic_array_of_function_pointers(void)
{
    struct binop_list fns;
    assert(bl_init(&fns, NULL, 0) == CVX2_FLAG_OK);

    assert(bl_push_back(&fns, add) == CVX2_FLAG_OK);
    assert(bl_push_back(&fns, mul) == CVX2_FLAG_OK);
    assert(bl_count(&fns) == 2);

    binop_fn f;
    assert(bl_get(&fns, 0, &f) == CVX2_FLAG_OK && f(3, 4) == 7);
    assert(bl_get(&fns, 1, &f) == CVX2_FLAG_OK && f(3, 4) == 12);

    bl_drop(&fns);
    printf("dynamic_array<binop_fn> (function pointer): OK\n");
}

int main(void)
{
    test_dynamic_array();
    test_hashtable_open_addressing();
    test_hashtable_separate_chaining();
    test_dynamic_array_of_fixed_array();
    test_dynamic_array_of_function_pointers();
    printf("all cvx2 smoke tests passed\n");
    return 0;
}
