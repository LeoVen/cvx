#include "cvx/fallback.h"

// clang-format off
#ifndef INTERFACE
#error "cvx/interface/deque_cast.h requires INTERFACE to be defined (the struct name, e.g. #define INTERFACE my_deque)"
#endif
#ifndef IMPL_NEW
#error "cvx/interface/deque_cast.h requires IMPL_NEW to be defined"
#endif
#ifndef IMPL_CLONE
#error "cvx/interface/deque_cast.h requires IMPL_CLONE to be defined"
#endif
#ifndef IMPL_DROP
#error "cvx/interface/deque_cast.h requires IMPL_DROP to be defined"
#endif
#ifndef IMPL_PUSH_FRONT
#error "cvx/interface/deque_cast.h requires IMPL_PUSH_FRONT to be defined"
#endif
#ifndef IMPL_PUSH_BACK
#error "cvx/interface/deque_cast.h requires IMPL_PUSH_BACK to be defined"
#endif
#ifndef IMPL_POP_FRONT
#error "cvx/interface/deque_cast.h requires IMPL_POP_FRONT to be defined"
#endif
#ifndef IMPL_POP_BACK
#error "cvx/interface/deque_cast.h requires IMPL_POP_BACK to be defined"
#endif
#ifndef IMPL_PEEK_FRONT
#error "cvx/interface/deque_cast.h requires IMPL_PEEK_FRONT to be defined"
#endif
#ifndef IMPL_PEEK_BACK
#error "cvx/interface/deque_cast.h requires IMPL_PEEK_BACK to be defined"
#endif
#ifndef IMPL_COUNT
#error "cvx/interface/deque_cast.h requires IMPL_COUNT to be defined"
#endif
// clang-format on

#include "cvx/core.h"

struct VTABLE(INTERFACE) GLOBAL_VTABLE(SNAME, _as_, INTERFACE) = {
    .new = IMPL_NEW,
    .clone = IMPL_CLONE,
    .drop = IMPL_DROP,
    .push_front = IMPL_PUSH_FRONT,
    .push_back = IMPL_PUSH_BACK,
    .pop_front = IMPL_POP_FRONT,
    .pop_back = IMPL_POP_BACK,
    .peek_front = IMPL_PEEK_FRONT,
    .peek_back = IMPL_PEEK_BACK,
    .count = IMPL_COUNT,
};

struct INTERFACE FUNC(CVX_(_as_, INTERFACE))(cvx_container *_instance_)
{
    return (struct INTERFACE){
        .vtable = &GLOBAL_VTABLE(SNAME, _as_, INTERFACE),
        .instance = _instance_,
    };
}

#undef INTERFACE
