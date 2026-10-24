// Smoke test for the generated examples; see config.json.
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cvx2/flags.h"
#include "generated/binop_list.h"
#include "generated/int10_array.h"
#include "generated/int_array.h"
#include "generated/int_dlist.h"
#include "generated/int_heap.h"
#include "generated/int_map_oa.h"
#include "generated/int_map_sc.h"
#include "generated/int_ring.h"
#include "generated/int_slist.h"
#include "generated/str_heap.h"
#include "generated/str_list.h"
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

#define PASTE_(A, B) A##B
#define PASTE(A, B) PASTE_(A, B)

// Walks the list checking count, tail and (for double lists) every prev link,
// then compares the elements against expected.
static void isl_check(struct int_slist *list, const int *expected, size_t count)
{
    assert(list->count == count);
    assert((list->head == NULL) == (count == 0) && (list->tail == NULL) == (count == 0));
    size_t i = 0;
    for (struct int_slist_node *node = list->head; node; node = node->next, i++)
    {
        assert(i < count && node->value == expected[i]);
        if (!node->next)
            assert(node == list->tail);
    }
    assert(i == count);
}

static void idl_check(struct int_dlist *list, const int *expected, size_t count)
{
    assert(list->count == count);
    assert((list->head == NULL) == (count == 0) && (list->tail == NULL) == (count == 0));
    size_t i = 0;
    struct int_dlist_node *prev = NULL;
    for (struct int_dlist_node *node = list->head; node; node = node->next, i++)
    {
        assert(i < count && node->value == expected[i]);
        assert(node->prev == prev);
        prev = node;
    }
    assert(i == count && prev == list->tail);
}

#define CHECK(PFX, list, ...)                                                          \
    do                                                                                  \
    {                                                                                   \
        const int expected[] = { __VA_ARGS__ };                                         \
        PASTE(PFX, _check)(list, expected, sizeof(expected) / sizeof(expected[0]));     \
    } while (0)

#define CHECK_EMPTY(PFX, list) PASTE(PFX, _check)(list, NULL, 0)

// The API shared by both variants, with PFX the function prefix and SNAME the struct name.
#define TEST_LINKED_LIST_COMMON(PFX, SNAME)                                            \
    static void PASTE(test_linked_list_, PFX)(void)                                     \
    {                                                                                   \
        struct PASTE(SNAME, _vtabv) vtabv = { .comp = int_comp };                       \
        struct SNAME list, other, clone;                                                \
        struct PASTE(SNAME, _node) *node = NULL;                                        \
        int out = 0;                                                                    \
        size_t index = 0;                                                               \
                                                                                        \
        assert(PASTE(PFX, _init)(&list, &vtabv) == CVX_FLAG_OK);                        \
        CHECK_EMPTY(PFX, &list);                                                        \
        assert(PASTE(PFX, _empty)(&list));                                              \
        assert(PASTE(PFX, _front)(&list, &out) == CVX_FLAG_EMPTY);                      \
        assert(PASTE(PFX, _back)(&list, &out) == CVX_FLAG_EMPTY);                       \
        assert(PASTE(PFX, _pop)(&list, 0, &out) == CVX_FLAG_EMPTY);                     \
        assert(PASTE(PFX, _replace)(&list, 0, 1, NULL) == CVX_FLAG_EMPTY);              \
        assert(PASTE(PFX, _get)(&list, 0, &out) == CVX_FLAG_RANGE);                     \
                                                                                        \
        assert(PASTE(PFX, _push)(&list, 0, 2) == CVX_FLAG_OK);                          \
        assert(PASTE(PFX, _push)(&list, 1, 4) == CVX_FLAG_OK);                          \
        assert(PASTE(PFX, _push)(&list, 0, 1) == CVX_FLAG_OK);                          \
        assert(PASTE(PFX, _push)(&list, 2, 3) == CVX_FLAG_OK);                          \
        assert(PASTE(PFX, _push)(&list, 9, 0) == CVX_FLAG_RANGE);                       \
        CHECK(PFX, &list, 1, 2, 3, 4);                                                  \
        assert(PASTE(PFX, _front)(&list, &out) == CVX_FLAG_OK && out == 1);             \
        assert(PASTE(PFX, _back)(&list, &out) == CVX_FLAG_OK && out == 4);              \
        for (int i = 0; i < 4; i++)                                                     \
            assert(PASTE(PFX, _get)(&list, (size_t)i, &out) == CVX_FLAG_OK && out == i + 1); \
                                                                                        \
        assert(PASTE(PFX, _replace)(&list, 1, 20, &out) == CVX_FLAG_OK && out == 2);    \
        assert(PASTE(PFX, _replace)(&list, 4, 0, NULL) == CVX_FLAG_RANGE);              \
        CHECK(PFX, &list, 1, 20, 3, 4);                                                 \
        assert(PASTE(PFX, _replace)(&list, 1, 2, NULL) == CVX_FLAG_OK);                 \
                                                                                        \
        assert(PASTE(PFX, _index_of)(&list, 3, &index) == CVX_FLAG_OK && index == 2);   \
        assert(PASTE(PFX, _index_of)(&list, 99, NULL) == CVX_FLAG_NOT_FOUND);           \
        assert(PASTE(PFX, _find)(&list, 3, &node) == CVX_FLAG_OK && node->value == 3);  \
        assert(PASTE(PFX, _find)(&list, 99, &node) == CVX_FLAG_NOT_FOUND);              \
                                                                                        \
        assert(PASTE(PFX, _insert_after)(&list, NULL, 0) == CVX_FLAG_OK);               \
        assert(PASTE(PFX, _insert_after)(&list, list.tail, 5) == CVX_FLAG_OK);          \
        assert(PASTE(PFX, _find)(&list, 2, &node) == CVX_FLAG_OK);                      \
        assert(PASTE(PFX, _insert_after)(&list, node, 25) == CVX_FLAG_OK);              \
        CHECK(PFX, &list, 0, 1, 2, 25, 3, 4, 5);                                        \
                                                                                        \
        assert(PASTE(PFX, _pop)(&list, 6, &out) == CVX_FLAG_OK && out == 5);            \
        assert(PASTE(PFX, _pop)(&list, 0, &out) == CVX_FLAG_OK && out == 0);            \
        assert(PASTE(PFX, _pop)(&list, 2, NULL) == CVX_FLAG_OK);                        \
        assert(PASTE(PFX, _pop)(&list, 4, &out) == CVX_FLAG_RANGE);                     \
        CHECK(PFX, &list, 1, 2, 3, 4);                                                  \
                                                                                        \
        assert(PASTE(PFX, _reverse)(&list) == CVX_FLAG_OK);                             \
        CHECK(PFX, &list, 4, 3, 2, 1);                                                  \
        assert(PASTE(PFX, _sort)(&list) == CVX_FLAG_OK);                                \
        CHECK(PFX, &list, 1, 2, 3, 4);                                                  \
                                                                                        \
        assert(PASTE(PFX, _init)(&other, &vtabv) == CVX_FLAG_OK);                       \
        assert(PASTE(PFX, _splice)(&list, 0, &other) == CVX_FLAG_OK);                   \
        assert(PASTE(PFX, _splice)(&list, 0, &list) == CVX_FLAG_INVALID);               \
        assert(PASTE(PFX, _splice)(&list, 5, &other) == CVX_FLAG_RANGE);                \
        assert(PASTE(PFX, _push)(&other, 0, 10) == CVX_FLAG_OK);                        \
        assert(PASTE(PFX, _push)(&other, 1, 11) == CVX_FLAG_OK);                        \
        assert(PASTE(PFX, _splice)(&list, 1, &other) == CVX_FLAG_OK);                   \
        CHECK_EMPTY(PFX, &other);                                                       \
        CHECK(PFX, &list, 1, 10, 11, 2, 3, 4);                                          \
        assert(PASTE(PFX, _push)(&other, 0, 12) == CVX_FLAG_OK);                        \
        assert(PASTE(PFX, _splice)(&list, list.count, &other) == CVX_FLAG_OK);          \
        assert(PASTE(PFX, _push)(&other, 0, 0) == CVX_FLAG_OK);                         \
        assert(PASTE(PFX, _splice)(&list, 0, &other) == CVX_FLAG_OK);                   \
        CHECK(PFX, &list, 0, 1, 10, 11, 2, 3, 4, 12);                                   \
        assert(PASTE(PFX, _push)(&list, list.count, 13) == CVX_FLAG_OK);                \
        CHECK(PFX, &list, 0, 1, 10, 11, 2, 3, 4, 12, 13);                               \
                                                                                        \
        int cmp = 0;                                                                    \
        assert(PASTE(PFX, _clone)(&list, &clone) == CVX_FLAG_OK);                       \
        CHECK(PFX, &clone, 0, 1, 10, 11, 2, 3, 4, 12, 13);                              \
        assert(PASTE(PFX, _compare)(&list, &clone, &cmp) == CVX_FLAG_OK && cmp == 0);   \
        assert(PASTE(PFX, _replace)(&clone, 3, 99, NULL) == CVX_FLAG_OK);               \
        assert(PASTE(PFX, _compare)(&list, &clone, &cmp) == CVX_FLAG_OK && cmp < 0);    \
        assert(PASTE(PFX, _truncate)(&clone, 2) == CVX_FLAG_OK);                        \
        CHECK(PFX, &clone, 0, 1);                                                       \
        assert(PASTE(PFX, _compare)(&list, &clone, &cmp) == CVX_FLAG_OK && cmp > 0);    \
        assert(PASTE(PFX, _push)(&clone, clone.count, 7) == CVX_FLAG_OK);               \
        CHECK(PFX, &clone, 0, 1, 7);                                                    \
        assert(PASTE(PFX, _truncate)(&clone, 0) == CVX_FLAG_OK);                        \
        CHECK_EMPTY(PFX, &clone);                                                       \
        PASTE(PFX, _drop)(&clone);                                                      \
                                                                                        \
        assert(PASTE(PFX, _truncate)(&list, 0) == CVX_FLAG_OK);                         \
        unsigned seed = 12345;                                                          \
        for (int i = 0; i < 101; i++)                                                   \
        {                                                                               \
            seed = seed * 1103515245u + 12345u;                                         \
            assert(PASTE(PFX, _push)(&list, list.count, (int)(seed >> 16) % 50) == CVX_FLAG_OK); \
        }                                                                               \
        assert(PASTE(PFX, _sort)(&list) == CVX_FLAG_OK);                                \
        assert(list.count == 101);                                                      \
        int last = -1;                                                                  \
        for (size_t i = 0; i < list.count; i++)                                         \
        {                                                                               \
            assert(PASTE(PFX, _get)(&list, i, &out) == CVX_FLAG_OK && out >= last);     \
            last = out;                                                                 \
        }                                                                               \
        assert(PASTE(PFX, _back)(&list, &out) == CVX_FLAG_OK && out == last);           \
                                                                                        \
        struct PASTE(SNAME, _vtabv) no_comp = { 0 };                                    \
        list.vtabv = &no_comp;                                                          \
        assert(PASTE(PFX, _sort)(&list) == CVX_FLAG_VTAB);                              \
        assert(PASTE(PFX, _find)(&list, 1, &node) == CVX_FLAG_VTAB);                    \
        assert(PASTE(PFX, _index_of)(&list, 1, NULL) == CVX_FLAG_VTAB);                 \
                                                                                        \
        PASTE(PFX, _drop)(&list);                                                       \
        PASTE(PFX, _drop)(&other);                                                      \
        printf("linked_list (" #PFX "): OK\n");                                         \
    }

TEST_LINKED_LIST_COMMON(isl, int_slist)
TEST_LINKED_LIST_COMMON(idl, int_dlist)

static void test_linked_list_single_remove_after(void)
{
    struct int_slist list;
    int out = 0;

    assert(isl_init(&list, NULL) == CVX_FLAG_OK);
    assert(isl_remove_after(&list, NULL, &out) == CVX_FLAG_EMPTY);

    for (int i = 1; i <= 4; i++)
        assert(isl_push(&list, list.count, i) == CVX_FLAG_OK);

    assert(isl_remove_after(&list, list.tail, &out) == CVX_FLAG_RANGE);
    assert(isl_remove_after(&list, NULL, &out) == CVX_FLAG_OK && out == 1);
    CHECK(isl, &list, 2, 3, 4);
    assert(isl_remove_after(&list, list.head, &out) == CVX_FLAG_OK && out == 3);
    CHECK(isl, &list, 2, 4);
    assert(isl_remove_after(&list, list.head, NULL) == CVX_FLAG_OK);
    CHECK(isl, &list, 2);
    assert(isl_remove_after(&list, NULL, NULL) == CVX_FLAG_OK);
    CHECK_EMPTY(isl, &list);

    isl_drop(&list);
    printf("linked_list single remove_after: OK\n");
}

static void test_linked_list_double_node_ops(void)
{
    struct int_dlist list;
    int out = 0;

    assert(idl_init(&list, NULL) == CVX_FLAG_OK);
    assert(idl_insert_before(&list, NULL, 3) == CVX_FLAG_OK);
    assert(idl_insert_before(&list, NULL, 4) == CVX_FLAG_OK);
    assert(idl_insert_before(&list, list.head, 1) == CVX_FLAG_OK);
    assert(idl_insert_before(&list, list.tail->prev, 2) == CVX_FLAG_OK);
    CHECK(idl, &list, 1, 2, 3, 4);

    assert(idl_remove(&list, list.head->next, &out) == CVX_FLAG_OK && out == 2);
    CHECK(idl, &list, 1, 3, 4);
    assert(idl_remove(&list, list.tail, &out) == CVX_FLAG_OK && out == 4);
    CHECK(idl, &list, 1, 3);
    assert(idl_remove(&list, list.head, NULL) == CVX_FLAG_OK);
    CHECK(idl, &list, 3);
    assert(idl_remove(&list, list.head, &out) == CVX_FLAG_OK && out == 3);
    CHECK_EMPTY(idl, &list);

    // Walking backwards from the tail for indices past the middle.
    for (int i = 0; i < 9; i++)
        assert(idl_push(&list, list.count, i) == CVX_FLAG_OK);
    for (size_t i = 0; i < list.count; i++)
        assert(idl_get(&list, i, &out) == CVX_FLAG_OK && out == (int)i);
    assert(idl_pop(&list, 7, &out) == CVX_FLAG_OK && out == 7);
    CHECK(idl, &list, 0, 1, 2, 3, 4, 5, 6, 8);

    idl_drop(&list);
    printf("linked_list double insert_before/remove: OK\n");
}

static void test_linked_list_of_owned_strings(void)
{
    struct StrListVtabV vtabv = { .copy = str_copy, .drop = str_drop };
    struct StrList list, clone, other;
    char *out = NULL;
    size_t drops = str_drops;

    assert(slInit(&list, &vtabv) == CVX_FLAG_OK);
    assert(slPush(&list, 0, str_new("a")) == CVX_FLAG_OK);
    assert(slPush(&list, 1, str_new("b")) == CVX_FLAG_OK);
    assert(slPush(&list, 2, str_new("c")) == CVX_FLAG_OK);

    assert(slClone(&list, &clone) == CVX_FLAG_OK);
    char *cloned = NULL;
    assert(slGet(&clone, 0, &cloned) == CVX_FLAG_OK && strcmp(cloned, "a") == 0);
    assert(slGet(&list, 0, &out) == CVX_FLAG_OK && out != cloned);

    assert(slPop(&list, 1, NULL) == CVX_FLAG_OK); // drops "b"
    assert(str_drops == drops + 1);
    assert(slReplace(&list, 0, str_new("A"), NULL) == CVX_FLAG_OK); // drops "a"
    assert(str_drops == drops + 2);
    assert(slPop(&list, 0, &out) == CVX_FLAG_OK && strcmp(out, "A") == 0); // caller owns it
    free(out);
    assert(str_drops == drops + 2);

    assert(slInit(&other, &vtabv) == CVX_FLAG_OK);
    assert(slPush(&other, 0, str_new("z")) == CVX_FLAG_OK);
    assert(slSplice(&list, 0, &other) == CVX_FLAG_OK);
    assert(slTruncate(&list, 1) == CVX_FLAG_OK); // drops "c"
    assert(str_drops == drops + 3);

    slDrop(&list);  // drops "z"
    slDrop(&clone); // drops the copies of "a", "b", "c"
    slDrop(&other);
    assert(str_drops == drops + 7);

    printf("linked_list<char *> (copy/drop): OK\n");
}

static void ih_check(struct int_heap *heap)
{
    for (size_t i = 1; i < heap->count; i++)
        assert(heap->buffer[(i - 1) / 2] >= heap->buffer[i]);
}

static void ih_pop_all_descending(struct int_heap *heap, size_t expected_count)
{
    int last = 0;
    for (size_t i = 0; i < expected_count; i++)
    {
        int out = 0;
        assert(ih_pop(heap, &out) == CVX_FLAG_OK);
        assert(i == 0 || out <= last);
        last = out;
        ih_check(heap);
    }
    assert(ih_empty(heap));
}

static int int_comp_reversed(int const *a, int const *b)
{
    return int_comp(b, a);
}

static void test_binary_heap(void)
{
    struct int_heap_vtabv vtabv = { .comp = int_comp };
    struct int_heap_vtabv no_comp = { 0 };
    struct int_heap heap, other, clone;
    int out = 0;

    assert(ih_init(&heap, NULL, 0) == CVX_FLAG_VTAB);
    assert(ih_init(&heap, &no_comp, 0) == CVX_FLAG_VTAB);
    ih_drop(&heap);

    assert(ih_init(&heap, &vtabv, 0) == CVX_FLAG_OK);
    assert(ih_empty(&heap) && ih_capacity(&heap) == 0);
    assert(ih_peek(&heap, &out) == CVX_FLAG_EMPTY);
    assert(ih_pop(&heap, &out) == CVX_FLAG_EMPTY);
    assert(ih_replace(&heap, 1, &out) == CVX_FLAG_EMPTY);
    assert(ih_push_pop(&heap, 7, &out) == CVX_FLAG_OK && out == 7);
    assert(ih_empty(&heap));
    assert(ih_clear(&heap) == CVX_FLAG_OK);
    assert(ih_shrink(&heap) == CVX_FLAG_OK);

    unsigned seed = 42;
    for (int i = 0; i < 200; i++)
    {
        seed = seed * 1103515245u + 12345u;
        assert(ih_push(&heap, (int)(seed >> 16) % 1000) == CVX_FLAG_OK);
        ih_check(&heap);
    }
    assert(ih_count(&heap) == 200);
    ih_pop_all_descending(&heap, 200);

    // Reversing comp gives a min-heap.
    struct int_heap_vtabv min_vtabv = { .comp = int_comp_reversed };
    struct int_heap min_heap;
    assert(ih_init(&min_heap, &min_vtabv, 4) == CVX_FLAG_OK);
    int unordered[] = { 5, 1, 4, 2, 3 };
    for (int i = 0; i < 5; i++)
        assert(ih_push(&min_heap, unordered[i]) == CVX_FLAG_OK);
    for (int i = 1; i <= 5; i++)
        assert(ih_pop(&min_heap, &out) == CVX_FLAG_OK && out == i);
    ih_drop(&min_heap);

    int values[150];
    for (int i = 0; i < 150; i++)
        values[i] = (i * 37) % 150;
    ih_drop(&heap);
    assert(ih_heapify(&heap, &no_comp, values, 150) == CVX_FLAG_VTAB);
    assert(ih_heapify(&heap, &vtabv, NULL, 0) == CVX_FLAG_OK);
    assert(ih_empty(&heap) && ih_capacity(&heap) == 0);
    assert(ih_heapify(&heap, &vtabv, values, 150) == CVX_FLAG_OK);
    ih_check(&heap);
    assert(ih_count(&heap) == 150 && ih_capacity(&heap) == 150);
    assert(ih_peek(&heap, &out) == CVX_FLAG_OK && out == 149);
    ih_pop_all_descending(&heap, 150);

    assert(ih_push(&heap, 5) == CVX_FLAG_OK);
    assert(ih_push(&heap, 3) == CVX_FLAG_OK);
    assert(ih_push(&heap, 1) == CVX_FLAG_OK);
    assert(ih_push_pop(&heap, 10, &out) == CVX_FLAG_OK && out == 10);
    assert(ih_push_pop(&heap, 5, &out) == CVX_FLAG_OK && out == 5); // ties return item
    assert(ih_count(&heap) == 3 && ih_peek(&heap, &out) == CVX_FLAG_OK && out == 5);
    assert(ih_push_pop(&heap, 4, &out) == CVX_FLAG_OK && out == 5);
    assert(ih_peek(&heap, &out) == CVX_FLAG_OK && out == 4);
    assert(ih_replace(&heap, 9, &out) == CVX_FLAG_OK && out == 4); // replace always pops the old top
    assert(ih_peek(&heap, &out) == CVX_FLAG_OK && out == 9);
    assert(ih_replace(&heap, 0, NULL) == CVX_FLAG_OK);
    ih_check(&heap);
    assert(ih_count(&heap) == 3 && ih_peek(&heap, &out) == CVX_FLAG_OK && out == 3);

    assert(ih_init(&other, &vtabv, 0) == CVX_FLAG_OK);
    assert(ih_merge(&heap, &other) == CVX_FLAG_OK);
    assert(ih_merge(&heap, &heap) == CVX_FLAG_INVALID);
    for (int i = 0; i < 20; i++)
        assert(ih_push(&other, i * 2) == CVX_FLAG_OK);
    assert(ih_merge(&heap, &other) == CVX_FLAG_OK);
    assert(ih_empty(&other) && ih_capacity(&other) > 0);
    assert(ih_count(&heap) == 23);
    ih_check(&heap);
    assert(ih_peek(&heap, &out) == CVX_FLAG_OK && out == 38);

    // other is smaller than heap, so the new elements are sifted up instead.
    assert(ih_push(&other, 100) == CVX_FLAG_OK);
    assert(ih_merge(&heap, &other) == CVX_FLAG_OK);
    assert(ih_empty(&other) && ih_count(&heap) == 24);
    ih_check(&heap);
    assert(ih_pop(&heap, &out) == CVX_FLAG_OK && out == 100);

    assert(ih_clone(&heap, &clone) == CVX_FLAG_OK);
    assert(ih_count(&clone) == 23 && ih_capacity(&clone) == ih_capacity(&heap));
    assert(ih_pop(&clone, NULL) == CVX_FLAG_OK);
    assert(ih_peek(&heap, &out) == CVX_FLAG_OK && out == 38);
    ih_pop_all_descending(&clone, 22);

    size_t capacity = ih_capacity(&heap);
    assert(ih_clear(&heap) == CVX_FLAG_OK);
    assert(ih_empty(&heap) && ih_capacity(&heap) == capacity);
    assert(ih_reserve(&heap, 500) == CVX_FLAG_OK && ih_capacity(&heap) == 500);
    assert(ih_reserve(&heap, 10) == CVX_FLAG_OK && ih_capacity(&heap) == 500);
    assert(ih_push(&heap, 1) == CVX_FLAG_OK);
    assert(ih_push(&heap, 2) == CVX_FLAG_OK);
    assert(ih_shrink(&heap) == CVX_FLAG_OK && ih_capacity(&heap) == 2);
    ih_pop_all_descending(&heap, 2);
    assert(ih_shrink(&heap) == CVX_FLAG_OK && ih_capacity(&heap) == 0 && heap.buffer == NULL);

    ih_drop(&heap);
    ih_drop(&other);
    ih_drop(&clone);
    printf("binary_heap: OK\n");
}

static int str_comp(char *const *a, char *const *b)
{
    return strcmp(*a, *b);
}

static void test_binary_heap_of_owned_strings(void)
{
    struct StrHeapVtabV vtabv = { .copy = str_copy, .comp = str_comp, .drop = str_drop };
    struct StrHeap heap, clone;
    char *out = NULL;
    size_t drops = str_drops;

    assert(shInit(&heap, &vtabv, 0) == CVX_FLAG_OK);
    assert(shPush(&heap, str_new("b")) == CVX_FLAG_OK);
    assert(shPush(&heap, str_new("d")) == CVX_FLAG_OK);
    assert(shPush(&heap, str_new("a")) == CVX_FLAG_OK);
    assert(shPush(&heap, str_new("c")) == CVX_FLAG_OK);

    assert(shClone(&heap, &clone) == CVX_FLAG_OK);
    char *cloned = NULL;
    assert(shPeek(&clone, &cloned) == CVX_FLAG_OK && strcmp(cloned, "d") == 0);
    assert(shPeek(&heap, &out) == CVX_FLAG_OK && out != cloned);

    assert(shPop(&heap, NULL) == CVX_FLAG_OK); // drops "d"
    assert(str_drops == drops + 1);
    assert(shPushPop(&heap, str_new("z"), NULL) == CVX_FLAG_OK); // "z" is the new top, dropped
    assert(str_drops == drops + 2);
    assert(shReplace(&heap, str_new("e"), NULL) == CVX_FLAG_OK); // drops "c"
    assert(str_drops == drops + 3);
    assert(shPop(&heap, &out) == CVX_FLAG_OK && strcmp(out, "e") == 0); // caller owns it
    free(out);

    char *more[] = { str_new("x"), str_new("y") };
    struct StrHeap built;
    assert(shHeapify(&built, &vtabv, more, 2) == CVX_FLAG_OK);
    assert(shMerge(&heap, &built) == CVX_FLAG_OK);
    shDrop(&built);
    assert(str_drops == drops + 3);
    assert(shPeek(&heap, &out) == CVX_FLAG_OK && strcmp(out, "y") == 0);
    assert(shClear(&heap) == CVX_FLAG_OK); // drops "a", "b", "x", "y"
    assert(str_drops == drops + 7);

    shDrop(&heap);
    shDrop(&clone); // drops the copies of "a", "b", "c", "d"
    assert(str_drops == drops + 11);

    printf("binary_heap<char *> (copy/drop): OK\n");
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
    test_linked_list_isl();
    test_linked_list_idl();
    test_linked_list_single_remove_after();
    test_linked_list_double_node_ops();
    test_linked_list_of_owned_strings();
    test_binary_heap();
    test_binary_heap_of_owned_strings();
    printf("all cvx2 smoke tests passed\n");
    return 0;
}
