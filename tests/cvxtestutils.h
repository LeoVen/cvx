#ifndef CVX_TEST_UTILS_H
#define CVX_TEST_UTILS_H

#define MAKE_INVALID_CONTAINER(name) \
    cvx_container *name = &(cvx_container){ .flag = 0, .tag = 999999 };

#endif /* CVX_TEST_UTILS_H */
