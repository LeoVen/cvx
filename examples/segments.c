#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define K int
#define V int
#define PFX st
#define SNAME segtree
#define TAG 10
#include "cvx/segment_tree.h"
typedef struct segtree segtree;

int sum(int a, int b)
{
    return a + b;
}

int max(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    return b;
}

int min(int a, int b)
{
    if (a < b)
    {
        return a;
    }
    return b;
}

void print_int(int key, int value)
{
    printf("K: %d - V: %d\n", key, value);
}

int main(void)
{
    int arr[] = { 1, 3, -2, 8, -7, 12, -12, 0, 2, 3, 2, 10, -1 };
    int n = sizeof(arr) / sizeof(arr[0]);

    segtree mintree, maxtree, sumtree, cloned;
    st_init(&mintree, n, INT_MAX, min);
    st_init(&maxtree, n, INT_MIN, max);
    st_init(&sumtree, n, 0, sum);

    st_build(&mintree, arr);
    st_build(&maxtree, arr);
    st_build(&sumtree, arr);

    st_clone(&sumtree, &cloned);

    st_for_each(&mintree, print_int);

    int w_size = 3;
    printf("\nWindow size: %d\n", w_size);
    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);
    printf("\n");
    for (int i = 0; i <= n - w_size; i++)
    {
        printf("min: %3d\tmax: %3d\tsum: %3d\tcloned: %3d\n", st_query(&mintree, i, i + w_size - 1),
               st_query(&maxtree, i, i + w_size - 1), st_query(&sumtree, i, i + w_size - 1),
               st_query(&cloned, i, i + w_size - 1));
    }

    printf("\nSequential:\n");
    for (int i = 0; i < mintree.leaves * 4; i++)
        printf("%2d ", mintree.tree[i]);

    st_drop(&mintree);
    st_drop(&maxtree);
    st_drop(&sumtree);
    st_drop(&cloned);

    return 0;
}
