// The order of imports matter
// clang-format off
#include "tests/interface/stack_dynamic_array_tests.h"
#include "tests/interface/stack_slinked_list_tests.h"

#include "tests/dynamic_array_tests.h"
#include "tests/dynamic_array_guard_tests.h"
#include "tests/dynamic_array_init_tests.h"
#include "tests/dynamic_array_iter_tests.h"
#include "tests/iter/dynamic_array_iter_random_access.h"

#include "tests/slinked_list_tests.h"
#include "tests/slinked_list_guard_tests.h"
#include "tests/slinked_list_init_tests.h"
#include "tests/slinked_list_iter_tests.h"
#include "tests/iter/slinked_list_iter_forward.h"
// clang-format on

int main(void)
{
    // clang-format off
    int (*tests[])(void) = {
        run_stack_dynamic_array_tests,
        run_stack_slinked_list_tests,

        run_dynamic_array_tests,
        run_dynamic_array_guard_tests,
        run_dynamic_array_init_tests,
        run_dynamic_array_iter_tests,
        run_dynamic_array_iter_random_access_tests,

        run_slinked_list_tests,
        run_slinked_list_guard_tests,
        run_slinked_list_init_tests,
        run_slinked_list_iter_tests,
        run_slinked_list_iter_forward_tests,
    };
    // clang-format on

    size_t count = sizeof(tests) / sizeof(tests[0]);
    int total = 0;

    for (size_t i = 0; i < count; i++)
    {
        total += tests[i]();
    }

    printf("Total Tests: %d\n", total);

    return 0;
}
