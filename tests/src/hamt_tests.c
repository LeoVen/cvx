#include "cvx/core.h"
#include "cvx/flags.h"
#include "cvx_test.h"
#include "cvxt_alloc.h"
#include "cvxt_funcs.h"
#include "hamt.h"
#include <stdbool.h>
#include <stdint.h>

static struct HAMTVtabK int_vtabk = {
    .comp = compare_void_as_int,
    .copy = NULL,
    .drop = NULL,
    .hash = hash_void_as_int,
};

static void hamt_test_init_validation(struct cvxtest *t)
{
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, NULL, NULL) == CVX_FLAG_VTAB);
    CVXCHECK(t, hamt_init(&hamt, &(struct HAMTVtabK){ 0 }, NULL) == CVX_FLAG_VTAB);
}

static void hamt_test_init_state(struct cvxtest *t)
{
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &int_vtabk, NULL) == CVX_FLAG_OK);

    CVXCHECK(t, hamt.vtabk == &int_vtabk);
    CVXCHECK(t, hamt.count == 0);
    CVXCHECK(t, hamt.root == NULL);
    CVXCHECK(t, hamt.vtabv == NULL);

    hamt_drop(&hamt);
}

static void hamt_test_clone(struct cvxtest *t)
{
    CVXCHECK(t, 1 == true);
}

static void hamt_test_insert_single(struct cvxtest *t)
{
    struct HAMTVtabK vtabk = {
        .comp = compare_void_as_int,
        .hash = hash_void_as_int_collide_always,
    };
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &vtabk, NULL) == CVX_FLAG_OK);

    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(10), cvxt_void(12)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt.root != NULL);
    CVXCHECK(t, hamt.root->type == CVX_HAMT_NODE_LEAF);
    CVXCHECK(t, hamt.root->as.leaf.key == cvxt_void(10));
    CVXCHECK(t, hamt.root->as.leaf.val == cvxt_void(12));
    CVXCHECK(t, hamt.root->as.leaf.next == NULL);

    hamt_drop(&hamt);
}

static void hamt_test_insert_collide_single(struct cvxtest *t)
{
    struct HAMTVtabK vtabk = {
        .comp = compare_void_as_int,
        .hash = hash_void_as_int_collide_always,
    };
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &vtabk, NULL) == CVX_FLAG_OK);

    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(10), cvxt_void(20)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(30), cvxt_void(40)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(50), cvxt_void(60)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt.root->as.leaf.next != NULL);
    CVXCHECK(t, hamt.root->as.leaf.key == cvxt_void(10));
    CVXCHECK(t, hamt.root->as.leaf.next->key == cvxt_void(30));

    hamt_drop(&hamt);
}

static void hamt_test_insert_duplicate(struct cvxtest *t)
{
    struct HAMTVtabK vtabk = {
        .comp = compare_void_as_int,
        .hash = hash_void_as_int_collide_always,
    };
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &vtabk, NULL) == CVX_FLAG_OK);

    // First entry
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(10), cvxt_void(20)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(10), cvxt_void(20)) == CVX_FLAG_DUPLICATE);

    // Second ->next entry
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(20), cvxt_void(30)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(20), cvxt_void(30)) == CVX_FLAG_DUPLICATE);

    hamt_drop(&hamt);
}

static void hamt_test_insert_root_split(struct cvxtest *t)
{
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &int_vtabk, NULL) == CVX_FLAG_OK);

    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(11), cvxt_void(12)) == CVX_FLAG_OK);

    // Split
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(12), cvxt_void(12)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt.root->type == CVX_HAMT_NODE_INTERNAL);

    hamt_drop(&hamt);
}

static void hamt_test_insert_make_subtree(struct cvxtest *t)
{
    struct HAMTNode *node;
    struct HAMTVtabK vtabk = {
        .comp = compare_void_as_int,
        .hash = hash_void_as_int_ret_number,
    };
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &vtabk, NULL) == CVX_FLAG_OK);

    // clang-format off
    int values[] = {
        0b1000'01000'01000'11010,

        0b1000'01000'01010'11010,
        0b1000'01001'01010'11010,
        0b1000'01011'01010'11010,

        0b1001'01000'01010'11010,
        0b1011'01000'01010'11010,

        0b1000'01001'01010'10110,
        0b1000'01011'01010'10110,
    };
    // clang-format on

    // Cause a __make_subtree by having different hashes, but same depth 0 bits
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(values[0]), cvxt_void(1)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(values[1]), cvxt_void(2)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt.root->type == CVX_HAMT_NODE_INTERNAL);
    CVXCHECK(t, cvx_popcount(hamt.root->as.internal.bitmap) == 1);

    // Insert more into that subtree
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(values[2]), cvxt_void(3)) == CVX_FLAG_OK);
    CVXCHECK(t, cvx_popcount(hamt.root->as.internal.bitmap) == 1);
    CVXCHECK(t, cvx_popcount(hamt.root->as.internal.children[0]->as.internal.bitmap) == 2);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(values[3]), cvxt_void(4)) == CVX_FLAG_OK);
    CVXCHECK(t, cvx_popcount(hamt.root->as.internal.bitmap) == 1);
    CVXCHECK(t, cvx_popcount(hamt.root->as.internal.children[0]->as.internal.bitmap) == 2);
    // values[0] is leaf, but values[1..3] are all in the same subtree because at depth 1 they match
    node = hamt.root->as.internal.children[0];
    CVXCHECK(t, node->as.internal.children[0]->type == CVX_HAMT_NODE_LEAF);
    CVXCHECK(t, node->as.internal.children[0]->as.leaf.val == cvxt_void(1));
    CVXCHECK(t, node->as.internal.children[1]->type == CVX_HAMT_NODE_INTERNAL);
    CVXCHECK(t, cvx_popcount(node->as.internal.children[1]->as.internal.bitmap) == 3);

    // Different subtree at depth 1 -> create another at depth 2
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(values[4]), cvxt_void(5)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(values[5]), cvxt_void(6)) == CVX_FLAG_OK);
    node = hamt.root->as.internal.children[0]->as.internal.children[1];
    // because values[1] bits at depth 2 is 0b1000, it lands on children[0]
    // values[2,3] bits are greater than 0b1000
    CVXCHECK(t, node->as.internal.children[0]->type == CVX_HAMT_NODE_INTERNAL);
    CVXCHECK(t, node->as.internal.children[1]->type == CVX_HAMT_NODE_LEAF);
    CVXCHECK(t, node->as.internal.children[2]->type == CVX_HAMT_NODE_LEAF);
    // values[4,5] collide at depth 2 with values[1], so its made into a subtree
    node = node->as.internal.children[0];
    CVXCHECK(t, cvx_popcount(node->as.internal.bitmap) == 3);
    // values[1,4,5] bits at depth 3 respectively: 0b1000, 0b1001, 0b1011, so this is the order
    CVXCHECK(t, node->as.internal.children[0]->as.leaf.val == cvxt_void(2));
    CVXCHECK(t, node->as.internal.children[1]->as.leaf.val == cvxt_void(5));
    CVXCHECK(t, node->as.internal.children[2]->as.leaf.val == cvxt_void(6));

    // Different subtree at root
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(values[6]), cvxt_void(7)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(values[7]), cvxt_void(8)) == CVX_FLAG_OK);
    CVXCHECK(t, cvx_popcount(hamt.root->as.internal.bitmap) == 2);

    hamt_drop(&hamt);
}

static void hamt_test_get_empty(struct cvxtest *t)
{
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &int_vtabk, NULL) == CVX_FLAG_OK);

    void *out;
    CVXCHECK(t, hamt_get(&hamt, cvxt_void(10), &out) == CVX_FLAG_EMPTY);

    hamt_drop(&hamt);
}

static void hamt_test_get_not_found_leaf(struct cvxtest *t)
{
    struct HAMTVtabK vtabk = {
        .comp = compare_void_as_int,
        .hash = hash_void_as_int_collide_5,
    };
    struct HAMT hamt;
    void *out;
    CVXCHECK(t, hamt_init(&hamt, &vtabk, NULL) == CVX_FLAG_OK);

    // Long chain of leaves
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(10), cvxt_void(10)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(20), cvxt_void(20)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(30), cvxt_void(30)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(40), cvxt_void(40)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(50), cvxt_void(50)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt.root->type == CVX_HAMT_NODE_LEAF);

    CVXCHECK(t, hamt_get(&hamt, cvxt_void(100), &out) == CVX_FLAG_NOT_FOUND);

    hamt_drop(&hamt);
}

static void hamt_test_get_not_found_internal(struct cvxtest *t)
{
    struct HAMTVtabK vtabk = {
        .comp = compare_void_as_int,
        .hash = hash_void_as_int_ret_number,
    };
    struct HAMT hamt;
    void *out;
    CVXCHECK(t, hamt_init(&hamt, &vtabk, NULL) == CVX_FLAG_OK);

    // Not found after checking an internal node
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(0b100'01100), cvxt_void(10)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(0b101'01100), cvxt_void(20)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt.root->type == CVX_HAMT_NODE_INTERNAL);

    CVXCHECK(t, hamt_get(&hamt, cvxt_void(0b101'00100), &out) == CVX_FLAG_NOT_FOUND);

    hamt_drop(&hamt);
}

static void hamt_test_get_not_found_depth_max(struct cvxtest *t)
{
    // TODO:
}

static void hamt_test_get_single_leaf(struct cvxtest *t)
{
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &int_vtabk, NULL) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(10), cvxt_void(20)) == CVX_FLAG_OK);

    void *out;
    CVXCHECK(t, hamt_get(&hamt, cvxt_void(10), &out) == CVX_FLAG_OK);
    CVXCHECK(t, cvxt_int(out) == 20);

    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(30), cvxt_void(40)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_get(&hamt, cvxt_void(30), &out) == CVX_FLAG_OK);
    CVXCHECK(t, cvxt_int(out) == 40);

    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(50), cvxt_void(60)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(70), cvxt_void(80)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_get(&hamt, cvxt_void(70), &out) == CVX_FLAG_OK);
    CVXCHECK(t, cvxt_int(out) == 80);

    hamt_drop(&hamt);
}

static void hamt_test_get_optional_out(struct cvxtest *t)
{
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &int_vtabk, NULL) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(10), cvxt_void(20)) == CVX_FLAG_OK);

    CVXCHECK(t, hamt_get(&hamt, cvxt_void(10), NULL) == CVX_FLAG_OK);

    hamt_drop(&hamt);
}

static void hamt_test_alloc_insert(struct cvxtest *t)
{
    struct HAMT hamt;
    enum cvx_flags flag;

    // clang-format off
    int values[] = {
        0b10100'01000'01000'01000'11010,
        0b10100'01000'01000'01010'11010,
        0b10100'01000'01001'01010'11010,
        0b10100'01000'01011'01010'11010,
        0b10100'01001'01000'01010'11010,
        0b10100'01011'01000'01010'11010,
        0b10100'01000'01001'01010'10110,
        0b10100'01001'01001'01010'10110,
        0b10100'01011'01001'01010'10110,
        0b10100'01111'01001'01010'10110,
        0b10101'01111'01001'01010'10110,
        0b10110'01111'01001'01010'10110,
    };
    // clang-format on

    int key = 0b11110'01111'01001'01010'10110;

    for (size_t i = 0; i < 100; i++)
    {
        CVXT_MALLOC_RESET();
        CVXCHECK(t, hamt_init(&hamt, &int_vtabk, NULL) == CVX_FLAG_OK);

        for (size_t j = 0; j < sizeof(values) / sizeof(values[0]); j++)
            CVXCHECK(t,
                     hamt_insert(&hamt, cvxt_void(values[j]), cvxt_void(values[j])) == CVX_FLAG_OK);

        size_t count = hamt.count;

        CVXT_MALLOC_FAIL_AFTER(i);
        flag = hamt_insert(&hamt, cvxt_void(key), NULL);
        if (flag != CVX_FLAG_OK)
        {
            CVXCHECK(t, flag == CVX_FLAG_ALLOC);
            CVXCHECK(t, count == hamt.count);
            CVXCHECK(t, hamt_get(&hamt, cvxt_void(key), NULL) == CVX_FLAG_NOT_FOUND);

            for (size_t j = 0; j < sizeof(values) / sizeof(values[0]); j++)
            {
                void *out;
                CVXCHECK(t, hamt_get(&hamt, cvxt_void(values[j]), &out) == CVX_FLAG_OK);
                CVXCHECK(t, cvxt_int(out) == values[j]);
            }

            hamt_drop(&hamt);
        }
        else
        {
            CVXCHECK(t, i != 0);

            void *out;
            CVXCHECK(t, hamt_get(&hamt, cvxt_void(key), &out) == CVX_FLAG_OK);
            CVXCHECK(t, cvxt_int(out) == 0);

            hamt_drop(&hamt);
            break;
        }
    }

    CVXT_MALLOC_RESET();
}

CVXSUITE(cvx_hamt_tests_init, hamt_test_init_validation, hamt_test_init_state, )
CVXSUITE(cvx_hamt_tests_clone, hamt_test_clone, )
CVXSUITE(cvx_hamt_tests_insert, hamt_test_insert_single, hamt_test_insert_collide_single,
         hamt_test_insert_duplicate, hamt_test_insert_root_split, hamt_test_insert_make_subtree, )
CVXSUITE(cvx_hamt_tests_get, hamt_test_get_empty, hamt_test_get_single_leaf,
         hamt_test_get_not_found_leaf, hamt_test_get_not_found_internal,
         hamt_test_get_optional_out, )
CVXSUITE(cvx_hamt_alloc, hamt_test_alloc_insert, )

CVXMAIN(HAMT, cvx_hamt_tests_init, cvx_hamt_tests_clone, cvx_hamt_tests_insert, cvx_hamt_tests_get,
        cvx_hamt_alloc)
