#include <stdio.h>

#include "cvx/core.h"

#define V int
#define PFX iset
#define SNAME interval
#define TAG 10
#include "cvx/interval_set.h"

int int_compare(int a, int b)
{
    return a - b;
}

static struct interval_vtabv *is_vtab = &(struct interval_vtabv){ .comp = int_compare };

#define K int
#define V const char *
#define PFX imap
#define SNAME interval_map
#define TAG 11
#include "cvx/interval_map.h"

static struct interval_map_vtabk *im_vtabk = &(struct interval_map_vtabk){
    .comp = int_compare,
};
static struct interval_map_vtabv *im_vtabv = &(struct interval_map_vtabv){
    .comp = strcmp,
};

int main(void)
{
    struct interval i0 = iset_init(is_vtab);
    cvx_container *i00 = (cvx_container *)&i0;

    if (i00->flag != CVX_FLAG_OK)
    {
        printf("Initializing error: %s\n", cvx_flags_str[i00->flag]);
    }

    iset_add(i00, 10, 20);
    iset_add(i00, 30, 40);
    iset_add(i00, 50, 60);
    iset_add(i00, 100, 101);

    if (i00->flag != CVX_FLAG_OK)
    {
        printf("Addition error: %s\n", cvx_flags_str[i00->flag]);
    }

    iset_remove(i00, 15, 35);
    iset_remove(i00, 99, 100); // No effect
    iset_remove(i00, 50, 51);
    iset_remove(i00, 51, 59);

    struct interval_iter set_iter = iset_iter_init_start(i00);
    cvx_container *i00_iter = (cvx_container *)&set_iter;

    for (iset_iter_to_start(i00_iter); !iset_iter_at_end(i00_iter); iset_iter_next(i00_iter))
    {
        struct interval_entry e = iset_iter_value(i00_iter);
        printf("[%d, %d)\n", e.lo, e.hi);
    }

    iset_clear(i00);
    printf("\n");

    struct interval_map im0 = imap_init(im_vtabk, im_vtabv);
    cvx_container *imap = (cvx_container *)&im0;

    imap_add(imap, 10, 20, "t1");
    imap_add(imap, 15, 20, "t1");
    imap_add(imap, 15, 20, "t2");
    imap_add(imap, 5, 10, "t0");

    printf("get(15) = %s\n", imap_get(imap, 15));
    printf("get(10) = %s\n", imap_get(imap, 10));

    struct interval_map_iter map_iter = imap_iter_init_start(imap);
    cvx_container *im_iter = (cvx_container *)&map_iter;

    for (imap_iter_to_start(im_iter); !imap_iter_at_end(im_iter); imap_iter_next(im_iter))
    {
        struct interval_map_entry e = imap_iter_value(im_iter);
        printf("[%d, %d): %s\n", e.lo, e.hi, e.val);
    }

    imap_clear(imap);

    return 0;
}
