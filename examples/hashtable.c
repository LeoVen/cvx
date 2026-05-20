#include <stdio.h>
#include <string.h>

// ---- vtab callbacks ----

static int int_comp(int a, int b)
{
    return a - b;
}

static size_t int_hash(int a)
{
    return (size_t)(a < 0 ? -a : a);
}

// ---- instantiate ----

#define K int
#define V int
#define SNAME word_count
#define PFX wc
#define TAG 1
#include "cvx/hashtable.h"

// ---- example ----

int main(void)
{
    struct word_count_vtabk vtabk = { .comp = int_comp, .hash = int_hash };
    struct word_count ht;
    wc_init(&ht, &vtabk, NULL, 0);

    // Insert key-value pairs
    wc_insert(&ht, 10, 100);
    wc_insert(&ht, 20, 200);
    wc_insert(&ht, 30, 300);

    printf("count = %zu\n", wc_count(&ht));
    printf("get(10) = %d\n", wc_get(&ht, 10));
    printf("get(20) = %d\n", wc_get(&ht, 20));
    printf("contains(30) = %d\n", wc_contains(&ht, 30));
    printf("contains(99) = %d\n", wc_contains(&ht, 99));

    // Update a value
    int old = 0;
    wc_update(&ht, 10, 999, &old);
    printf("after update(10, 999): get(10) = %d, old = %d\n", wc_get(&ht, 10), old);

    // Remove an entry
    wc_remove(&ht, 20, NULL);
    printf("after remove(20): count = %zu\n", wc_count(&ht));

    // Forward iteration
    printf("entries:");
    struct word_count_iter iter = wc_iter_init_start(&ht);
    while (!wc_iter_at_end(&iter))
    {
        printf(" (%d→%d)", wc_iter_key(&iter), wc_iter_value(&iter));
        wc_iter_next(&iter);
    }
    printf("\n");

    wc_drop(&ht);
    return 0;
}
