#include <stdio.h>

#define T int
#define PFX bi
#define SNAME bufiter
#define TAG 10
#include "cvx/buffer_iterator.h"
typedef struct bufiter bufiter;

#define T int
#define PFX ci
#define SNAME citer
#define TAG 10
#define CIRCULAR
#include "cvx/buffer_iterator.h"
typedef struct citer citer;

#define T int
#define PFX si
#define SNAME siter
#define TAG 10
#define SPARSE 0
#include "cvx/buffer_iterator.h"
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

    for (bufiter iter = bi_init(items, size, 50); !bi_at_end(&iter); bi_next(&iter))
    {
        printf("[%2zu] = %2d\n", bi_index(&iter), bi_value(&iter));
    }

    printf("\nCircular Buffer Iterator:\n");

    for (citer iter = ci_init(items, size, 50, 75); !ci_at_end(&iter); ci_next(&iter))
    {
        printf("[%2zu] = %2d\n", ci_index(&iter), ci_value(&iter));
    }

    printf("\nSparse Buffer Iterator:\n");

    for (siter iter = si_init(items, size, size - not_counted); !si_at_end(&iter); si_next(&iter))
    {
        printf("[%2zu] = %2d\n", si_index(&iter), si_value(&iter));
    }

    printf("\n");

    return 0;
}
