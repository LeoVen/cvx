#include "tests/dynamic_array_tests.h"
#include "tests/istack_dynamic_array_tests.h"
#include "tests/istack_slinked_list_tests.h"
#include "tests/slinked_list_tests.h"

int main(void)
{
    run_dynamic_array_tests();
    run_slinked_list_tests();

    run_istack_slinked_list_tests();
    run_istack_dynamic_array_tests();

    return 0;
}