// clang-format off
#include "tests/interface/deque_dlinked_list_tests.h"
#include "tests/interface/queue_slinked_list_tests.h"
#include "tests/interface/stack_dynamic_array_tests.h"
#include "tests/interface/stack_slinked_list_tests.h"

#include "tests/dynamic_array_tests.h"
#include "tests/dynamic_array_guard_tests.h"
#include "tests/dynamic_array_iter_tests.h"
#include "tests/dynamic_array_vtabv_tests.h"
#include "tests/iter/dynamic_array_iter_random_access.h"

#include "tests/slinked_list_tests.h"
#include "tests/slinked_list_guard_tests.h"
#include "tests/slinked_list_iter_tests.h"
#include "tests/slinked_list_vtabv_tests.h"
#include "tests/iter/slinked_list_iter_forward.h"

#include "tests/dlinked_list_tests.h"
#include "tests/dlinked_list_guard_tests.h"
#include "tests/dlinked_list_iter_tests.h"
#include "tests/dlinked_list_vtabv_tests.h"
#include "tests/iter/dlinked_list_iter_bidirectional.h"
// clang-format on

int main(void)
{
    // clang-format off
    int (*tests[])(void) = {
        run_deque_dlinked_list_tests,
        run_queue_slinked_list_tests,

        run_stack_dynamic_array_tests,
        run_stack_slinked_list_tests,

        run_dynamic_array_tests,
        run_dynamic_array_guard_tests,
        run_dynamic_array_iter_tests,
        run_dynamic_array_vtabv_tests,
        run_dynamic_array_iter_random_access_tests,

        run_slinked_list_tests,
        run_slinked_list_guard_tests,
        run_slinked_list_iter_tests,
        run_slinked_list_vtabv_tests,
        run_slinked_list_iter_forward_tests,

        run_dlinked_list_tests,
        run_dlinked_list_guard_tests,
        run_dlinked_list_iter_tests,
        run_dlinked_list_vtabv_tests,
        run_dlinked_list_iter_bidirectional_tests,
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
