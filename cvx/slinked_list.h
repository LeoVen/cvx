#include <stdlib.h>

#include "cvx/core.h"

#define FUNC(X) CVX_(PFX, X)
#define NODE CVX_(SNAME, _node)
#define ITERATOR CVX_(SNAME, _iter)
#define ITER_TAG (TAG * CVX_ITER_TAG_MULT)

struct NODE
{
    V value;
    struct NODE *next;
};

struct SNAME
{
    cvx_container super;
    struct NODE *head;
    struct NODE *tail;
    size_t count;
};

struct ITERATOR
{
    cvx_container super;
    size_t index;
    struct SNAME *target;
    struct NODE *cursor;
};

// Non-allocating initializer
struct SNAME FUNC(_init)(void);

// Allocating initializers
cvx_container *FUNC(_new)(void);
cvx_container *FUNC(_clone)(cvx_container *_col_);

// Destructors
void FUNC(_drop)(cvx_container *_col_);
void FUNC(_clear)(cvx_container *_col_);

// Getters
size_t FUNC(_count)(cvx_container *_col_);
bool FUNC(_empty)(cvx_container *_col_);
V FUNC(_front)(cvx_container *_col_);
V FUNC(_back)(cvx_container *_col_);
V FUNC(_get)(cvx_container *_col_, size_t index);

// Mutators
void FUNC(_push_front)(cvx_container *_col_, V item);
void FUNC(_push_back)(cvx_container *_col_, V item);
void FUNC(_push_at)(cvx_container *_col_, V item, size_t index);
V FUNC(_pop_front)(cvx_container *_col_);
V FUNC(_pop_back)(cvx_container *_col_);
V FUNC(_pop_at)(cvx_container *_col_, size_t index);
V FUNC(_replace_front)(cvx_container *_col_, V new);
V FUNC(_replace_back)(cvx_container *_col_, V new);

// Iterators
struct ITERATOR FUNC(_iter_init_start)(cvx_container *_target_);
cvx_container *FUNC(_iter_start)(cvx_container *_target_);
void FUNC(_iter_drop)(cvx_container *_iter_);
// Iterator state
bool FUNC(_iter_at_start)(cvx_container *_iter_);
bool FUNC(_iter_at_end)(cvx_container *_iter_);
size_t FUNC(_iter_count)(cvx_container *_iter_);
// Iterator movement
void FUNC(_iter_to_start)(cvx_container *_iter_);
void FUNC(_iter_to_end)(cvx_container *_iter_);
void FUNC(_iter_next)(cvx_container *_iter_);
void FUNC(_iter_forward)(cvx_container *_iter_, size_t steps);
// Iterator access
V FUNC(_iter_value)(cvx_container *_iter_);
size_t FUNC(_iter_index)(cvx_container *_iter_);

struct SNAME FUNC(_init)(void)
{
    struct SNAME _res_ = (struct SNAME){ 0 };
    _res_.super.tag = TAG;
    return _res_;
}

cvx_container *FUNC(_new)(void)
{
    struct SNAME *_res_ = calloc(1, sizeof(struct SNAME));

    if (!_res_)
        return NULL;

    _res_->super.tag = TAG;
    _res_->super.flag = CVX_FLAG_OK;

    return (cvx_container *)_res_;
}

cvx_container *FUNC(_clone)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, NULL);

    cvx_container *_res_ = FUNC(_new)();
    if (!_res_)
        return NULL;

    struct SNAME *_orig_ = (struct SNAME *)_col_;
    struct SNAME *_copy_ = (struct SNAME *)_res_;

    struct NODE *_curr_ = _orig_->head;
    while (_curr_)
    {
        struct NODE *_new_node_ = malloc(sizeof(struct NODE));
        if (!_new_node_)
        {
            FUNC(_drop)(_res_);
            return NULL;
        }

        _new_node_->next = NULL;

#ifdef V_COPY
        _new_node_->value = V_COPY(_curr_->value);
#else
        _new_node_->value = _curr_->value;
#endif

        if (!_copy_->head)
        {
            _copy_->head = _new_node_;
            _copy_->tail = _new_node_;
        }
        else
        {
            _copy_->tail->next = _new_node_;
            _copy_->tail = _new_node_;
        }

        _copy_->count++;
        _curr_ = _curr_->next;
    }

    _res_->flag = CVX_FLAG_OK;
    return _res_;
}

void FUNC(_drop)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    struct NODE *curr = _self_->head;
    while (curr)
    {
        struct NODE *next = curr->next;
#ifdef V_DROP
        V_DROP(curr->value);
#endif
        free(curr);
        curr = next;
    }

    free(_self_);
}

void FUNC(_clear)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    struct NODE *curr = _self_->head;
    while (curr)
    {
        struct NODE *next = curr->next;
#ifdef V_DROP
        V_DROP(curr->value);
#endif
        free(curr);
        curr = next;
    }

    _self_->head = NULL;
    _self_->tail = NULL;
    _self_->count = 0;
    _col_->flag = CVX_FLAG_OK;
}

size_t FUNC(_count)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, 0);

    return ((struct SNAME *)_col_)->count;
}

bool FUNC(_empty)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, false);

    return ((struct SNAME *)_col_)->count == 0;
}

V FUNC(_front)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->head)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    _col_->flag = CVX_FLAG_OK;
    return _self_->head->value;
}

V FUNC(_back)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->tail)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    _col_->flag = CVX_FLAG_OK;
    return _self_->tail->value;
}

V FUNC(_get)(cvx_container *_col_, size_t index)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (index >= _self_->count)
    {
        _col_->flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    struct NODE *curr = _self_->head;
    for (size_t i = 0; i < index; i++)
        curr = curr->next;

    _col_->flag = CVX_FLAG_OK;
    return curr->value;
}

void FUNC(_push_front)(cvx_container *_col_, V item)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    struct NODE *node = malloc(sizeof(struct NODE));
    if (!node)
    {
        _col_->flag = CVX_FLAG_ALLOC;
        return;
    }

    node->value = item;
    node->next = _self_->head;
    _self_->head = node;

    if (!_self_->tail)
        _self_->tail = node;

    _self_->count++;
    _col_->flag = CVX_FLAG_OK;
}

void FUNC(_push_back)(cvx_container *_col_, V item)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    struct NODE *node = malloc(sizeof(struct NODE));
    if (!node)
    {
        _col_->flag = CVX_FLAG_ALLOC;
        return;
    }

    node->value = item;
    node->next = NULL;

    if (!_self_->tail)
    {
        _self_->head = node;
        _self_->tail = node;
    }
    else
    {
        _self_->tail->next = node;
        _self_->tail = node;
    }

    _self_->count++;
    _col_->flag = CVX_FLAG_OK;
}

void FUNC(_push_at)(cvx_container *_col_, V item, size_t index)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (index > _self_->count)
    {
        _col_->flag = CVX_FLAG_RANGE;
        return;
    }

    if (index == 0)
    {
        FUNC(_push_front)(_col_, item);
        return;
    }

    if (index == _self_->count)
    {
        FUNC(_push_back)(_col_, item);
        return;
    }

    struct NODE *node = malloc(sizeof(struct NODE));
    if (!node)
    {
        _col_->flag = CVX_FLAG_ALLOC;
        return;
    }

    struct NODE *prev = _self_->head;
    for (size_t i = 0; i < index - 1; i++)
        prev = prev->next;

    node->value = item;
    node->next = prev->next;
    prev->next = node;

    _self_->count++;
    _col_->flag = CVX_FLAG_OK;
}

V FUNC(_pop_front)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->head)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    struct NODE *node = _self_->head;
    V _val_ = node->value;

    _self_->head = node->next;
    if (!_self_->head)
        _self_->tail = NULL;

    free(node);
    _self_->count--;
    _col_->flag = CVX_FLAG_OK;
    return _val_;
}

V FUNC(_pop_back)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->tail)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _val_ = _self_->tail->value;

    if (_self_->count == 1)
    {
        free(_self_->head);
        _self_->head = NULL;
        _self_->tail = NULL;
    }
    else
    {
        struct NODE *prev = _self_->head;
        while (prev->next != _self_->tail)
            prev = prev->next;

        free(_self_->tail);
        prev->next = NULL;
        _self_->tail = prev;
    }

    _self_->count--;
    _col_->flag = CVX_FLAG_OK;
    return _val_;
}

V FUNC(_pop_at)(cvx_container *_col_, size_t index)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_self_->count == 0)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    if (index >= _self_->count)
    {
        _col_->flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    if (index == 0)
        return FUNC(_pop_front)(_col_);

    if (index == _self_->count - 1)
        return FUNC(_pop_back)(_col_);

    struct NODE *prev = _self_->head;
    for (size_t i = 0; i < index - 1; i++)
        prev = prev->next;

    struct NODE *node = prev->next;
    V _val_ = node->value;

    prev->next = node->next;
    free(node);

    _self_->count--;
    _col_->flag = CVX_FLAG_OK;
    return _val_;
}

V FUNC(_replace_front)(cvx_container *_col_, V new)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->head)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = _self_->head->value;
    _self_->head->value = new;
    _col_->flag = CVX_FLAG_OK;
    return _old_;
}

V FUNC(_replace_back)(cvx_container *_col_, V new)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->tail)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = _self_->tail->value;
    _self_->tail->value = new;
    _col_->flag = CVX_FLAG_OK;
    return _old_;
}

///
///
/// ITERATOR
///
///

// Iterators
struct ITERATOR FUNC(_iter_init_start)(cvx_container *_target_)
{
    struct ITERATOR _res_ = { 0 };

    if (_target_->tag != TAG)
    {
        _res_.super.flag = CVX_FLAG_WRONG_TAG;
        return _res_;
    }

    struct SNAME *_self_ = (struct SNAME *)_target_;

    _res_.super.tag = ITER_TAG;
    _res_.target = _self_;
    _res_.index = 0;
    _res_.cursor = _self_->head;
    _res_.super.flag = CVX_FLAG_OK;

    return _res_;
}

cvx_container *FUNC(_iter_start)(cvx_container *_target_)
{
    CVX_CONTAINER_GUARDS(TAG, _target_, NULL);

    struct ITERATOR *_res_ = malloc(sizeof(struct ITERATOR));

    if (!_res_)
        return NULL;

    struct SNAME *_self_ = (struct SNAME *)_target_;

    _res_->super.tag = ITER_TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->index = 0;
    _res_->target = _self_;
    _res_->cursor = _self_->head;

    return (cvx_container *)_res_;
}

void FUNC(_iter_drop)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    free(_iter_);
}

// Iterator state
bool FUNC(_iter_at_start)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, false);

    return ((struct ITERATOR *)_iter_)->index == 0;
}

bool FUNC(_iter_at_end)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, false);

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    return _self_->index == _self_->target->count;
}

size_t FUNC(_iter_count)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, 0);

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    return _self_->target->count;
}

// Iterator movement
void FUNC(_iter_to_start)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    _self_->index = 0;
    _self_->cursor = _self_->target->head;
    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_to_end)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    _self_->index = _self_->target->count;
    _self_->cursor = _self_->target->tail;
    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_next)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    if (_self_->index >= _self_->target->count)
    {
        _iter_->flag = CVX_FLAG_RANGE;
        return;
    }

    _self_->index++;
    _self_->cursor = _self_->cursor->next;
    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_forward)(cvx_container *_iter_, size_t steps)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    size_t remaining = _self_->target->count - _self_->index;

    if (remaining < steps)
    {
        _iter_->flag = CVX_FLAG_RANGE;
        return;
    }

    for (size_t i = 0; i < steps; i++)
        _self_->cursor = _self_->cursor->next;

    _self_->index += steps;
    _iter_->flag = CVX_FLAG_OK;
}

// Iterator access
V FUNC(_iter_value)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, (V){ 0 });

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    if (_self_->index >= _self_->target->count)
    {
        _iter_->flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    return _self_->cursor->value;
}

size_t FUNC(_iter_index)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, 0);

    return ((struct ITERATOR *)_iter_)->index;
}

#ifdef IMPL_STACK
#define INTERFACE IMPL_STACK

#define IMPL_NEW FUNC(_new)
#define IMPL_DROP FUNC(_drop)
#define IMPL_CLONE FUNC(_clone)
#define IMPL_PUSH FUNC(_push_front)
#define IMPL_POP FUNC(_pop_front)
#define IMPL_COUNT FUNC(_count)
#define IMPL_PEEK FUNC(_front)
#define IMPL_REPLACE FUNC(_replace_front)

#include "istack_cast.h"
#undef IMPL_STACK

#undef IMPL_NEW
#undef IMPL_DROP
#undef IMPL_CLONE
#undef IMPL_PUSH
#undef IMPL_POP
#undef IMPL_COUNT
#undef IMPL_PEEK
#undef IMPL_REPLACE
#endif

#ifdef IMPL_FORWARD_ITER
#define INTERFACE IMPL_FORWARD_ITER

#define IMPL_START FUNC(_iter_start)
#define IMPL_DROP FUNC(_iter_drop)
#define IMPL_AT_START FUNC(_iter_at_start)
#define IMPL_AT_END FUNC(_iter_at_end)
#define IMPL_COUNT FUNC(_iter_count)
#define IMPL_TO_START FUNC(_iter_to_start)
#define IMPL_NEXT FUNC(_iter_next)
#define IMPL_FORWARD FUNC(_iter_forward)
#define IMPL_VALUE FUNC(_iter_value)
#define IMPL_INDEX FUNC(_iter_index)

#include "cvx/iter/forward_iterator_cast.h"
#undef IMPL_FORWARD_ITER

#undef IMPL_START
#undef IMPL_DROP
#undef IMPL_AT_START
#undef IMPL_AT_END
#undef IMPL_COUNT
#undef IMPL_TO_START
#undef IMPL_NEXT
#undef IMPL_FORWARD
#undef IMPL_VALUE
#undef IMPL_INDEX
#endif

#include "cvx/undef.h"
