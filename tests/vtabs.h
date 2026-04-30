#ifndef CVX_VTABS_TEST_H
#define CVX_VTABS_TEST_H

#include "cvxtest.h"

static size_t cvx_counter_vtab_comp = 0;
#define CVX_TEST_COUNTER_COMP(t, n) CVXCHECK(t, n == cvx_counter_vtab_comp)
#define CVX_TEST_COUNTER_COMP_RESET() cvx_counter_vtab_comp = 0

static size_t cvx_counter_vtab_copy = 0;
#define CVX_TEST_COUNTER_COPY(t, n) CVXCHECK(t, n == cvx_counter_vtab_copy)
#define CVX_TEST_COUNTER_COPY_RESET() cvx_counter_vtab_copy = 0

static size_t cvx_counter_vtab_drop = 0;
#define CVX_TEST_COUNTER_DROP(t, n) CVXCHECK(t, n == cvx_counter_vtab_drop)
#define CVX_TEST_COUNTER_DROP_RESET() cvx_counter_vtab_drop = 0

int int_comp(int a, int b)
{
    cvx_counter_vtab_comp += 1;
    return a - b;
}

int int_copy(int a)
{
    cvx_counter_vtab_copy += 1;
    return a;
}

void int_drop(int a)
{
    (void)a;
    cvx_counter_vtab_drop += 1;
}

size_t int_hash(int a)
{
    return (size_t)(a < 0 ? -a : a);
}

#endif /* CVX_VTABS_TEST_H */
