#include <stdio.h>

#define V int
#define PFX bi_iter
#define SNAME bufiter
#define TAG 10
#include "cvx/array_iterator.h"
typedef struct bufiter bufiter;

#define V int
#define PFX ci_iter
#define SNAME citer
#define TAG 10
#define CIRCULAR
#include "cvx/array_iterator.h"
typedef struct citer citer;

#define V int
#define PFX si_iter
#define SNAME siter
#define TAG 10
#define SPARSE(item) item == 0
#include "cvx/array_iterator.h"
typedef struct siter siter;

int main(void)
{
    int items[100] = { 0 };
    int size = sizeof(items) / sizeof(int);
    int not_counted = 0;
    for (int i = 0; i < size; i++)
    {
        if (i % 5 != 0)
            items[i] = i;
        else
            not_counted++;
    }

    printf("Linear Buffer Iterator:\n");

    for (bufiter iterf = bi_iter_start(items, size, 50), iterb = bi_iter_end(items, size, 50);
         !bi_iter_at_end(&iterf) && !bi_iter_at_start(&iterb);
         bi_iter_next(&iterf), bi_iter_prev(&iterb))
    {
        printf("[%2zu] = %2d\t[%2zu] = %2d\n", bi_iter_index(&iterf), bi_iter_value(&iterf),
               bi_iter_index(&iterb), bi_iter_value(&iterb));
    }

    printf("\nCircular Buffer Iterator:\n");

    for (citer iterf = ci_iter_start(items, size, 50, 75), iterb = ci_iter_end(items, size, 50, 25);
         !ci_iter_at_end(&iterf) && !ci_iter_at_start(&iterb);
         ci_iter_next(&iterf), ci_iter_prev(&iterb))
    {
        printf("[%2zu] = %2d\t[%2zu] = %2d\n", ci_iter_index(&iterf), ci_iter_value(&iterf),
               ci_iter_index(&iterb), ci_iter_value(&iterb));
    }

    printf("\nSparse Buffer Iterator:\n");

    for (siter iterf = si_iter_start(items, size, size - not_counted),
               iterb = si_iter_end(items, size, size - not_counted);
         !si_iter_at_end(&iterf) && !si_iter_at_start(&iterb);
         si_iter_next(&iterf), si_iter_prev(&iterb))
    {
        printf("[%2zu] = %2d\t[%2zu] = %2d\n", si_iter_index(&iterf), si_iter_value(&iterf),
               si_iter_index(&iterb), si_iter_value(&iterb));
    }

    printf("\n");

    return 0;
}
