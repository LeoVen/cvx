#ifndef CVX_IMPLEMENTATIONS_TEST_H
#define CVX_IMPLEMENTATIONS_TEST_H

#include "tests/vtabs.h"

///
/// INTERFACES
///

#define V int
#define INTERFACE stack_int
#include "cvx/interface/stack.h"

#define V int
#define INTERFACE raccess_iter
#include "cvx/iter/random_access_iterator.h"

#define V int
#define INTERFACE forward_iter
#include "cvx/iter/forward_iterator.h"

#define V int
#define INTERFACE queue_int
#include "cvx/interface/queue.h"

///
/// IMPLEMENTATIONS
///

#define V int
#define SNAME dynamic_array_int
#define PFX da_int
#define TAG 99
#define IMPL_RANDOM_ACCESS_ITER raccess_iter
#define IMPL_STACK stack_int
#include "cvx/dynamic_array.h"
#define DA_ITER_TAG (99 * CVX_ITER_TAG_MULT)
static struct dynamic_array_int_vtabv *da_int_vtabv_full = &(struct dynamic_array_int_vtabv){
    .copy = int_copy,
    .drop = int_drop,
};

#define V int
#define SNAME slinked_int
#define PFX sll_int
#define TAG 77
#define IMPL_FORWARD_ITER forward_iter
#define IMPL_STACK stack_int
#define IMPL_QUEUE queue_int
#include "cvx/slinked_list.h"
#define SLL_ITER_TAG (77 * CVX_ITER_TAG_MULT)
static struct slinked_int_vtabv *sll_int_vtabv_full = &(struct slinked_int_vtabv){
    .copy = int_copy,
    .drop = int_drop,
};

#endif /* CVX_IMPLEMENTATIONS_TEST_H */
