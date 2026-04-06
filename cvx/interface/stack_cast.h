#include "cvx/core.h"

struct VTABLE(INTERFACE) GLOBAL_VTABLE(SNAME, _as_, INTERFACE) = {
    .new = IMPL_NEW,
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
