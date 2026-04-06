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
    struct VTABLE(INTERFACE) *vtable;
};

#include "cvx/undef.h"
