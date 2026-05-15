#include <stdio.h>
#include <string.h>

#define V char *
#define SNAME darray
#define PFX da
#define TAG 1
#define ITERATOR
#include "cvx/dynamic_array.h"
static void str_drop(char *s)
{
    free(s);
}
static char *str_clone(char *s)
{
    return strdup(s);
}

struct darray_vtabv *vtab = &(struct darray_vtabv){
    .drop = str_drop,
    .clone = str_clone,
};

int main(void)
{
    struct darray darr;
    da_init(&darr, vtab, 0);
    char template[] = "  String";

    for (char i = ' ' + 1; i < 127; i++)
    {
        template[0] = i;
        da_push_back(&darr, strdup(template));
    }

    for (struct darray_iter itf = da_iter_start(&darr), itb = da_iter_end(&darr);
         !da_iter_at_end(&itf) && !da_iter_at_start(&itb); da_iter_next(&itf), da_iter_prev(&itb))
    {
        printf("%s\t%s\n", da_iter_value(&itf), da_iter_value(&itb));
    }

    da_drop(&darr);
}
