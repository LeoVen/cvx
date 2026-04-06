#define VTABLE CVX_(IMPL_FORWARD_ITER, _vtable)
#define FUNC(X) CVX_(PFX, X)

#define GLOBAL_VTABLE(SNAME, IMPL_FORWARD_ITER) \
    CVX_(CVX_(CVX_(cvx_vtables_, SNAME), _iter_as_), IMPL_FORWARD_ITER)

struct VTABLE GLOBAL_VTABLE(SNAME, IMPL_FORWARD_ITER) = {
    .start = IMPL_START,
    .drop = IMPL_DROP,
    .at_start = IMPL_AT_START,
    .at_end = IMPL_AT_END,
    .count = IMPL_COUNT,
    .to_start = IMPL_TO_START,
    .next = IMPL_NEXT,
    .forward = IMPL_FORWARD,
    .value = IMPL_VALUE,
    .index = IMPL_INDEX,
};

struct IMPL_FORWARD_ITER FUNC(CVX_(_iter_as_,
                                         IMPL_FORWARD_ITER))(cvx_container *instance)
{
    return (struct IMPL_FORWARD_ITER){
        .vtable = &GLOBAL_VTABLE(SNAME, IMPL_FORWARD_ITER),
        .instance = instance,
    };
}

#undef IMPL_FORWARD_ITER
#include "cvx/undef.h"
