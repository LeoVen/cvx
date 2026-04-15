#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "cvx/interface_macros.h"

#define V int
#define INTERFACE iterator
#include "cvx/iter/forward_iterator.h"

#define V int
#define INTERFACE queue
#include "cvx/interface/queue.h"

#define V int
#define PFX sl
#define SNAME slist
#define TAG 1
#define IMPL_FORWARD_ITER iterator
#define IMPL_QUEUE queue
#include "cvx/slinked_list.h"

int get_sum(struct iterator *it)
{
    int sum = 0;
    for (cvx_to_start(it); !cvx_at_end(it); cvx_next(it))
    {
        sum += cvx_value(it);
    }

    return sum;
}

int shift_in(struct queue *q, int new_item)
{
    cvx_enqueue(q, new_item);
    return cvx_dequeue(q);
}

int main(void)
{
    srand(time(NULL));

    struct slist *list = sl_new();
    struct queue queue = sl_as_queue((cvx_container *)list);

    // Initial data
    for (int i = 0; i < 256; i++)
    {
        sl_push_back(list, rand() % 256);
    }

    size_t num_items = cvx_count(&queue);

    for (int i = 0; i < 1000; i++)
    {
        struct slist_iter sl_iter = sl_iter_init_start(list);
        struct iterator iter = sl_iter_as_iterator((cvx_container *)(&sl_iter));

        int sum = get_sum(&iter);
        int count = cvx_count(&queue);

        printf("%d ", sum / count);

        // Ingest more data
        for (int j = 0; j < 64; j++)
        {
            shift_in(&queue, rand() % 256);
        }

        assert(num_items == cvx_count(&queue));
    }

    cvx_drop(&queue);

    printf("\n");

    return 0;
}
