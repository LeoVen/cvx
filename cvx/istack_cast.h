#define VTABLE CVX_(IMPL_STACK, _vtable)
#define FUNC(X) CVX_(PFX, X)

#define GLOBAL_VTABLE(SNAME, IMPL_STACK) \
    CVX_(CVX_(CVX_(cvx_vtables_, SNAME), _as_), IMPL_STACK)

struct VTABLE GLOBAL_VTABLE(SNAME, IMPL_STACK) = {
    .new = IMPL_NEW,
    .drop = IMPL_DROP,
    .clone = IMPL_CLONE,
    .push = IMPL_PUSH,
    .pop = IMPL_POP,
    .count = IMPL_COUNT,
    .peek = IMPL_PEEK,
    .replace = IMPL_REPLACE,
};

struct IMPL_STACK FUNC(CVX_(_as_, IMPL_STACK))(cvx_container *instance)
{
    return (struct IMPL_STACK){
        .vtable = &GLOBAL_VTABLE(SNAME, IMPL_STACK),
        .instance = instance,
    };
}

#undef IMPL_STACK
#include "cvx/undef.h"
