#ifndef QUEUE_SLINKED_TESTS_H
#define QUEUE_SLINKED_TESTS_H

#include "cvx/interface_macros.h"
#include "tests/cvxtest.h"
#include "tests/implementations.h"

#define MAKE_QUEUE(name) struct queue_int name = sll_int_as_queue_int((cvx_container *)sll_int_new())

/* ---- enqueue / count ---- */

static void test_sll_int_q_enqueue_count(struct cvxtest *t)
{
    MAKE_QUEUE(q);

    CVXCHECK(t, cvx_count(&q) == 0);
    cvx_enqueue(&q, 1);
    cvx_enqueue(&q, 2);
    cvx_enqueue(&q, 3);
    CVXCHECK(t, cvx_count(&q) == 3);
    CVXCHECK(t, cvx_flag(&q) == CVX_FLAG_OK);

    cvx_drop(&q);
}

static void test_sll_int_q_enqueue_many(struct cvxtest *t)
{
    MAKE_QUEUE(q);

    for (int i = 0; i < 100; i++)
        cvx_enqueue(&q, i);

    CVXCHECK(t, cvx_count(&q) == 100);
    CVXCHECK(t, cvx_flag(&q) == CVX_FLAG_OK);

    cvx_drop(&q);
}

/* ---- dequeue ---- */

static void test_sll_int_q_dequeue_fifo_order(struct cvxtest *t)
{
    MAKE_QUEUE(q);

    cvx_enqueue(&q, 10);
    cvx_enqueue(&q, 20);
    cvx_enqueue(&q, 30);

    CVXCHECK(t, cvx_dequeue(&q) == 10);
    CVXCHECK(t, cvx_dequeue(&q) == 20);
    CVXCHECK(t, cvx_dequeue(&q) == 30);
    CVXCHECK(t, cvx_count(&q) == 0);

    cvx_drop(&q);
}

static void test_sll_int_q_dequeue_empty(struct cvxtest *t)
{
    MAKE_QUEUE(q);

    cvx_dequeue(&q);
    CVXCHECK(t, cvx_flag(&q) == CVX_FLAG_EMPTY);

    cvx_drop(&q);
}

static void test_sll_int_q_dequeue_to_empty(struct cvxtest *t)
{
    MAKE_QUEUE(q);

    cvx_enqueue(&q, 1);
    cvx_dequeue(&q);

    CVXCHECK(t, cvx_count(&q) == 0);
    /* underlying list must have cleaned up both head and tail */
    CVXCHECK(t, ((struct slinked_int *)q.instance)->head == NULL);
    CVXCHECK(t, ((struct slinked_int *)q.instance)->tail == NULL);

    cvx_drop(&q);
}

/* ---- clone ---- */

static void test_sll_int_q_clone(struct cvxtest *t)
{
    MAKE_QUEUE(q);

    cvx_enqueue(&q, 10);
    cvx_enqueue(&q, 20);
    cvx_enqueue(&q, 30);

    struct slinked_int *copy = (struct slinked_int *)cvx_clone(&q);
    CVXCHECK(t, copy != NULL);
    if (!copy)
    {
        cvx_drop(&q);
        return;
    }

    /* clone is independent: dequeue from original does not affect copy */
    cvx_dequeue(&q);
    CVXCHECK(t, sll_int_count(copy) == 3);
    CVXCHECK(t, sll_int_front(copy) == 10);

    sll_int_drop(copy);
    cvx_drop(&q);
}

/* ---- runner ---- */

static struct cvxresult run_queue_slinked_list_tests(void)
{
    struct cvxtest t = { 0 };

    printf("queue (slinked_list)\n");

    CVXRUN(&t, test_sll_int_q_enqueue_count);
    CVXRUN(&t, test_sll_int_q_enqueue_many);

    CVXRUN(&t, test_sll_int_q_dequeue_fifo_order);
    CVXRUN(&t, test_sll_int_q_dequeue_empty);
    CVXRUN(&t, test_sll_int_q_dequeue_to_empty);

    CVXRUN(&t, test_sll_int_q_clone);

    return CVXSUMMARY(&t);
}

#endif /* QUEUE_SLINKED_TESTS_H */
