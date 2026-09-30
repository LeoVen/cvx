/// segment_tree.h
///
/// Status
///
///   [x] concept
///   [ ] v1
///   [ ] tests
///   [ ] refine
///   [ ] stabilize
///
/// Segment tree with closed intervals [start, end]
///

#include "cvx/fallback.h"
#include "cvx/flags.h"

// clang-format off
#ifndef K
#error "cvx/interval_map.h requires K to be defined (the domain/key type, e.g. #define K int)"
#endif
#ifndef V
#error "cvx/interval_map.h requires V to be defined (the value/codomain type, e.g. #define V int)"
#endif
#ifndef SNAME
#error "cvx/interval_map.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_imap)"
#endif
#ifndef PFX
#error "cvx/interval_map.h requires PFX to be defined (the function prefix, e.g. #define PFX mi)"
#endif
#ifndef TAG
#error "cvx/interval_map.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
#endif
// clang-format on

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cvx/core.h"

#define FUNC(X) CVX_(PFX, X)

struct SNAME
{
    cvx_container super;
    V *tree;
    K leaves;
    V (*combine)(V left, V right);
    V identity;
};

// ---- Initializers ----
void FUNC(_init)(struct SNAME *self, K count, V identity, V (*combine)(V left, V right));
void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone);

// ---- Destructor ----
void FUNC(_drop)(struct SNAME *self);

// ---- Operations ----
void FUNC(_build)(struct SNAME *self, const V *array);
V FUNC(_query)(struct SNAME *self, K left, K right);

// ---- private functions ----
void FUNC(__build)(struct SNAME *self, const V *array, K node, K start, K end);
V FUNC(__query)(struct SNAME *self, K node, K start, K end, K left, K right);

void FUNC(_init)(struct SNAME *self, K count, V identity, V (*combine)(V left, V right))
{
    if (!combine)
    {
        self->super.flag = CVX_FLAG_VTAB;
        return;
    }

    self->leaves = count;
    self->combine = combine;
    self->identity = identity;
    self->tree = malloc(4 * count * sizeof(V));

    if (!self->tree)
    {
        self->super.flag = CVX_FLAG_ALLOC;
        return;
    }

    self->super.flag = CVX_FLAG_OK;
}

void FUNC(_clone)(struct SNAME *self, struct SNAME *other)
{
    FUNC(_init)(other, self->leaves, self->identity, self->combine);

    if (other->super.flag != CVX_FLAG_OK)
    {
        return;
    }

    size_t size = 4 * self->leaves * sizeof(V);
    memcpy(other->tree, self->tree, size);
}

void FUNC(_drop)(struct SNAME *self)
{
    if (!self)
    {
        return;
    }

    free(self->tree);

    *self = (struct SNAME){
        .super = {
            .flag = CVX_FLAG_OK,
            .tag = self->super.tag,
        },
    };
}

void FUNC(_build)(struct SNAME *self, const V *array)
{
    if (self->leaves > 0)
    {
        FUNC(__build)(self, array, 1, 0, self->leaves - 1);
    }
}

V FUNC(_query)(struct SNAME *self, K left, K right)
{
    if (self->leaves == 0)
    {
        return self->identity;
    }

    return FUNC(__query)(self, 1, 0, self->leaves - 1, left, right);
}

void FUNC(_traverse_leaves)(struct SNAME *self, K node, K start, K end,
                            void (*leaf_action)(K index, V value))
{
    // Base case: We are at a leaf
    if (start == end)
    {
        leaf_action(start, self->tree[node]);
        return;
    }

    // Internal node: traverse left and right children
    K mid = start + (end - start) / 2;
    FUNC(_traverse_leaves)(self, 2 * node, start, mid, leaf_action);
    FUNC(_traverse_leaves)(self, 2 * node + 1, mid + 1, end, leaf_action);
}

// Clean wrapper function so the user doesn't have to pass 'node', 'start', and 'end' manually
void FUNC(_for_each)(struct SNAME *self, void (*leaf_action)(K index, V value))
{
    if (self->leaves > 0)
    {
        FUNC(_traverse_leaves)(self, 1, 0, self->leaves - 1, leaf_action);
    }
}

//
//
// Private Functions
//
//

void FUNC(__build)(struct SNAME *self, const V *array, K node, K start, K end)
{
    if (start == end)
    {
        self->tree[node] = array[start];
        return;
    }

    K mid = start + (end - start) / 2;
    FUNC(__build)(self, array, 2 * node, start, mid);
    FUNC(__build)(self, array, 2 * node + 1, mid + 1, end);

    self->tree[node] = self->combine(self->tree[2 * node], self->tree[2 * node + 1]);
}

V FUNC(__query)(struct SNAME *self, K node, K start, K end, K left, K right)
{
    if (right < start || left > end)
    {
        return self->identity;
    }

    if (left <= start && end <= right)
    {
        return self->tree[node];
    }

    K mid = start + (end - start) / 2;
    V left_res = FUNC(__query)(self, 2 * node, start, mid, left, right);
    V right_res = FUNC(__query)(self, 2 * node + 1, mid + 1, end, left, right);

    return self->combine(left_res, right_res);
}

#include "cvx/undef.h"
