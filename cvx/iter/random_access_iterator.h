#include "cvx/core.h"

#define VTABLE CVX_(INTERFACE, _vtable)

struct VTABLE
{
    // constructors and destructors
    cvx_container *(*start)(cvx_container *);
    cvx_container *(*end)(cvx_container *);
    void (*drop)(cvx_container *);
    // state
    bool (*at_start)(cvx_container *);
    bool (*at_end)(cvx_container *);
    size_t (*count)(cvx_container *);
    // movement
    void (*to_start)(cvx_container *);
    void (*to_end)(cvx_container *);
    void (*next)(cvx_container *);
    void (*prev)(cvx_container *);
    void (*forward)(cvx_container *, size_t steps);
    void (*backward)(cvx_container *, size_t steps);
    void (*go_to)(cvx_container *, size_t index);
    // access
    V (*value)(cvx_container *);
    size_t (*index)(cvx_container *);
};

struct INTERFACE
{
    cvx_container *instance;
    struct VTABLE *vtable;
};

#include "cvx/undef.h"
