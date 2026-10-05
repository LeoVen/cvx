#ifndef CVX2_EXAMPLES_TYPES_H
#define CVX2_EXAMPLES_TYPES_H

// value_type/key_type get spliced into a cvx2 template as a plain
// declarator prefix (e.g. `CVX_VALUE item`, `(CVX_VALUE){0}`), which only
// works for a type that's a single name at the use site. C's own grammar
// wraps the identifier *inside* the type for a raw array ("int[10]") or a
// raw function pointer ("int (*)(int, int)"), so neither can be used
// directly as a value_type/key_type -- typedef them first, same as you
// would to use either as a plain variable's type anywhere else in C.
//
// See the two dynamic_array instantiations in config.json that use these
// (int10_array, binop_list) -- both set "includes": ["types.h"] so the
// generated .c, which is its own translation unit and only #includes its
// own generated header, can see these typedefs too.

typedef struct
{
    int items[10];
} int10_t;

typedef int (*binop_fn)(int, int);

#endif
