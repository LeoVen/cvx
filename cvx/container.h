#ifndef CVX_CONTAINER_H
#define CVX_CONTAINER_H

#include <stddef.h>

#include "flags.h"

typedef struct cvx_container
{
    size_t tag;
    enum cvx_flags flag;
} cvx_container;

#endif /* CVX_CONTAINER_H */
