#include "cvx/core.h"

struct VTABLE(INTERFACE) GLOBAL_VTABLE(SNAME, _iter_as_, INTERFACE) = {
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

struct INTERFACE FUNC(CVX_(_iter_as_,
                                         INTERFACE))(cvx_container *instance)
{
    return (struct INTERFACE){
        .vtable = &GLOBAL_VTABLE(SNAME, _iter_as_, INTERFACE),
        .instance = instance,
    };
}

#undef INTERFACE
