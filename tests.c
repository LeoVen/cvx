// clang-format off

// intercept malloc implementation
// this must come before everything
#include "tests/alloc.h"

#include "tests/binary_heap_tests.h"
#include "tests/binary_heap_guard_tests.h"
#include "tests/binary_heap_iter_tests.h"
#include "tests/binary_heap_vtab_tests.h"
#include "tests/binary_heap_alloc_tests.h"

#include "tests/interface/deque_dlinked_list_tests.h"
#include "tests/interface/queue_slinked_list_tests.h"
#include "tests/interface/stack_slinked_list_tests.h"

#include "tests/dynamic_array_tests.h"

#include "tests/slinked_list_tests.h"
#include "tests/slinked_list_guard_tests.h"
#include "tests/slinked_list_iter_tests.h"
#include "tests/slinked_list_vtab_tests.h"
#include "tests/slinked_list_alloc_tests.h"
#include "tests/iter/slinked_list_iter_forward.h"

#include "tests/dlinked_list_tests.h"
#include "tests/dlinked_list_guard_tests.h"
#include "tests/dlinked_list_iter_tests.h"
#include "tests/dlinked_list_vtab_tests.h"
#include "tests/dlinked_list_alloc_tests.h"
#include "tests/iter/dlinked_list_iter_bidirectional.h"

#include "tests/interval_set_tests.h"
#include "tests/interval_set_guard_tests.h"
#include "tests/interval_set_iter_tests.h"
#include "tests/interval_set_vtab_tests.h"
#include "tests/interval_set_alloc_tests.h"

#include "tests/interval_map_tests.h"
#include "tests/interval_map_guard_tests.h"
#include "tests/interval_map_iter_tests.h"
#include "tests/interval_map_vtab_tests.h"
#include "tests/interval_map_alloc_tests.h"

#include "tests/hashtable_tests.h"
#include "tests/hashtable_guard_tests.h"
#include "tests/hashtable_iter_tests.h"
#include "tests/hashtable_vtab_tests.h"
#include "tests/hashtable_alloc_tests.h"
// clang-format on

int main(void)
{
    // clang-format off
    struct cvxresult (*tests[])(void) = {
        run_binary_heap_tests,
        run_binary_heap_guard_tests,
        run_binary_heap_iter_tests,
        run_binary_heap_vtab_tests,
        run_binary_heap_alloc_tests,

        run_deque_dlinked_list_tests,
        run_queue_slinked_list_tests,

        run_stack_slinked_list_tests,

        run_slinked_list_tests,
        run_slinked_list_guard_tests,
        run_slinked_list_iter_tests,
        run_slinked_list_vtab_tests,
        run_slinked_list_alloc_tests,
        run_slinked_list_iter_forward_tests,

        run_dlinked_list_tests,
        run_dlinked_list_guard_tests,
        run_dlinked_list_iter_tests,
        run_dlinked_list_vtab_tests,
        run_dlinked_list_alloc_tests,
        run_dlinked_list_iter_bidirectional_tests,

        run_interval_set_tests,
        run_interval_set_guard_tests,
        run_interval_set_iter_tests,
        run_interval_set_vtab_tests,
        run_interval_set_alloc_tests,

        run_interval_map_tests,
        run_interval_map_guard_tests,
        run_interval_map_iter_tests,
        run_interval_map_vtab_tests,
        run_interval_map_alloc_tests,

        run_hashtable_tests,
        run_hashtable_guard_tests,
        run_hashtable_iter_tests,
        run_hashtable_vtab_tests,
        run_hashtable_alloc_tests,

        run_dynamic_array_tests,
    };
    // clang-format on

    size_t count = sizeof(tests) / sizeof(tests[0]);
    int total_passed = 0;
    int total_failed = 0;

    for (size_t i = 0; i < count; i++)
    {
        struct cvxresult r = tests[i]();
        total_passed += r.passed;
        total_failed += r.failed;
    }

    printf("Total: %d passed", total_passed);
    printf(", %d failed\n", total_failed);

    printf("\n");

    return 0;
}
