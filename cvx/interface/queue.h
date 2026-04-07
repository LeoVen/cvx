#ifndef V
#error "cvx/interface/queue.h requires V to be defined (the element type, e.g. #define V int)"
#endif
#ifndef INTERFACE
#error "cvx/interface/queue.h requires INTERFACE to be defined (the struct name, e.g. #define INTERFACE my_queue)"
#endif

#include "cvx/core.h"

struct VTABLE(INTERFACE)
{
    // constructors and destructors
    cvx_container *(*new)(void);
    cvx_container *(*clone)(cvx_container *);
    void (*drop)(cvx_container *);
    // operations
    void (*enqueue)(cvx_container *, V);
    V (*dequeue)(cvx_container *);
    // state
    size_t (*count)(cvx_container *);
};

struct INTERFACE
{
    cvx_container *instance;
    struct VTABLE(INTERFACE) * vtable;
};

#include "cvx/undef.h"
