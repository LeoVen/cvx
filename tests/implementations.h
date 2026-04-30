#ifndef CVX_IMPLEMENTATIONS_TESTS_H
#define CVX_IMPLEMENTATIONS_TESTS_H

#include "tests/vtabs.h"

///
/// INTERFACES
///

#define V int
#define INTERFACE bidirectional_iter
#include "cvx/iter/bidirectional_iterator.h"

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

#define V int
#define INTERFACE deque_int
#include "cvx/interface/deque.h"

///
/// IMPLEMENTATIONS
///

#define V int
#define SNAME bheap_int
#define PFX bh_int
#define TAG 33
#include "cvx/binary_heap.h"
#define BH_ITER_TAG (33 * CVX_ITER_TAG_MULT)
static struct bheap_int_vtabv *bh_int_vtabv_comp_only = &(struct bheap_int_vtabv){
    .comp = int_comp,
};
static struct bheap_int_vtabv *bh_int_vtabv_full = &(struct bheap_int_vtabv){
    .comp = int_comp,
    .copy = int_copy,
    .drop = int_drop,
};

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

#define V int
#define SNAME dlinked_int
#define PFX dll_int
#define TAG 88
// #define IMPL_FORWARD_ITER forward_iter
// #define IMPL_STACK stack_int
// #define IMPL_QUEUE queue_int
#define IMPL_DEQUE deque_int
#define IMPL_BIDIRECTIONAL_ITER bidirectional_iter
#include "cvx/dlinked_list.h"
#define DLL_ITER_TAG (88 * CVX_ITER_TAG_MULT)
static struct dlinked_int_vtabv *dll_int_vtabv_full = &(struct dlinked_int_vtabv){
    .copy = int_copy,
    .drop = int_drop,
};

// Interval-set iterator interface: V = the entry struct (interval), not int.
#define V struct iset_int_entry
#define INTERFACE iset_int_bidi_iter
#include "cvx/iter/bidirectional_iterator.h"
#undef V

#define V int
#define SNAME iset_int
#define PFX is_int
#define TAG 55
#define IMPL_BIDIRECTIONAL_ITER iset_int_bidi_iter
#include "cvx/interval_set.h"
#define IS_ITER_TAG (55 * CVX_ITER_TAG_MULT)
static struct iset_int_vtabv *is_int_vtabv_comp_only = &(struct iset_int_vtabv){
    .comp = int_comp,
};
static struct iset_int_vtabv *is_int_vtabv_full = &(struct iset_int_vtabv){
    .comp = int_comp,
    .copy = int_copy,
    .drop = int_drop,
};

// Interval-map iterator interface: V = the entry struct.
#define V struct imap_int_int_entry
#define INTERFACE imap_int_int_bidi_iter
#include "cvx/iter/bidirectional_iterator.h"
#undef V

#define K int
#define V int
#define SNAME imap_int_int
#define PFX im_int
#define TAG 44
#define IMPL_BIDIRECTIONAL_ITER imap_int_int_bidi_iter
#include "cvx/interval_map.h"
#define IM_ITER_TAG (44 * CVX_ITER_TAG_MULT)
static struct imap_int_int_vtabk *im_int_vtabk = &(struct imap_int_int_vtabk){
    .comp = int_comp,
};
static struct imap_int_int_vtabk *im_int_vtabk_full = &(struct imap_int_int_vtabk){
    .comp = int_comp,
    .copy = int_copy,
    .drop = int_drop,
};
static struct imap_int_int_vtabv *im_int_vtabv_with_comp = &(struct imap_int_int_vtabv){
    .comp = int_comp,
};
static struct imap_int_int_vtabv *im_int_vtabv_full = &(struct imap_int_int_vtabv){
    .comp = int_comp,
    .copy = int_copy,
    .drop = int_drop,
};

#define K int
#define V int
#define SNAME ht_int_int
#define PFX ht_int
#define TAG 22
#include "cvx/hashtable.h"
#define HT_ITER_TAG (22 * CVX_ITER_TAG_MULT)
static struct ht_int_int_vtabk *ht_int_vtabk = &(struct ht_int_int_vtabk){
    .comp = int_comp,
    .hash = int_hash,
};
static struct ht_int_int_vtabk *ht_int_vtabk_full = &(struct ht_int_int_vtabk){
    .comp = int_comp,
    .hash = int_hash,
    .copy = int_copy,
    .drop = int_drop,
};
static struct ht_int_int_vtabv *ht_int_vtabv_full = &(struct ht_int_int_vtabv){
    .copy = int_copy,
    .drop = int_drop,
};

#endif /* CVX_IMPLEMENTATIONS_TEST_H */
