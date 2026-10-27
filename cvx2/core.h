#ifndef CVX_CORE_H
#define CVX_CORE_H

#include <stddef.h>
#include <stdint.h>

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
#include <intrin.h>
#endif

#define CVX__(A, B) A##B
#define CVX_(A, B) CVX__(A, B)

#define CVX_VTAB_COMP(name, T) int (*name)(T const *, T const *)
#define CVX_VTAB_COPY(name, T) enum cvx_flags (*name)(T const *, T *)
#define CVX_VTAB_DROP(name, T) void (*name)(T *)
#define CVX_VTAB_HASH(name, T) size_t (*name)(T const *)
#define CVX_VTAB_BOOL(name, T) bool (*name)(T const *)
#define CVX_VTAB_COMBINE(name, T) T (*name)(T const *, T const *)

// TODO: move to integers so I can check this at preprocessor time
#ifndef CVX_BUFFER_GROWTH_RATE
#define CVX_BUFFER_GROWTH_RATE 1.5
#endif
#ifndef CVX_BUFFER_MIN_SIZE
#define CVX_BUFFER_MIN_SIZE 8
#endif
#if CVX_BUFFER_MIN_SIZE < 2
#error "CVX_BUFFER_MIN_SIZE must be greater than 1"
#endif

// Number of set bits in x. Lives here rather than in a template because the
// generator preprocesses templates (which would pick the generator's compiler
// branch), while this header stays a real #include resolved by the user's
// compiler.
static inline int cvx_popcount(uint64_t x)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(x);
#elif defined(_MSC_VER) && defined(_M_X64)
    return (int)__popcnt64(x);
#elif defined(_MSC_VER) && defined(_M_IX86)
    return (int)(__popcnt((unsigned int)x) + __popcnt((unsigned int)(x >> 32)));
#else
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    return (int)((x * 0x0101010101010101ULL) >> 56);
#endif
}

enum cvx_hamt_node_t
{
    CVX_HAMT_NODE_LEAF,
    CVX_HAMT_NODE_INTERNAL,
    CVX_HAMT_NODE_COLLISION,
};

#endif /* CVX_CORE_H */
