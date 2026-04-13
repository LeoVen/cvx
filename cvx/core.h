#ifndef CVX_CORE_H
#define CVX_CORE_H

#include <stdbool.h>
#include <stddef.h>

#include "flags.h"

#define CVX__(A, B) A##B
#define CVX_(A, B) CVX__(A, B)

#define VTABLE(X) CVX_(X, _vtable)
#define GLOBAL_VTABLE(SNAME, X, NAME) CVX_(CVX_(CVX_(cvx_vtables_, SNAME), X), NAME)

#define CVX_VTAB_COMP(name, T) int (*name)(T, T)
#define CVX_VTAB_COPY(name, T) T (*name)(T)
#define CVX_VTAB_DROP(name, T) void (*name)(T)
#define CVX_VTAB_HASH(name, T) size_t (*name)(T)
#define CVX_VTAB_PRIO(name, T) int (*name)(T, T)

#define CVX_VTAB_DEFINITION(T) \
    CVX_VTAB_COMP(comp, T); \
    CVX_VTAB_COPY(copy, T); \
    CVX_VTAB_DROP(drop, T); \
    CVX_VTAB_HASH(hash, T); \
    CVX_VTAB_PRIO(prio, T);

#ifndef CVX_ITER_TAG_MULT
// Iterator tag equals to the user-provided tag times this value
#define CVX_ITER_TAG_MULT 100
#endif

typedef struct cvx_container
{
    size_t tag;
    enum cvx_flags flag;
} cvx_container;

#define CVX_CONTAINER_GUARDS(TAG, _col_, error_value) \
    if (_col_->tag != TAG) \
    { \
        _col_->flag = CVX_FLAG_WRONG_TAG; \
        return error_value; \
    }

#endif /* CVX_CORE_H */
