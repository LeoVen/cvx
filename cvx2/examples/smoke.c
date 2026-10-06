// Smoke test for the generated examples; see config.json.
#include <assert.h>
#include <stdio.h>

#include "cvx2/flags.h"
#include "generated/int_array.h"
#include "generated/int_map_oa.h"
#include "generated/int_map_sc.h"
#include "generated/int10_array.h"
#include "generated/binop_list.h"
#include "types.h"

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

    assert(ia_init(&arr, &vtabv, 0) == CVX_FLAG_OK);
    assert(ia_empty(&arr));

    assert(ia_push_back(&arr, 10) == CVX_FLAG_OK);
    assert(ia_push_back(&arr, 20) == CVX_FLAG_OK);
    assert(ia_push_front(&arr, 5) == CVX_FLAG_OK);
    assert(ia_count(&arr) == 3);

    int out = 0;
    assert(ia_get(&arr, 0, &out) == CVX_FLAG_OK && out == 5);
    assert(ia_get(&arr, 2, &out) == CVX_FLAG_OK && out == 20);
    assert(ia_get(&arr, 99, &out) == CVX_FLAG_RANGE);

    assert(ia_pop_front(&arr, &out) == CVX_FLAG_OK && out == 5);
    assert(ia_count(&arr) == 2);

    ia_drop(&arr);
    assert(ia_pop_back(&arr, &out) == CVX_FLAG_EMPTY);

    printf("dynamic_array: OK\n");
}

static void test_hashtable_open_addressing(void)
{
    // case="camelCase" cases a library suffix's own first word too, except
    // when the suffix is only one word (nothing to capitalize) -- so e.g.
    // moainit/moacount stay lowercase, while moagetRef (from "_get_ref")
    // shows real camelCasing on its second word.
    struct intMapOavtabk vtabk = { .hash = int_hash, .comp = int_comp };
    struct intMapOa map;

    assert(moainit(&map, &vtabk, NULL, 0) == CVX_FLAG_OK);

    for (int i = 0; i < 200; i++)
        assert(moainsert(&map, i, i * 10) == CVX_FLAG_OK);
    assert(moacount(&map) == 200);

    assert(moainsert(&map, 5, 999) == CVX_FLAG_DUPLICATE);

    int out = 0;
    assert(moaget(&map, 42, &out) == CVX_FLAG_OK && out == 420);
    assert(moaget(&map, 99999, &out) == CVX_FLAG_NOT_FOUND);
    assert(moacontains(&map, 7));
    assert(!moacontains(&map, 99999));

    int old = 0;
    assert(moaupdate(&map, 7, 7000, &old) == CVX_FLAG_OK && old == 70);
    assert(moaget(&map, 7, &out) == CVX_FLAG_OK && out == 7000);

    assert(moaremove(&map, 7, &out) == CVX_FLAG_OK && out == 7000);
    assert(!moacontains(&map, 7));
    assert(moaremove(&map, 7, &out) == CVX_FLAG_NOT_FOUND);
    assert(moacount(&map) == 199);

    moadrop(&map);
    printf("hashtable (open_addressing): OK\n");
}

static void test_hashtable_separate_chaining(void)
{
    struct int_map_sc_vtabk vtabk = { .hash = int_hash, .comp = int_comp };
    struct int_map_sc map;

    assert(msc_init(&map, &vtabk, NULL, 0) == CVX_FLAG_OK);

    for (int i = 0; i < 200; i++)
        assert(msc_insert(&map, i, i * 10) == CVX_FLAG_OK);
    assert(msc_count(&map) == 200);

    assert(msc_insert(&map, 5, 999) == CVX_FLAG_DUPLICATE);

    int out = 0;
    assert(msc_get(&map, 42, &out) == CVX_FLAG_OK && out == 420);
    assert(msc_get(&map, 99999, &out) == CVX_FLAG_NOT_FOUND);
    assert(msc_contains(&map, 7));
    assert(!msc_contains(&map, 99999));

    int old = 0;
    assert(msc_update(&map, 7, 7000, &old) == CVX_FLAG_OK && old == 70);
    assert(msc_get(&map, 7, &out) == CVX_FLAG_OK && out == 7000);

    assert(msc_remove(&map, 7, &out) == CVX_FLAG_OK && out == 7000);
    assert(!msc_contains(&map, 7));
    assert(msc_remove(&map, 7, &out) == CVX_FLAG_NOT_FOUND);
    assert(msc_count(&map) == 199);

    msc_drop(&map);
    printf("hashtable (separate_chaining): OK\n");
}

static void test_dynamic_array_of_fixed_array(void)
{
    struct int10_array arr;
    assert(i10a_init(&arr, NULL, 0) == CVX_FLAG_OK);

    int10_t a = { .items = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } };
    int10_t b = { .items = { 0 } };
    assert(i10a_push_back(&arr, a) == CVX_FLAG_OK);
    assert(i10a_push_back(&arr, b) == CVX_FLAG_OK);
    assert(i10a_count(&arr) == 2);

    int10_t out;
    assert(i10a_get(&arr, 0, &out) == CVX_FLAG_OK);
    for (int i = 0; i < 10; i++)
        assert(out.items[i] == a.items[i]);

    assert(i10a_pop_back(&arr, &out) == CVX_FLAG_OK);
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
    assert(bl_init(&fns, NULL, 0) == CVX_FLAG_OK);

    assert(bl_push_back(&fns, add) == CVX_FLAG_OK);
    assert(bl_push_back(&fns, mul) == CVX_FLAG_OK);
    assert(bl_count(&fns) == 2);

    binop_fn f;
    assert(bl_get(&fns, 0, &f) == CVX_FLAG_OK && f(3, 4) == 7);
    assert(bl_get(&fns, 1, &f) == CVX_FLAG_OK && f(3, 4) == 12);

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
