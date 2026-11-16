#include "cvxt_funcs.h"
#include "cvx/flags.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

size_t hash_void_collide_always(void *const *item)
{
    (void)item;
    return 0xAAAAAA;
}

void drop_void_simple_free(void **item)
{
    free(*item);
}

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

size_t hash_void_as_int_ret_number(void *const *item)
{
    return (size_t)(intptr_t)(*item);
}

enum cvx_flags copy_void_as_str(void *const *item, void **out)
{
    *out = strdup(*item);
    return CVX_FLAG_OK;
}

int compare_void_as_char_ptr(void *const *a, void *const *b)
{
    const char *ca = *a;
    const char *cb = *b;
    return strcmp(ca, cb);
}

size_t hash_void_as_char_ptr(void *const *item)
{
    const unsigned char *x = *item;
    unsigned int hval = 0;

    while (*x)
    {
        hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
        hval ^= *x++;
    }

    return hval;
}
