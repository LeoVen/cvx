#include <stdio.h>

#define V int
#define INTERFACE stack
#include "cvx/istack.h"

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

#define list_nth(s, i) ((struct list *)(s->instance))->buffer[i]
#define linked_nth(s, i) \
    (ll_get((struct llist *)(s)->instance, cvx_count(s) - i - 1))
void print_stacks()
{
    for (int i = 9; i >= 0; i--)
    {
        // NOTE: We are cheating a lot here just to print the stacks
        int ia = list_nth(print_aux.from, i);
        int ib = list_nth(print_aux.aux, i);
        int ic = linked_nth(print_aux.to, i);

        printf("| %c | %c | %c |\n", ia == 0 ? 32 : ia + 48,
               ib == 0 ? 32 : ib + 48, ic == 0 ? 32 : ic + 48);
    }

    printf("+---+---+---+\n\n");
}

void move(struct stack *from, struct stack *to)
{
    int res;
    cvx_pop(from, &res);
    cvx_push(to, res);
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

int main()
{
    printf("Main\n");

    struct stack from = l_as_stack(l_new_with(10));
    struct stack to = ll_as_stack(ll_new());
    struct stack aux = l_as_stack(l_new_with(10));

    printf("%d\n", cvx_flag(&from));

    for (int i = disks; i >= 1; i--)
        cvx_push(&from, i);

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
