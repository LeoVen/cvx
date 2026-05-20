#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define V char *
#define SNAME str_array
#define PFX sa
#define TAG 1
#include "cvx/dynamic_array.h"

char *str_copy(char *s)
{
    return strdup(s);
}

void str_drop(char *s)
{
    free(s);
}

static struct str_array_vtabv *cbs =
    &(struct str_array_vtabv){ .clone = str_copy, .drop = str_drop };

int main(void)
{
    struct str_array arr, *col = &arr, clone;
    sa_init(&arr, cbs, 0);

    sa_push_back(col, strdup("hello"));
    sa_push_back(col, strdup("world"));

    sa_clone(col, &clone);

    printf("%s %s\n", sa_get(col, 0), sa_get(&clone, 1));

    sa_drop(col); // calls str_drop on each element, then frees the array and `col` pointer
    sa_drop(&clone);
}
