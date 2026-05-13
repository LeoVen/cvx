#include "cvx/fallback.h"

// clang-format off
#ifndef INTERFACE
#error "cvx/interface/stack_cast.h requires INTERFACE to be defined (the struct name, e.g. #define INTERFACE my_stack)"
#endif
#ifndef IMPL_DROP
#error "cvx/interface/stack_cast.h requires IMPL_DROP to be defined"
#endif
#ifndef IMPL_CLONE
#error "cvx/interface/stack_cast.h requires IMPL_CLONE to be defined"
#endif
#ifndef IMPL_PUSH
#error "cvx/interface/stack_cast.h requires IMPL_PUSH to be defined"
#endif
#ifndef IMPL_POP
#error "cvx/interface/stack_cast.h requires IMPL_POP to be defined"
#endif
#ifndef IMPL_COUNT
#error "cvx/interface/stack_cast.h requires IMPL_COUNT to be defined"
#endif
#ifndef IMPL_PEEK
#error "cvx/interface/stack_cast.h requires IMPL_PEEK to be defined"
#endif
#ifndef IMPL_REPLACE
#error "cvx/interface/stack_cast.h requires IMPL_REPLACE to be defined"
#endif
// clang-format on

#include "cvx/core.h"

struct VTABLE(INTERFACE) GLOBAL_VTABLE(SNAME, _as_, INTERFACE) = {
    .drop = IMPL_DROP,
    .clone = IMPL_CLONE,
    .push = IMPL_PUSH,
    .pop = IMPL_POP,
    .count = IMPL_COUNT,
    .peek = IMPL_PEEK,
    .replace = IMPL_REPLACE,
};

struct INTERFACE FUNC(CVX_(_as_, INTERFACE))(cvx_container *instance)
{
    return (struct INTERFACE){
        .vtable = &GLOBAL_VTABLE(SNAME, _as_, INTERFACE),
        .instance = instance,
    };
}

#undef INTERFACE
