#ifndef CVX_CORE_H
#define CVX_CORE_H

#include "flags.h"
#include <stddef.h>

// Token-paste helpers used by generated code and by templates' own local
// macros (FUNC(X), VTAB_V, etc.) before the generator resolves them away.
#define CVX__(A, B) A##B
#define CVX_(A, B) CVX__(A, B)

#define CVX_VTAB_COMP(name, T) int (*name)(T, T)
#define CVX_VTAB_COPY(name, T) enum cvx_flags (*name)(T, T *)
#define CVX_VTAB_DROP(name, T) void (*name)(T)
#define CVX_VTAB_HASH(name, T) size_t (*name)(T)
#define CVX_VTAB_PRIO(name, T) int (*name)(T, T)

#define CVX_VTAB_DEFINITION(T) \
    CVX_VTAB_COMP(comp, T); \
    CVX_VTAB_COPY(copy, T); \
    CVX_VTAB_DROP(drop, T); \
    CVX_VTAB_HASH(hash, T); \
    CVX_VTAB_PRIO(prio, T);

#ifndef CVX_BUFFER_GROWTH_RATE
#define CVX_BUFFER_GROWTH_RATE 1.5
#endif
#ifndef CVX_BUFFER_MIN_SIZE
#define CVX_BUFFER_MIN_SIZE 8
#endif
#if CVX_BUFFER_MIN_SIZE < 2
#error "CVX_BUFFER_MIN_SIZE must be greater than 1"
#endif

#endif /* CVX_CORE_H */
