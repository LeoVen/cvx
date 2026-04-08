// clang-format off
#ifndef INTERFACE
#error "cvx/iter/forward_iterator_cast.h requires INTERFACE to be defined (the struct name, e.g. #define INTERFACE my_iter)"
#endif
#ifndef IMPL_START
#error "cvx/iter/forward_iterator_cast.h requires IMPL_START to be defined"
#endif
#ifndef IMPL_DROP
#error "cvx/iter/forward_iterator_cast.h requires IMPL_DROP to be defined"
#endif
#ifndef IMPL_AT_START
#error "cvx/iter/forward_iterator_cast.h requires IMPL_AT_START to be defined"
#endif
#ifndef IMPL_AT_END
#error "cvx/iter/forward_iterator_cast.h requires IMPL_AT_END to be defined"
#endif
#ifndef IMPL_COUNT
#error "cvx/iter/forward_iterator_cast.h requires IMPL_COUNT to be defined"
#endif
#ifndef IMPL_TO_START
#error "cvx/iter/forward_iterator_cast.h requires IMPL_TO_START to be defined"
#endif
#ifndef IMPL_NEXT
#error "cvx/iter/forward_iterator_cast.h requires IMPL_NEXT to be defined"
#endif
#ifndef IMPL_FORWARD
#error "cvx/iter/forward_iterator_cast.h requires IMPL_FORWARD to be defined"
#endif
#ifndef IMPL_VALUE
#error "cvx/iter/forward_iterator_cast.h requires IMPL_VALUE to be defined"
#endif
#ifndef IMPL_INDEX
#error "cvx/iter/forward_iterator_cast.h requires IMPL_INDEX to be defined"
#endif
// clang-format on

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

struct INTERFACE FUNC(CVX_(_iter_as_, INTERFACE))(cvx_container *instance)
{
    return (struct INTERFACE){
        .vtable = &GLOBAL_VTABLE(SNAME, _iter_as_, INTERFACE),
        .instance = instance,
    };
}

#undef INTERFACE
