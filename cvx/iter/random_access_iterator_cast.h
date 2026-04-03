#define VTABLE CVX_(IMPL_RANDOM_ACCESS_ITER, _vtable)
#define FUNC(X) CVX_(PFX, X)

#define GLOBAL_VTABLE(SNAME, IMPL_RANDOM_ACCESS_ITER) \
    CVX_(CVX_(CVX_(cvx_vtables_, SNAME), _iter_as_), IMPL_RANDOM_ACCESS_ITER)

struct VTABLE GLOBAL_VTABLE(SNAME, IMPL_RANDOM_ACCESS_ITER) = {
    .start = IMPL_START,
    .end = IMPL_END,
    .drop = IMPL_DROP,
    .at_start = IMPL_AT_START,
    .at_end = IMPL_AT_END,
    .count = IMPL_COUNT,
    .to_start = IMPL_TO_START,
    .to_end = IMPL_TO_END,
    .next = IMPL_NEXT,
    .prev = IMPL_PREV,
    .forward = IMPL_FORWARD,
    .backward = IMPL_BACKWARD,
    .go_to = IMPL_GO_TO,
    .value = IMPL_VALUE,
    .index = IMPL_INDEX,
};

struct IMPL_RANDOM_ACCESS_ITER FUNC(CVX_(_iter_as_,
                                         IMPL_RANDOM_ACCESS_ITER))(cvx_container *instance)
{
    return (struct IMPL_RANDOM_ACCESS_ITER){
        .vtable = &GLOBAL_VTABLE(SNAME, IMPL_RANDOM_ACCESS_ITER),
        .instance = instance,
    };
}

#undef IMPL_RANDOM_ACCESS_ITER
#include "cvx/undef.h"
