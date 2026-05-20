#include <stdio.h>
#include <stdlib.h>

int int_compare(int a, int b)
{
    return a - b;
}

#define V int
#define PFX pq
#define SNAME pqueue
#define TAG 10
#include "cvx/binary_heap.h"
typedef struct pqueue pqueue;
static struct pqueue_vtabv *vtab = &(struct pqueue_vtabv){
    .comp = int_compare,
};

int main(void)
{
    pqueue queue;
    pq_init(&queue, vtab, CVX_MIN_HEAP, 0);

    for (int i = 0; i < 100; i++)
    {
        pq_push(&queue, rand() % 512);
    }

    while (pq_count(&queue) > 0)
    {
        printf("%d ", pq_pop(&queue));
    }

    pq_drop(&queue);

    return 0;
}
