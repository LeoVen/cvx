#include "cvx/fallback.h"

// clang-format off
#ifndef V
#error "cvx/interface/stack.h requires V to be defined (the element type, e.g. #define V int)"
#endif
#ifndef INTERFACE
#error "cvx/interface/stack.h requires INTERFACE to be defined (the struct name, e.g. #define INTERFACE my_stack)"
#endif
// clang-format on

#include "cvx/core.h"

struct VTABLE(INTERFACE)
{
    // constructors and destructors
    void (*clone)(cvx_container *, cvx_container *);
    void (*drop)(cvx_container *);
    // operations
    void (*push)(cvx_container *, V);
    V (*pop)(cvx_container *);
    V (*peek)(cvx_container *);
    V (*replace)(cvx_container *, V);
    // state
    size_t (*count)(cvx_container *);
};

struct INTERFACE
{
    cvx_container *instance;
    struct VTABLE(INTERFACE) * vtable;
};

#undef INTERFACE
