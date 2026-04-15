#include <stdio.h>
#include <time.h>

#define V int
#define INTERFACE ra_iter
#include "cvx/iter/random_access_iterator.h"

#define V int
#define SNAME list
#define PFX l
#define TAG 1
#define IMPL_RANDOM_ACCESS_ITER ra_iter
#include "cvx/dynamic_array.h"

#include "cvx/interface_macros.h"

double calculate_average(struct ra_iter *values);

int main(void)
{
    srand(time(NULL));

    struct list *entries = l_new();

    for (int i = 0; i < 1000; i++)
        l_push_back(entries, rand() % 256);

    struct ra_iter iter = l_iter_as_ra_iter((cvx_container *)l_iter_start(entries));
    double avg = calculate_average(&iter);

    if (cvx_flag(&iter) != CVX_FLAG_OK)
    {
        enum cvx_flags f = cvx_flag(&iter);
        printf("Error %s: %s\n", cvx_flags_str[f], cvx_flags_description[f]);
        return 1;
    }

    printf("Average: %f\n", avg);

    cvx_drop(&iter);
    l_drop(entries);

    return 0;
}

double calculate_average(struct ra_iter *values)
{
    double result = 0.0;
    size_t entries = cvx_count(values);

    for (cvx_to_start(values); !cvx_at_end(values); cvx_next(values))
    {
        result += cvx_value(values);
        if (cvx_flag(values) != CVX_FLAG_OK)
            return 0.0;
    }

    return result / (double)entries;
}
