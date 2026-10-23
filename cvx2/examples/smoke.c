// Smoke test for the generated examples; see config.json.
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cvx2/flags.h"
#include "generated/binop_list.h"
#include "generated/int10_array.h"
#include "generated/int_array.h"
#include "generated/int_map_oa.h"
#include "generated/int_map_sc.h"
#include "generated/int_ring.h"
#include "generated/str_ring.h"
#include "types.h"

static int int_comp(int const *a, int const *b)
{
    return *a - *b;
}

static size_t int_hash(int const *v)
{
    return (size_t)*v;
}

static void test_dynamic_array(void)
{
    struct int_array_vtabv vtabv = { .comp = int_comp };
    struct int_array arr;

    assert(ia_init(&arr, &vtabv, 0) == CVX_FLAG_OK);
    assert(ia_empty(&arr));

    assert(ia_push(&arr, ia_count(&arr), 10) == CVX_FLAG_OK);
    assert(ia_push(&arr, ia_count(&arr), 20) == CVX_FLAG_OK);
    assert(ia_push(&arr, 0, 5) == CVX_FLAG_OK);
    assert(ia_count(&arr) == 3);

    int out = 0;
    assert(ia_get(&arr, 0, &out) == CVX_FLAG_OK && out == 5);
    assert(ia_get(&arr, 2, &out) == CVX_FLAG_OK && out == 20);
    assert(ia_get(&arr, 99, &out) == CVX_FLAG_RANGE);

    assert(ia_pop(&arr, 0, &out) == CVX_FLAG_OK && out == 5);
    assert(ia_count(&arr) == 2);

    size_t idx = 0;
    assert(ia_index_of(&arr, 20, &idx) == CVX_FLAG_OK && idx == 1);
    assert(ia_index_of(&arr, 999, NULL) == CVX_FLAG_NOT_FOUND);
    assert(ia_index_of(&arr, 10, NULL) == CVX_FLAG_OK); // containment check idiom

    int seq[] = { 100, 200, 300 };
    assert(ia_seq_push(&arr, 1, seq, 3) == CVX_FLAG_OK);
    assert(ia_count(&arr) == 5);
    assert(ia_get(&arr, 1, &out) == CVX_FLAG_OK && out == 100);
    assert(ia_get(&arr, 3, &out) == CVX_FLAG_OK && out == 300);

    assert(ia_truncate(&arr, 2) == CVX_FLAG_OK);
    assert(ia_count(&arr) == 2);
    assert(ia_truncate(&arr, 0) == CVX_FLAG_OK); // same as "clear"
    assert(ia_empty(&arr));

    assert(ia_push(&arr, 0, 1) == CVX_FLAG_OK);
    assert(ia_push(&arr, 1, 2) == CVX_FLAG_OK);
    assert(ia_push(&arr, 2, 3) == CVX_FLAG_OK);

    assert(ia_resize(&arr, 100) == CVX_FLAG_OK);
    assert(ia_capacity(&arr) >= 100);
    assert(ia_count(&arr) == 3);

    assert(ia_shrink(&arr) == CVX_FLAG_OK);
    assert(ia_capacity(&arr) == ia_count(&arr));

    struct int_array sub;
    assert(ia_sublist(&arr, 1, 2, &sub) == CVX_FLAG_OK);
    assert(ia_count(&sub) == 2);
    assert(ia_get(&sub, 0, &out) == CVX_FLAG_OK && out == 2);
    assert(ia_get(&sub, 1, &out) == CVX_FLAG_OK && out == 3);

    assert(ia_append(&arr, &sub) == CVX_FLAG_OK);
    assert(ia_count(&arr) == 5);
    assert(ia_get(&arr, 3, &out) == CVX_FLAG_OK && out == 2);
    assert(ia_get(&arr, 4, &out) == CVX_FLAG_OK && out == 3);

    assert(ia_prepend(&arr, &sub) == CVX_FLAG_OK);
    assert(ia_count(&arr) == 7);
    assert(ia_get(&arr, 0, &out) == CVX_FLAG_OK && out == 2);
    assert(ia_get(&arr, 1, &out) == CVX_FLAG_OK && out == 3);

    ia_drop(&sub);
    ia_drop(&arr);
    assert(ia_pop(&arr, 0, &out) == CVX_FLAG_EMPTY);

    printf("dynamic_array: OK\n");
}

static void test_hashtable_open_addressing(void)
{
    // case="camelCase" cases a library suffix's own first word too, except
    // when the suffix is only one word (nothing to capitalize) -- so e.g.
    // moainit/moacount stay lowercase, while moagetRef (from "_get_ref")
    // shows real camelCasing on its second word.
    struct intMapOavtabK vtabk = { .hash = int_hash, .comp = int_comp };
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
    assert(i10a_push(&arr, i10a_count(&arr), a) == CVX_FLAG_OK);
    assert(i10a_push(&arr, i10a_count(&arr), b) == CVX_FLAG_OK);
    assert(i10a_count(&arr) == 2);

    int10_t out;
    assert(i10a_get(&arr, 0, &out) == CVX_FLAG_OK);
    for (int i = 0; i < 10; i++)
        assert(out.items[i] == a.items[i]);

    assert(i10a_pop(&arr, i10a_count(&arr) - 1, &out) == CVX_FLAG_OK);
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

    assert(bl_push(&fns, bl_count(&fns), add) == CVX_FLAG_OK);
    assert(bl_push(&fns, bl_count(&fns), mul) == CVX_FLAG_OK);
    assert(bl_count(&fns) == 2);

    binop_fn f;
    assert(bl_get(&fns, 0, &f) == CVX_FLAG_OK && f(3, 4) == 7);
    assert(bl_get(&fns, 1, &f) == CVX_FLAG_OK && f(3, 4) == 12);

    bl_drop(&fns);
    printf("dynamic_array<binop_fn> (function pointer): OK\n");
}

static int ir_at(struct int_ring *ring, size_t index)
{
    int out = -1;
    assert(ir_get(ring, index, &out) == CVX_FLAG_OK);
    return out;
}

static void test_circular_buffer(void)
{
    struct int_ring ring;
    int out = 0;

    assert(ir_init(&ring, NULL, 4) == CVX_FLAG_OK);
    assert(ir_empty(&ring) && ir_capacity(&ring) == 4);
    assert(ir_front(&ring, &out) == CVX_FLAG_EMPTY);
    assert(ir_back(&ring, &out) == CVX_FLAG_EMPTY);
    assert(ir_pop_back(&ring, &out) == CVX_FLAG_EMPTY);
    assert(ir_pop_front(&ring, &out) == CVX_FLAG_EMPTY);

    assert(ir_push_back(&ring, 1) == CVX_FLAG_OK);
    assert(ir_push_back(&ring, 2) == CVX_FLAG_OK);
    assert(ir_push_back(&ring, 3) == CVX_FLAG_OK);
    assert(ir_front(&ring, &out) == CVX_FLAG_OK && out == 1);
    assert(ir_back(&ring, &out) == CVX_FLAG_OK && out == 3);
    assert(ir_at(&ring, 1) == 2);
    assert(ir_get(&ring, 3, &out) == CVX_FLAG_RANGE);

    // [0 1 2 3], then overwriting the front: [1 2 3 4]
    assert(ir_push_front(&ring, 0) == CVX_FLAG_OK);
    assert(ir_full(&ring));
    assert(ir_push_back(&ring, 4) == CVX_FLAG_OK);
    assert(ir_count(&ring) == 4);
    assert(ir_at(&ring, 0) == 1 && ir_at(&ring, 3) == 4);

    // Wrap around the end of the buffer: [2 3 4 5]
    assert(ir_pop_front(&ring, &out) == CVX_FLAG_OK && out == 1);
    assert(ir_push_back(&ring, 5) == CVX_FLAG_OK);
    assert(!ir_is_linearized(&ring));
    assert(ir_linearize(&ring) == CVX_FLAG_OK);
    assert(ir_is_linearized(&ring) && ring.head == 0);
    for (int i = 0; i < 4; i++)
        assert(ring.buffer[i] == i + 2);

    // Overwriting the back: [9 2 3 4]
    assert(ir_push_front(&ring, 9) == CVX_FLAG_OK);
    assert(ir_at(&ring, 0) == 9 && ir_at(&ring, 3) == 4);
    assert(ir_pop_back(&ring, &out) == CVX_FLAG_OK && out == 4);
    assert(ir_pop_front(&ring, NULL) == CVX_FLAG_OK);
    assert(ir_count(&ring) == 2 && ir_at(&ring, 0) == 2 && ir_at(&ring, 1) == 3);

    // Grow, fill, then shrink, keeping the newest elements: [11 12 13]
    assert(ir_set_capacity(&ring, 6) == CVX_FLAG_OK);
    assert(ir_capacity(&ring) == 6 && ir_count(&ring) == 2);
    for (int i = 10; i < 14; i++)
        assert(ir_push_back(&ring, i) == CVX_FLAG_OK);
    assert(ir_full(&ring));
    assert(ir_set_capacity(&ring, 3) == CVX_FLAG_OK);
    assert(ir_capacity(&ring) == 3 && ir_count(&ring) == 3);
    assert(ir_at(&ring, 0) == 11 && ir_at(&ring, 2) == 13);

    struct int_ring clone;
    assert(ir_clone(&ring, &clone) == CVX_FLAG_OK);
    assert(ir_capacity(&clone) == 3 && ir_count(&clone) == 3);
    assert(ir_push_back(&clone, 14) == CVX_FLAG_OK);
    assert(ir_at(&clone, 0) == 12 && ir_at(&ring, 0) == 11);
    ir_drop(&clone);

    assert(ir_set_capacity(&ring, 0) == CVX_FLAG_OK);
    assert(ir_capacity(&ring) == 0 && ir_empty(&ring) && ring.buffer == NULL);
    assert(ir_push_back(&ring, 1) == CVX_FLAG_FULL);
    assert(ir_push_front(&ring, 1) == CVX_FLAG_FULL);
    assert(ir_linearize(&ring) == CVX_FLAG_OK);
    ir_drop(&ring);

    printf("circular_buffer: OK\n");
}

static void test_circular_buffer_linearize_every_head(void)
{
    for (size_t capacity = 1; capacity <= 7; capacity++)
    {
        for (size_t count = 0; count <= capacity; count++)
        {
            for (size_t head = 0; head < capacity; head++)
            {
                struct int_ring ring;
                assert(ir_init(&ring, NULL, capacity) == CVX_FLAG_OK);

                for (size_t i = 0; i < head; i++)
                {
                    assert(ir_push_back(&ring, -1) == CVX_FLAG_OK);
                    assert(ir_pop_front(&ring, NULL) == CVX_FLAG_OK);
                }
                assert(ring.head == head);

                for (size_t i = 0; i < count; i++)
                    assert(ir_push_back(&ring, (int)i) == CVX_FLAG_OK);

                assert(ir_linearize(&ring) == CVX_FLAG_OK);
                assert(ring.head == 0 && ir_count(&ring) == count);
                for (size_t i = 0; i < count; i++)
                    assert(ring.buffer[i] == (int)i);

                ir_drop(&ring);
            }
        }
    }

    printf("circular_buffer linearize: OK\n");
}

static size_t str_drops = 0;

static enum cvx_flags str_copy(char *const *src, char **dst)
{
    *dst = malloc(strlen(*src) + 1);
    if (!*dst)
        return CVX_FLAG_ALLOC;
    strcpy(*dst, *src);
    return CVX_FLAG_OK;
}

static void str_drop(char **str)
{
    free(*str);
    str_drops++;
}

static char *str_new(const char *str)
{
    char *out = NULL;
    assert(str_copy((char *const *)&str, &out) == CVX_FLAG_OK);
    return out;
}

static void test_circular_buffer_of_owned_strings(void)
{
    struct StrRingVtabV vtabv = { .copy = str_copy, .drop = str_drop };
    struct StrRing ring;
    char *out = NULL;

    assert(srInit(&ring, &vtabv, 2) == CVX_FLAG_OK);
    assert(srPushBack(&ring, str_new("a")) == CVX_FLAG_OK);
    assert(srPushBack(&ring, str_new("b")) == CVX_FLAG_OK);
    assert(srPushBack(&ring, str_new("c")) == CVX_FLAG_OK); // drops "a"
    assert(str_drops == 1);
    assert(srFront(&ring, &out) == CVX_FLAG_OK && strcmp(out, "b") == 0);

    struct StrRing clone;
    assert(srClone(&ring, &clone) == CVX_FLAG_OK);
    char *cloned = NULL;
    assert(srGet(&clone, 1, &cloned) == CVX_FLAG_OK && strcmp(cloned, "c") == 0);
    assert(srGet(&ring, 1, &out) == CVX_FLAG_OK && out != cloned);

    assert(srSetCapacity(&ring, 1) == CVX_FLAG_OK); // drops "b"
    assert(str_drops == 2);
    assert(srFront(&ring, &out) == CVX_FLAG_OK && strcmp(out, "c") == 0);

    assert(srPopBack(&ring, &out) == CVX_FLAG_OK); // ownership moves to the caller
    assert(str_drops == 2);
    free(out);

    srDrop(&ring);
    srDrop(&clone);
    assert(str_drops == 4);

    printf("circular_buffer<char *> (copy/drop): OK\n");
}

int main(void)
{
    test_dynamic_array();
    test_hashtable_open_addressing();
    test_hashtable_separate_chaining();
    test_dynamic_array_of_fixed_array();
    test_dynamic_array_of_function_pointers();
    test_circular_buffer();
    test_circular_buffer_linearize_every_head();
    test_circular_buffer_of_owned_strings();
    printf("all cvx2 smoke tests passed\n");
    return 0;
}
