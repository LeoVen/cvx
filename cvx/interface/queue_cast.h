#include "cvx/fallback.h"

// clang-format off
#ifndef INTERFACE
#error "cvx/interface/queue_cast.h requires INTERFACE to be defined (the struct name, e.g. #define INTERFACE my_queue)"
#endif
#ifndef IMPL_NEW
#error "cvx/interface/queue_cast.h requires IMPL_NEW to be defined"
#endif
#ifndef IMPL_DROP
#error "cvx/interface/queue_cast.h requires IMPL_DROP to be defined"
#endif
#ifndef IMPL_CLONE
#error "cvx/interface/queue_cast.h requires IMPL_CLONE to be defined"
#endif
#ifndef IMPL_ENQUEUE
#error "cvx/interface/queue_cast.h requires IMPL_ENQUEUE to be defined"
#endif
#ifndef IMPL_DEQUEUE
#error "cvx/interface/queue_cast.h requires IMPL_DEQUEUE to be defined"
#endif
#ifndef IMPL_COUNT
#error "cvx/interface/queue_cast.h requires IMPL_COUNT to be defined"
#endif
// clang-format on

#include "cvx/core.h"

struct VTABLE(INTERFACE) GLOBAL_VTABLE(SNAME, _as_, INTERFACE) = {
    .new = IMPL_NEW,
    .drop = IMPL_DROP,
    .clone = IMPL_CLONE,
    .enqueue = IMPL_ENQUEUE,
    .dequeue = IMPL_DEQUEUE,
    .count = IMPL_COUNT,
};

struct INTERFACE FUNC(CVX_(_as_, INTERFACE))(cvx_container *instance)
{
    return (struct INTERFACE){
        .vtable = &GLOBAL_VTABLE(SNAME, _as_, INTERFACE),
        .instance = instance,
    };
}

#undef INTERFACE
