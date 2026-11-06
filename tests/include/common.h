#ifndef CVX_COMMON_H
#define CVX_COMMON_H

#include <stddef.h>
#include <stdint.h>

#define cvxt_void(n) (void *)(intptr_t)(n)
#define cvxt_int(n) (intptr_t)(n)

int compare_void_as_int(void *const *a, void *const *b)
{
    intptr_t inta = (int)(intptr_t)(*a);
    intptr_t intb = (int)(intptr_t)(*b);
    return inta - intb;
}

size_t hash_void_as_int(void *const *item)
{
    int x = (int)(intptr_t)(*item);
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = (x >> 16) ^ x;
    return x;
}

size_t hash_void_as_int_collide_5(void *const *item)
{
    int x = (int)(intptr_t)(*item);
    if (x % 5 == 0)
        return 0xAAAAAA;
    return hash_void_as_int(item);
}

#endif /* CVX_COMMON_H */
