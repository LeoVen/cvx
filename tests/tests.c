#include "cvx_test.h"
#include <stdio.h>

int main(void)
{
    struct cvxresult (*tests[])(void) = {
        // TODO:
    };

    size_t count = sizeof(tests) / sizeof(tests[0]);
    int total_passed = 0;
    int total_failed = 0;
    int total_empty = 0;

    for (size_t i = 0; i < count; i++)
    {
        struct cvxresult r = tests[i]();
        total_passed += r.passed;
        total_failed += r.failed;
        total_empty += r.empty;
    }

    printf("Total: %d passed, %d empty, %d failed\n", total_passed, total_empty, total_failed);

    return total_failed != 0;
}
