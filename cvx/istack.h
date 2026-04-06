#include "core.h"

#define VTABLE CVX_(INTERFACE, _vtable)

struct VTABLE
{
    // constructors and destructors
    cvx_container *(*new)(void);
    cvx_container *(*clone)(cvx_container *);
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
    struct VTABLE *vtable;
};

#include "cvx/undef.h"
