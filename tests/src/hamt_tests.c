#include "common.h"
#include "cvx/flags.h"
#include "cvx_test.h"
#include "hamt.h"
#include <stdbool.h>
#include <stdint.h>

static struct HAMTVtabK int_vtabk = {
    .comp = compare_void_as_int,
    .copy = NULL,
    .drop = NULL,
    .hash = hash_void_as_int_collide_5,
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
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &int_vtabk, NULL) == CVX_FLAG_OK);

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
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &int_vtabk, NULL) == CVX_FLAG_OK);

    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(10), cvxt_void(20)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(30), cvxt_void(40)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt.root->as.leaf.next != NULL);
    CVXCHECK(t, hamt.root->as.leaf.key == cvxt_void(10));
    CVXCHECK(t, hamt.root->as.leaf.next->key == cvxt_void(30));

    hamt_drop(&hamt);
}

static void hamt_test_insert_duplicate(struct cvxtest *t)
{
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &int_vtabk, NULL) == CVX_FLAG_OK);

    // First entry
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(10), cvxt_void(20)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(10), cvxt_void(20)) == CVX_FLAG_DUPLICATE);

    // Second ->next entry
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(20), cvxt_void(30)) == CVX_FLAG_OK);
    CVXCHECK(t, hamt_insert(&hamt, cvxt_void(20), cvxt_void(30)) == CVX_FLAG_DUPLICATE);

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

static void hamt_test_get(struct cvxtest *t)
{
    struct HAMT hamt;
    CVXCHECK(t, hamt_init(&hamt, &int_vtabk, NULL) == CVX_FLAG_OK);

    hamt_drop(&hamt);
}

CVXSUITE(cvx_hamt_tests_init, hamt_test_init_validation, hamt_test_init_state, )
CVXSUITE(cvx_hamt_tests_clone, hamt_test_clone, )
CVXSUITE(cvx_hamt_tests_insert, hamt_test_insert_single, hamt_test_insert_collide_single,
         hamt_test_insert_duplicate, )
CVXSUITE(cvx_hamt_tests_get, hamt_test_get_empty, hamt_test_get_single_leaf,
         hamt_test_get_optional_out, )

CVXMAIN(HAMT, cvx_hamt_tests_init, cvx_hamt_tests_clone, cvx_hamt_tests_insert, cvx_hamt_tests_get)
