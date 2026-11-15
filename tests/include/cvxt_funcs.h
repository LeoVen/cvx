#ifndef CVXT_FUNCS_H
#define CVXT_FUNCS_H

#include <stddef.h>

#define cvxt_void(n) (void *)(intptr_t)(n)
#define cvxt_int(n) (intptr_t)(n)

int compare_void_as_int(void *const *a, void *const *b);
size_t hash_void_as_int(void *const *item);
size_t hash_void_as_int_collide_always(void *const *item);
size_t hash_void_as_int_collide_5(void *const *item);
size_t hash_void_as_int_ret_number(void *const *item);

#endif /* CVXT_FUNCS_H */
