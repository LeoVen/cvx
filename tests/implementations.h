#ifndef CVX_IMPLEMENTATIONS_TEST_H
#define CVX_IMPLEMENTATIONS_TEST_H

///
/// INTERFACES
///

#define V int
#define INTERFACE stack_int
#include "cvx/istack.h"

#define V int
#define INTERFACE raccess_iter
#include "cvx/iter/random_access_iterator.h"

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

#define V int
#define SNAME slinked_int
#define PFX sll_int
#define TAG 77
#define IMPL_STACK stack_int
#include "cvx/slinked_list.h"
#define SLL_ITER_TAG (77 * CVX_ITER_TAG_MULT)

#endif /* CVX_IMPLEMENTATIONS_TEST_H */
