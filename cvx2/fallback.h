// This file defines a bunch of macros only when I'm using clangd LSP.
// This is enabled by defining CVX_ENABLE_FALLBACK.
//
// Since I'm writing templating files, clangd doesn't understand where macros
// like CVX_VAL and CVX_SNAME are defined. I need to include this file at the
// top of every template file so that I get decent LSP features.
//
// It is not ideal, but I think it works for now.
#ifndef CVX_FALLBACK_H
#define CVX_FALLBACK_H

#ifdef CVX_ENABLE_FALLBACK

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef CVX_KEY
#define CVX_KEY int
#endif
#ifndef CVX_VAL
#define CVX_VAL int
#endif
#ifndef CVX_SNAME
#define CVX_SNAME cvx2_fallback
#endif
#ifndef CVX_PFX
#define CVX_PFX cvx2_fb
#endif

// Dynamic Array
#define CVX_BUFFER_MIN_SIZE 8
#define CVX_BUFFER_GROWTH_RATE 1.2

// Hashtable
#define CVX2_KIND_MAP
#ifndef CVX2_COLLISION_OPEN_ADDRESSING
#ifndef CVX2_COLLISION_SEPARATE_CHAINING
#define CVX2_COLLISION_OPEN_ADDRESSING
#endif
#endif

// Linked List
#ifndef CVX2_NODE_SINGLE
#ifndef CVX2_NODE_DOUBLE
#define CVX2_NODE_DOUBLE
#endif
#endif

#endif // CVX_ENABLE_FALLBACK

#endif // CVX_FALLBACK_H
