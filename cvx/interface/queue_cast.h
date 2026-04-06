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

