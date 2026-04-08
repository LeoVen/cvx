// clang-format off
#ifndef V
#error "cvx/interface/deque.h requires V to be defined (the element type, e.g. #define V int)"
#endif
#ifndef INTERFACE
#error "cvx/interface/deque.h requires INTERFACE to be defined (the struct name, e.g. #define INTERFACE my_deque)"
#endif
// clang-format on

#include "cvx/core.h"

struct VTABLE(INTERFACE)
{
    // constructors and destructors
    cvx_container *(*new)(void);
    cvx_container *(*clone)(cvx_container *);
    void (*drop)(cvx_container *);
    // operations
    void (*push_front)(cvx_container *, V);
    void (*push_back)(cvx_container *, V);
    V (*pop_front)(cvx_container *);
    V (*pop_back)(cvx_container *);
    V (*peek_front)(cvx_container *);
    V (*peek_back)(cvx_container *);
    // state
    size_t (*count)(cvx_container *);
};

struct INTERFACE
{
    cvx_container *instance;
    struct VTABLE(INTERFACE) * vtable;
};

#include "cvx/undef.h"
