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

int main(void)
{
    // Build a vtabv with copy and drop — no comp/hash/prio needed here
    struct str_array_vtabv cbs = { .copy = str_copy, .drop = str_drop };

    cvx_container *col = sa_new_with(&cbs, 8);

    sa_push_back(col, strdup("hello"));
    sa_push_back(col, strdup("world"));

    // clone() calls str_copy on each element — original and clone are independent
    cvx_container *clone = sa_clone(col);

    printf("%s %s\n", sa_get(col, 0), sa_get(clone, 1));

    sa_drop(col); // calls str_drop on each element, then frees the array
    sa_drop(clone);
}
