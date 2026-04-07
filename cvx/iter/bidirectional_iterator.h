#ifndef V
#error "cvx/iter/bidirectional_iterator.h requires V to be defined (the element type, e.g. #define V int)"
#endif
#ifndef INTERFACE
#error "cvx/iter/bidirectional_iterator.h requires INTERFACE to be defined (the struct name, e.g. #define INTERFACE my_iter)"
#endif

#include "cvx/core.h"

struct VTABLE(INTERFACE)
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
    // access
    V (*value)(cvx_container *);
    size_t (*index)(cvx_container *);
};

struct INTERFACE
{
    cvx_container *instance;
    struct VTABLE(INTERFACE) *vtable;
};

#undef INTERFACE
