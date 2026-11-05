#ifndef CVX_COMMON_H
#define CVX_COMMON_H

#include <stddef.h>
#include <stdint.h>

int compare_void_as_int(void *const *a, void *const *b)
{
    intptr_t inta = (int)(intptr_t)(a);
    intptr_t intb = (int)(intptr_t)(b);
    return inta - intb;
}

size_t hash_void_as_int(void *const *item)
{
    int x = (int)(intptr_t)item;
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = (x >> 16) ^ x;
    return x;
}

#endif /* CVX_COMMON_H */
