// Defines placeholder macros only when editing a cvx2 template directly
// (CVX_ENABLE_FALLBACK), so clangd/gcc can type-check the template as
// standalone C even though it's never included this way by real consumers
// -- the generator text-substitutes these placeholders instead.
//
// Mirrors cvx/fallback.h's role, but for cvx2's renamed placeholders
// (CVX_VALUE/CVX_KEY/CVX_SNAME/CVX_PFX/CVX_TAG instead of V/K/SNAME/PFX/TAG)
// and for cvx2's variant-axis default selection.
#ifndef CVX2_FALLBACK_H
#define CVX2_FALLBACK_H

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
#ifndef CVX_TAG
#define CVX_TAG 99
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

#endif // CVX2_FALLBACK_H
