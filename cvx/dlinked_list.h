#include "cvx/fallback.h"

// clang-format off
#ifndef V
#error "cvx/dlinked_list.h requires V to be defined (the element type, e.g. #define V int)"
#endif
#ifndef SNAME
#error "cvx/dlinked_list.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_list)"
#endif
#ifndef PFX
#error "cvx/dlinked_list.h requires PFX to be defined (the function prefix, e.g. #define PFX ml)"
#endif
#ifndef TAG
#error "cvx/dlinked_list.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
#endif
// clang-format on

#include <stdlib.h>

#include "cvx/core.h"

#define FUNC(X) CVX_(PFX, X)
#define NODE CVX_(SNAME, _node)
#define ITERATOR CVX_(SNAME, _iter)
#define ITER_TAG (TAG * CVX_ITER_TAG_MULT)
#define VTAB_V CVX_(SNAME, _vtabv)

struct VTAB_V
{
    CVX_VTAB_DEFINITION(V)
};

struct NODE
{
    V value;
    struct NODE *prev;
    struct NODE *next;
};

struct SNAME
{
    cvx_container super;
    struct NODE *head;
    struct NODE *tail;
    size_t count;
    struct VTAB_V *vtabv;
};

struct ITERATOR
{
    cvx_container super;
    size_t index;
    struct SNAME *target;
    struct NODE *cursor; // NULL when index == target->count (past-end position)
};

// Non-allocating initializers
struct SNAME FUNC(_init)(struct VTAB_V *_vtabv_);
struct SNAME FUNC(_copy)(struct SNAME *_self_);

// Allocating initializers
cvx_container *FUNC(_new)(void);
cvx_container *FUNC(_new_with)(struct VTAB_V *_vtabv_);
cvx_container *FUNC(_clone)(cvx_container *_col_);

// Destructors
void FUNC(_drop)(cvx_container *_col_);
void FUNC(_clear)(cvx_container *_col_);

// Getters
size_t FUNC(_count)(cvx_container *_col_);
bool FUNC(_empty)(cvx_container *_col_);
V FUNC(_front)(cvx_container *_col_);
V FUNC(_back)(cvx_container *_col_);
V FUNC(_get)(cvx_container *_col_, size_t _index_);

// Operations
void FUNC(_push_front)(cvx_container *_col_, V _item_);
void FUNC(_push_back)(cvx_container *_col_, V _item_);
void FUNC(_push_at)(cvx_container *_col_, V _item_, size_t _index_);
V FUNC(_pop_front)(cvx_container *_col_);
V FUNC(_pop_back)(cvx_container *_col_);
V FUNC(_pop_at)(cvx_container *_col_, size_t _index_);
V FUNC(_replace_front)(cvx_container *_col_, V _new_);
V FUNC(_replace_back)(cvx_container *_col_, V _new_);

// Iterators
struct ITERATOR FUNC(_iter_init_start)(cvx_container *_target_);
struct ITERATOR FUNC(_iter_init_end)(cvx_container *_target_);
cvx_container *FUNC(_iter_start)(cvx_container *_target_);
cvx_container *FUNC(_iter_end)(cvx_container *_target_);
void FUNC(_iter_drop)(cvx_container *_iter_);
// Iterator state
bool FUNC(_iter_at_start)(cvx_container *_iter_);
bool FUNC(_iter_at_end)(cvx_container *_iter_);
size_t FUNC(_iter_count)(cvx_container *_iter_);
// Iterator movement
void FUNC(_iter_to_start)(cvx_container *_iter_);
void FUNC(_iter_to_end)(cvx_container *_iter_);
void FUNC(_iter_next)(cvx_container *_iter_);
void FUNC(_iter_prev)(cvx_container *_iter_);
void FUNC(_iter_forward)(cvx_container *_iter_, size_t _steps_);
void FUNC(_iter_backward)(cvx_container *_iter_, size_t _steps_);
// Iterator access
V FUNC(_iter_value)(cvx_container *_iter_);
size_t FUNC(_iter_index)(cvx_container *_iter_);

struct SNAME FUNC(_init)(struct VTAB_V *_vtabv_)
{
    struct SNAME _res_ = (struct SNAME){ 0 };
    _res_.super.tag = TAG;
    _res_.vtabv = _vtabv_;
    return _res_;
}

struct SNAME FUNC(_copy)(struct SNAME *_self_)
{
    struct SNAME _res_ = FUNC(_init)(_self_->vtabv);
    _res_.super.flag = CVX_FLAG_OK;

    struct NODE *_curr_ = _self_->head;
    while (_curr_)
    {
        struct NODE *_new_node_ = malloc(sizeof(struct NODE));
        if (!_new_node_)
        {
            struct NODE *_curr_ = _res_.head;
            while (_curr_)
            {
                struct NODE *_next_ = _curr_->next;
                if (_res_.vtabv && _res_.vtabv->drop)
                    _res_.vtabv->drop(_curr_->value);
                free(_curr_);
                _curr_ = _next_;
            }
            _res_.head = NULL;
            _res_.tail = NULL;
            _res_.count = 0;
            _res_.super.flag = CVX_FLAG_ALLOC;
            return _res_;
        }

        _new_node_->next = NULL;
        _new_node_->prev = _res_.tail;

        if (_self_->vtabv && _self_->vtabv->copy)
            _new_node_->value = _self_->vtabv->copy(_curr_->value);
        else
            _new_node_->value = _curr_->value;

        if (!_res_.head)
        {
            _res_.head = _new_node_;
            _res_.tail = _new_node_;
        }
        else
        {
            _res_.tail->next = _new_node_;
            _res_.tail = _new_node_;
        }

        _res_.count++;
        _curr_ = _curr_->next;
    }

    return _res_;
}

cvx_container *FUNC(_new)(void)
{
    struct SNAME *_res_ = malloc(sizeof(struct SNAME));

    if (!_res_)
        return NULL;

    _res_->super.tag = TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->head = NULL;
    _res_->tail = NULL;
    _res_->count = 0;
    _res_->vtabv = NULL;

    return (cvx_container *)_res_;
}

cvx_container *FUNC(_new_with)(struct VTAB_V *_vtabv_)
{
    struct SNAME *_res_ = malloc(sizeof(struct SNAME));

    if (!_res_)
        return NULL;

    _res_->super.tag = TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->head = NULL;
    _res_->tail = NULL;
    _res_->count = 0;
    _res_->vtabv = _vtabv_;

    return (cvx_container *)_res_;
}

cvx_container *FUNC(_clone)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, NULL);

    struct SNAME *_orig_ = (struct SNAME *)_col_;

    cvx_container *_res_ = FUNC(_new)();
    if (!_res_)
        return NULL;

    struct SNAME *_copy_ = (struct SNAME *)_res_;
    _copy_->vtabv = _orig_->vtabv;

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
        _new_node_->prev = _copy_->tail;

        if (_copy_->vtabv && _copy_->vtabv->copy)
            _new_node_->value = _copy_->vtabv->copy(_curr_->value);
        else
            _new_node_->value = _curr_->value;

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

    struct NODE *_curr_ = _self_->head;
    while (_curr_)
    {
        struct NODE *_next_ = _curr_->next;
        if (_self_->vtabv && _self_->vtabv->drop)
            _self_->vtabv->drop(_curr_->value);
        free(_curr_);
        _curr_ = _next_;
    }

    free(_self_);
}

void FUNC(_clear)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    struct NODE *_curr_ = _self_->head;
    while (_curr_)
    {
        struct NODE *_next_ = _curr_->next;
        if (_self_->vtabv && _self_->vtabv->drop)
            _self_->vtabv->drop(_curr_->value);
        free(_curr_);
        _curr_ = _next_;
    }

    _self_->head = NULL;
    _self_->tail = NULL;
    _self_->count = 0;
    _col_->flag = CVX_FLAG_OK;
}

size_t FUNC(_count)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, 0);

    struct SNAME *_self_ = (struct SNAME *)_col_;

    _col_->flag = CVX_FLAG_OK;
    return _self_->count;
}

bool FUNC(_empty)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, false);

    struct SNAME *_self_ = (struct SNAME *)_col_;

    _col_->flag = CVX_FLAG_OK;
    return _self_->count == 0;
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

V FUNC(_get)(cvx_container *_col_, size_t _index_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_index_ >= _self_->count)
    {
        _col_->flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    struct NODE *_curr_ = _self_->head;
    for (size_t i = 0; i < _index_; i++)
        _curr_ = _curr_->next;

    _col_->flag = CVX_FLAG_OK;
    return _curr_->value;
}

void FUNC(_push_front)(cvx_container *_col_, V _item_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    struct NODE *_node_ = malloc(sizeof(struct NODE));
    if (!_node_)
    {
        _col_->flag = CVX_FLAG_ALLOC;
        return;
    }

    _node_->value = _item_;
    _node_->prev = NULL;
    _node_->next = _self_->head;

    if (_self_->head)
        _self_->head->prev = _node_;

    _self_->head = _node_;

    if (!_self_->tail)
        _self_->tail = _node_;

    _self_->count++;
    _col_->flag = CVX_FLAG_OK;
}

void FUNC(_push_back)(cvx_container *_col_, V _item_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    struct NODE *_node_ = malloc(sizeof(struct NODE));
    if (!_node_)
    {
        _col_->flag = CVX_FLAG_ALLOC;
        return;
    }

    _node_->value = _item_;
    _node_->next = NULL;
    _node_->prev = _self_->tail;

    if (!_self_->tail)
    {
        _self_->head = _node_;
        _self_->tail = _node_;
    }
    else
    {
        _self_->tail->next = _node_;
        _self_->tail = _node_;
    }

    _self_->count++;
    _col_->flag = CVX_FLAG_OK;
}

void FUNC(_push_at)(cvx_container *_col_, V _item_, size_t _index_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_index_ > _self_->count)
    {
        _col_->flag = CVX_FLAG_RANGE;
        return;
    }

    if (_index_ == 0)
    {
        FUNC(_push_front)(_col_, _item_);
        return;
    }

    if (_index_ == _self_->count)
    {
        FUNC(_push_back)(_col_, _item_);
        return;
    }

    struct NODE *_node_ = malloc(sizeof(struct NODE));
    if (!_node_)
    {
        _col_->flag = CVX_FLAG_ALLOC;
        return;
    }

    struct NODE *_curr_ = _self_->head;
    for (size_t i = 0; i < _index_; i++)
        _curr_ = _curr_->next;

    _node_->value = _item_;
    _node_->next = _curr_;
    _node_->prev = _curr_->prev;
    _curr_->prev->next = _node_;
    _curr_->prev = _node_;

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

    struct NODE *_node_ = _self_->head;
    V _val_ = _node_->value;

    _self_->head = _node_->next;
    if (_self_->head)
        _self_->head->prev = NULL;
    else
        _self_->tail = NULL;

    free(_node_);
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

    struct NODE *_node_ = _self_->tail;
    V _val_ = _node_->value;

    _self_->tail = _node_->prev;
    if (_self_->tail)
        _self_->tail->next = NULL;
    else
        _self_->head = NULL;

    free(_node_);
    _self_->count--;
    _col_->flag = CVX_FLAG_OK;
    return _val_;
}

V FUNC(_pop_at)(cvx_container *_col_, size_t _index_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_self_->count == 0)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    if (_index_ >= _self_->count)
    {
        _col_->flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    if (_index_ == 0)
        return FUNC(_pop_front)(_col_);

    if (_index_ == _self_->count - 1)
        return FUNC(_pop_back)(_col_);

    struct NODE *_curr_ = _self_->head;
    for (size_t i = 0; i < _index_; i++)
        _curr_ = _curr_->next;

    V _val_ = _curr_->value;

    _curr_->prev->next = _curr_->next;
    _curr_->next->prev = _curr_->prev;
    free(_curr_);

    _self_->count--;
    _col_->flag = CVX_FLAG_OK;
    return _val_;
}

V FUNC(_replace_front)(cvx_container *_col_, V _new_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->head)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = _self_->head->value;
    _self_->head->value = _new_;
    _col_->flag = CVX_FLAG_OK;
    return _old_;
}

V FUNC(_replace_back)(cvx_container *_col_, V _new_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->tail)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = _self_->tail->value;
    _self_->tail->value = _new_;
    _col_->flag = CVX_FLAG_OK;
    return _old_;
}

///
///
/// ITERATOR
///
///

struct ITERATOR FUNC(_iter_init_start)(cvx_container *_target_)
{
    CVX_CONTAINER_GUARDS(TAG, _target_, (struct ITERATOR){ 0 });

    struct ITERATOR _res_ = { 0 };

    struct SNAME *_self_ = (struct SNAME *)_target_;

    _res_.super.tag = ITER_TAG;
    _res_.target = _self_;
    _res_.index = 0;
    _res_.cursor = _self_->head;
    _res_.super.flag = CVX_FLAG_OK;

    return _res_;
}

struct ITERATOR FUNC(_iter_init_end)(cvx_container *_target_)
{
    CVX_CONTAINER_GUARDS(TAG, _target_, (struct ITERATOR){ 0 });

    struct ITERATOR _res_ = { 0 };

    struct SNAME *_self_ = (struct SNAME *)_target_;

    _res_.super.tag = ITER_TAG;
    _res_.target = _self_;
    _res_.index = _self_->count;
    _res_.cursor = NULL;
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

cvx_container *FUNC(_iter_end)(cvx_container *_target_)
{
    CVX_CONTAINER_GUARDS(TAG, _target_, NULL);

    struct ITERATOR *_res_ = malloc(sizeof(struct ITERATOR));

    if (!_res_)
        return NULL;

    struct SNAME *_self_ = (struct SNAME *)_target_;

    _res_->super.tag = ITER_TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->target = _self_;
    _res_->index = _self_->count;
    _res_->cursor = NULL;

    return (cvx_container *)_res_;
}

void FUNC(_iter_drop)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    free(_iter_);
}

bool FUNC(_iter_at_start)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, false);

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    _iter_->flag = CVX_FLAG_OK;
    return _self_->index == 0;
}

bool FUNC(_iter_at_end)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, false);

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    _iter_->flag = CVX_FLAG_OK;
    return _self_->index == _self_->target->count;
}

size_t FUNC(_iter_count)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, 0);

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    _iter_->flag = CVX_FLAG_OK;
    return _self_->target->count;
}

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
    _self_->cursor = NULL;
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

void FUNC(_iter_prev)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    if (_self_->index == 0)
    {
        _iter_->flag = CVX_FLAG_RANGE;
        return;
    }

    _self_->index--;
    if (_self_->cursor == NULL)
        _self_->cursor = _self_->target->tail;
    else
        _self_->cursor = _self_->cursor->prev;

    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_forward)(cvx_container *_iter_, size_t _steps_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    size_t _remaining_ = _self_->target->count - _self_->index;
    size_t _actual_ = _steps_ < _remaining_ ? _steps_ : _remaining_;

    for (size_t i = 0; i < _actual_; i++)
        _self_->cursor = _self_->cursor->next;

    _self_->index += _actual_;
    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_backward)(cvx_container *_iter_, size_t _steps_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    size_t _actual_ = _steps_ < _self_->index ? _steps_ : _self_->index;

    for (size_t i = 0; i < _actual_; i++)
    {
        if (_self_->cursor == NULL)
            _self_->cursor = _self_->target->tail;
        else
            _self_->cursor = _self_->cursor->prev;
    }

    _self_->index -= _actual_;
    _iter_->flag = CVX_FLAG_OK;
}

V FUNC(_iter_value)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, (V){ 0 });

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    if (_self_->index >= _self_->target->count)
    {
        _iter_->flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    _iter_->flag = CVX_FLAG_OK;
    return _self_->cursor->value;
}

size_t FUNC(_iter_index)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, 0);

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    _iter_->flag = CVX_FLAG_OK;
    return _self_->index;
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

#include "cvx/interface/stack_cast.h"
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

#ifdef IMPL_QUEUE
#define INTERFACE IMPL_QUEUE

#define IMPL_NEW FUNC(_new)
#define IMPL_DROP FUNC(_drop)
#define IMPL_CLONE FUNC(_clone)
#define IMPL_ENQUEUE FUNC(_push_back)
#define IMPL_DEQUEUE FUNC(_pop_front)
#define IMPL_COUNT FUNC(_count)

#include "cvx/interface/queue_cast.h"
#undef IMPL_QUEUE

#undef IMPL_NEW
#undef IMPL_DROP
#undef IMPL_CLONE
#undef IMPL_ENQUEUE
#undef IMPL_DEQUEUE
#undef IMPL_COUNT
#endif

#ifdef IMPL_DEQUE
#define INTERFACE IMPL_DEQUE

#define IMPL_NEW FUNC(_new)
#define IMPL_CLONE FUNC(_clone)
#define IMPL_DROP FUNC(_drop)
#define IMPL_PUSH_FRONT FUNC(_push_front)
#define IMPL_PUSH_BACK FUNC(_push_back)
#define IMPL_POP_FRONT FUNC(_pop_front)
#define IMPL_POP_BACK FUNC(_pop_back)
#define IMPL_PEEK_FRONT FUNC(_front)
#define IMPL_PEEK_BACK FUNC(_back)
#define IMPL_COUNT FUNC(_count)

#include "cvx/interface/deque_cast.h"
#undef IMPL_DEQUE

#undef IMPL_NEW
#undef IMPL_CLONE
#undef IMPL_DROP
#undef IMPL_PUSH_FRONT
#undef IMPL_PUSH_BACK
#undef IMPL_POP_FRONT
#undef IMPL_POP_BACK
#undef IMPL_PEEK_FRONT
#undef IMPL_PEEK_BACK
#undef IMPL_COUNT
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

#ifdef IMPL_BIDIRECTIONAL_ITER
#define INTERFACE IMPL_BIDIRECTIONAL_ITER

#define IMPL_START FUNC(_iter_start)
#define IMPL_END FUNC(_iter_end)
#define IMPL_DROP FUNC(_iter_drop)
#define IMPL_AT_START FUNC(_iter_at_start)
#define IMPL_AT_END FUNC(_iter_at_end)
#define IMPL_COUNT FUNC(_iter_count)
#define IMPL_TO_START FUNC(_iter_to_start)
#define IMPL_TO_END FUNC(_iter_to_end)
#define IMPL_NEXT FUNC(_iter_next)
#define IMPL_PREV FUNC(_iter_prev)
#define IMPL_FORWARD FUNC(_iter_forward)
#define IMPL_BACKWARD FUNC(_iter_backward)
#define IMPL_VALUE FUNC(_iter_value)
#define IMPL_INDEX FUNC(_iter_index)

#include "cvx/iter/bidirectional_iterator_cast.h"
#undef IMPL_BIDIRECTIONAL_ITER

#undef IMPL_START
#undef IMPL_END
#undef IMPL_DROP
#undef IMPL_AT_START
#undef IMPL_AT_END
#undef IMPL_COUNT
#undef IMPL_TO_START
#undef IMPL_TO_END
#undef IMPL_NEXT
#undef IMPL_PREV
#undef IMPL_FORWARD
#undef IMPL_BACKWARD
#undef IMPL_VALUE
#undef IMPL_INDEX
#endif

#include "cvx/undef.h"
