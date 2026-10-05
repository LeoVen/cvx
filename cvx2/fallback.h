// This file defines a bunch of macros only when I'm using clangd LSP.
// This is enabled by defining CVX_ENABLE_FALLBACK.
//
// Since I'm writing templating headers, clangd doesn't understand where macros
// like V and SNAME are defined. Basically, the header files aren't stand-alone
// and I need to include this file at the top of every templating header file
// so that I get decent LSP features.
//
// It is not ideal, but I think it works for now.
#ifndef CVX_FALLBACK_H
#define CVX_FALLBACK_H

#ifdef CVX_ENABLE_FALLBACK

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Template placeholder macros (renamed from cvx/'s V/K/SNAME/PFX/TAG so they
// can't collide with identifiers that legitimately appear inside a user's
// own value/key types -- see cvx2 plan, "Template conventions").
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

// Variant axes: one default #define per axis/name pair so a template with
// @cvx2:variant blocks still compiles to exactly one coherent
// implementation when opened directly. The generator selects blocks by
// parsing the marker comments, not by evaluating these -- they exist purely
// for standalone compilation of the template itself.
#ifndef CVX2_COLLISION_OPEN_ADDRESSING
#ifndef CVX2_COLLISION_SEPARATE_CHAINING
#define CVX2_COLLISION_OPEN_ADDRESSING
#endif
#endif

#endif // CVX_ENABLE_FALLBACK

#endif // CVX_FALLBACK_H
