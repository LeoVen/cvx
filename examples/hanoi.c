#include <stdio.h>

#define V int
#define INTERFACE stack
#include "cvx/interface/stack.h"

#define V int
#define SNAME list
#define PFX l
#define TAG 1
#define IMPL_STACK stack
#include "cvx/dynamic_array.h"

#define V int
#define SNAME llist
#define PFX ll
#define TAG 2
#define IMPL_STACK stack
#include "cvx/slinked_list.h"

#include "cvx/interface_macros.h"

const int disks = 6;

struct
{
    struct stack *from;
    struct stack *aux;
    struct stack *to;
} print_aux = { 0 };

int get_nth(struct stack *st, int nth)
{
    if (cvx_tag(st) == 1)
    {
        return (l_get((struct list *)(st->instance), nth));
    }
    else if (cvx_tag(st) == 2)
    {
        return ll_get((struct llist *)(st)->instance, cvx_count(st) - nth - 1);
    }
    printf("Unknown tag %d\n", cvx_tag(st));
    exit(1);
}

void print_stacks(void)
{
    for (int i = 9; i >= 0; i--)
    {
        // NOTE: We are cheating a lot here just to print the stacks
        int ia = get_nth(print_aux.from, i);
        int ib = get_nth(print_aux.aux, i);
        int ic = get_nth(print_aux.to, i);

        printf("| %c | %c | %c |\n", ia == 0 ? 32 : ia + 48, ib == 0 ? 32 : ib + 48,
               ic == 0 ? 32 : ic + 48);
    }

    printf("+---+---+---+\n\n");
}

void move(struct stack *from, struct stack *to)
{
    cvx_push(to, cvx_pop(from));
}

void hanoi(int n, struct stack *from, struct stack *aux, struct stack *to)
{
    if (n == 0)
        return;

    hanoi(n - 1, from, to, aux);

    move(from, to);

    print_stacks();

    hanoi(n - 1, aux, from, to);
}

int main(void)
{
    struct llist l1;
    struct list l2, l3;
    l_init(&l2, NULL, 10);
    l_init(&l3, NULL, 10);
    ll_init(&l1, NULL);

    printf("%zu\n", l_capacity(&l2));
    printf("%zu\n", l_capacity(&l3));

    struct stack from = ll_as_stack((cvx_container *)(&l1));
    struct stack to = l_as_stack((cvx_container *)(&l2));
    struct stack aux = l_as_stack((cvx_container *)(&l3));

    for (int i = disks; i >= 1; i--)
        cvx_push(&from, i);

    printf("%zu\n", cvx_count(&from));

    print_aux.from = &from;
    print_aux.aux = &aux;
    print_aux.to = &to;

    print_stacks();

    hanoi(cvx_count(&from), &from, &aux, &to);

    cvx_drop(&from);
    cvx_drop(&to);
    cvx_drop(&aux);

    return 0;
}
