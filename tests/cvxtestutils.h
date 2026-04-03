#ifndef CVX_TEST_UTILS_H
#define CVX_TEST_UTILS_H

#include <string.h>

static char *mkstr(const char *s)
{
    return strdup(s);
}

#define cvx_col(ds) ((cvx_container *)(&(ds)))

#endif /* CVX_TEST_UTILS_H */
