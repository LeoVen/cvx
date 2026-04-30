#include <stdio.h>

#define V int
#define PFX cb
#define SNAME cbuffer
#define TAG 10
#include "cvx/circular_buffer.h"
typedef struct cbuffer cbuffer;

int main(void)
{
    cbuffer *cb = cb_new_with(NULL, 10);

    for (int i = 10; i < 20; i++)
        cb_push_back(cb, i);

    printf("First insert: [");
    for (size_t i = 0; i < cb_count(cb); i++)
        printf("%2d ", cb_get(cb, i));

    printf("]\n\n");
    for (int i = 0; i < 24; i++)
    {
        printf("Overwrite: [");

        cb_push_back(cb, i);

        for (size_t i = 0; i < cb_count(cb); i++)
            printf("%2d ", cb_get(cb, i));

        printf("] - Front: %2d, Back: %2d\n", cb_front(cb), cb_back(cb));
    }

    cb_drop(cb);

    return 0;
}
