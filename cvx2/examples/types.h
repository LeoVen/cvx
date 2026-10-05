#ifndef CVX2_EXAMPLES_TYPES_H
#define CVX2_EXAMPLES_TYPES_H

// Array and function-pointer types need a typedef before they can be used
// as a value_type/key_type -- C can't express them as a plain prefix type.

typedef struct
{
    int items[10];
} int10_t;

typedef int (*binop_fn)(int, int);

#endif
