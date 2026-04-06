#include <stdio.h>

// 1. Define the interface type
#define V         int             // data type for the stack interface
#define INTERFACE stack           // name of the struct
#include "cvx/interface/stack.h"

// 2. Instantiate an implementation
#define V          int            // data type for my dynamic array implementation
#define SNAME      my_list        // name of my dynamic array (e.g. struct my_list)
#define PFX        ml             // prefix to all functions
#define TAG        1              // unique tag for this implementation
#define IMPL_STACK stack          // generates the ml_as_stack() "cast"
#include "cvx/dynamic_array.h"

#include "cvx/interface_macros.h" // utility macros when working with interfaces

int main(void)
{
    struct stack s = ml_as_stack(ml_new_with(8));

    cvx_push(&s, 10);
    cvx_push(&s, 20);
    cvx_push(&s, 30);

    int out = cvx_pop(&s);
    printf("Pop: %d\nPeek: %d\n", out, cvx_peek(&s));

    cvx_drop(&s);
}