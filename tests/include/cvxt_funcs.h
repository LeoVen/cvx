#ifndef CVXT_FUNCS_H
#define CVXT_FUNCS_H

#include "cvx/flags.h"
#include <stddef.h>

#define cvxt_void(n) (void *)(intptr_t)(n)
#define cvxt_int(n) (intptr_t)(n)

size_t hash_void_collide_always(void *const *item);
void drop_void_simple_free(void **item);

int compare_void_as_int(void *const *a, void *const *b);
size_t hash_void_as_int(void *const *item);
size_t hash_void_as_int_collide_5(void *const *item);
size_t hash_void_as_int_ret_number(void *const *item);

int compare_void_as_char_ptr(void *const *a, void *const *b);
size_t hash_void_as_char_ptr(void *const *item);
enum cvx_flags copy_void_as_str(void *const *item, void **out);

#endif /* CVXT_FUNCS_H */
