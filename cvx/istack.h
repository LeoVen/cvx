#include "container.h"
#include "core.h"
#include <stddef.h>

#define VTABLE CVX_(INTERFACE, _vtable)

struct VTABLE
{
    cvx_container *(*new)(void);
    cvx_container *(*clone)(cvx_container *);
    void (*drop)(cvx_container *);

    void (*push)(cvx_container *, V);
    void (*pop)(cvx_container *, V *);
    V (*peek)(cvx_container *);
    void (*replace)(cvx_container *, V, V *);

    size_t (*count)(cvx_container *);
};

struct INTERFACE
{
    cvx_container *instance;
    struct VTABLE *vtable;
};

#include "undef.h"
