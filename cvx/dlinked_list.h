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
#define FUNC_PROXY(X) CVX_(PFX, CVX_(__proxy, X))
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
struct SNAME *FUNC(_new)(void);
struct SNAME *FUNC(_new_with)(struct VTAB_V *_vtabv_);
struct SNAME *FUNC(_clone)(struct SNAME *_orig_);

// Destructors
void FUNC(_drop)(struct SNAME *_self_);
void FUNC(_clear)(struct SNAME *_self_);

// Getters
size_t FUNC(_count)(struct SNAME *_self_);
bool FUNC(_empty)(struct SNAME *_self_);
V FUNC(_front)(struct SNAME *_self_);
V FUNC(_back)(struct SNAME *_self_);
V FUNC(_get)(struct SNAME *_self_, size_t _index_);

// Operations
void FUNC(_push_front)(struct SNAME *_self_, V _item_);
void FUNC(_push_back)(struct SNAME *_self_, V _item_);
void FUNC(_push_at)(struct SNAME *_self_, V _item_, size_t _index_);
V FUNC(_pop_front)(struct SNAME *_self_);
V FUNC(_pop_back)(struct SNAME *_self_);
V FUNC(_pop_at)(struct SNAME *_self_, size_t _index_);
V FUNC(_replace_front)(struct SNAME *_self_, V _new_);
V FUNC(_replace_back)(struct SNAME *_self_, V _new_);

// Iterators
struct ITERATOR FUNC(_iter_init_start)(struct SNAME *_target_);
struct ITERATOR FUNC(_iter_init_end)(struct SNAME *_target_);
struct ITERATOR *FUNC(_iter_start)(struct SNAME *_target_);
struct ITERATOR *FUNC(_iter_end)(struct SNAME *_target_);
void FUNC(_iter_drop)(struct ITERATOR *_iter_);
// Iterator state
bool FUNC(_iter_at_start)(struct ITERATOR *_iter_);
bool FUNC(_iter_at_end)(struct ITERATOR *_iter_);
size_t FUNC(_iter_count)(struct ITERATOR *_iter_);
// Iterator movement
void FUNC(_iter_to_start)(struct ITERATOR *_iter_);
void FUNC(_iter_to_end)(struct ITERATOR *_iter_);
void FUNC(_iter_next)(struct ITERATOR *_iter_);
void FUNC(_iter_prev)(struct ITERATOR *_iter_);
void FUNC(_iter_forward)(struct ITERATOR *_iter_, size_t _steps_);
void FUNC(_iter_backward)(struct ITERATOR *_iter_, size_t _steps_);
// Iterator access
V FUNC(_iter_value)(struct ITERATOR *_iter_);
size_t FUNC(_iter_index)(struct ITERATOR *_iter_);

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

struct SNAME *FUNC(_new)(void)
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

    return _res_;
}

struct SNAME *FUNC(_new_with)(struct VTAB_V *_vtabv_)
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

    return _res_;
}

struct SNAME *FUNC(_clone)(struct SNAME *_orig_)
{
    struct SNAME *_copy_ = FUNC(_new)();
    if (!_copy_)
        return NULL;

    _copy_->vtabv = _orig_->vtabv;

    struct NODE *_curr_ = _orig_->head;
    while (_curr_)
    {
        struct NODE *_new_node_ = malloc(sizeof(struct NODE));
        if (!_new_node_)
        {
            FUNC(_drop)(_copy_);
            _orig_->super.flag = CVX_FLAG_ALLOC;
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

    _orig_->super.flag = CVX_FLAG_ALLOC;
    _copy_->super.flag = CVX_FLAG_OK;
    return _copy_;
}

void FUNC(_drop)(struct SNAME *_self_)
{
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

void FUNC(_clear)(struct SNAME *_self_)
{
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
    _self_->super.flag = CVX_FLAG_OK;
}

size_t FUNC(_count)(struct SNAME *_self_)
{
    _self_->super.flag = CVX_FLAG_OK;
    return _self_->count;
}

bool FUNC(_empty)(struct SNAME *_self_)
{
    _self_->super.flag = CVX_FLAG_OK;
    return _self_->count == 0;
}

V FUNC(_front)(struct SNAME *_self_)
{
    if (!_self_->head)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    _self_->super.flag = CVX_FLAG_OK;
    return _self_->head->value;
}

V FUNC(_back)(struct SNAME *_self_)
{
    if (!_self_->tail)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    _self_->super.flag = CVX_FLAG_OK;
    return _self_->tail->value;
}

V FUNC(_get)(struct SNAME *_self_, size_t _index_)
{
    if (_index_ >= _self_->count)
    {
        _self_->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    struct NODE *_curr_ = _self_->head;
    for (size_t i = 0; i < _index_; i++)
        _curr_ = _curr_->next;

    _self_->super.flag = CVX_FLAG_OK;
    return _curr_->value;
}

void FUNC(_push_front)(struct SNAME *_self_, V _item_)
{
    struct NODE *_node_ = malloc(sizeof(struct NODE));
    if (!_node_)
    {
        _self_->super.flag = CVX_FLAG_ALLOC;
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
    _self_->super.flag = CVX_FLAG_OK;
}

void FUNC(_push_back)(struct SNAME *_self_, V _item_)
{
    struct NODE *_node_ = malloc(sizeof(struct NODE));
    if (!_node_)
    {
        _self_->super.flag = CVX_FLAG_ALLOC;
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
    _self_->super.flag = CVX_FLAG_OK;
}

void FUNC(_push_at)(struct SNAME *_self_, V _item_, size_t _index_)
{
    if (_index_ > _self_->count)
    {
        _self_->super.flag = CVX_FLAG_RANGE;
        return;
    }

    if (_index_ == 0)
    {
        FUNC(_push_front)(_self_, _item_);
        return;
    }

    if (_index_ == _self_->count)
    {
        FUNC(_push_back)(_self_, _item_);
        return;
    }

    struct NODE *_node_ = malloc(sizeof(struct NODE));
    if (!_node_)
    {
        _self_->super.flag = CVX_FLAG_ALLOC;
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
    _self_->super.flag = CVX_FLAG_OK;
}

V FUNC(_pop_front)(struct SNAME *_self_)
{
    if (!_self_->head)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
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
    _self_->super.flag = CVX_FLAG_OK;
    return _val_;
}

V FUNC(_pop_back)(struct SNAME *_self_)
{
    if (!_self_->tail)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
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
    _self_->super.flag = CVX_FLAG_OK;
    return _val_;
}

V FUNC(_pop_at)(struct SNAME *_self_, size_t _index_)
{
    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    if (_index_ >= _self_->count)
    {
        _self_->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    if (_index_ == 0)
        return FUNC(_pop_front)(_self_);

    if (_index_ == _self_->count - 1)
        return FUNC(_pop_back)(_self_);

    struct NODE *_curr_ = _self_->head;
    for (size_t i = 0; i < _index_; i++)
        _curr_ = _curr_->next;

    V _val_ = _curr_->value;

    _curr_->prev->next = _curr_->next;
    _curr_->next->prev = _curr_->prev;
    free(_curr_);

    _self_->count--;
    _self_->super.flag = CVX_FLAG_OK;
    return _val_;
}

V FUNC(_replace_front)(struct SNAME *_self_, V _new_)
{
    if (!_self_->head)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = _self_->head->value;
    _self_->head->value = _new_;
    _self_->super.flag = CVX_FLAG_OK;
    return _old_;
}

V FUNC(_replace_back)(struct SNAME *_self_, V _new_)
{
    if (!_self_->tail)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = _self_->tail->value;
    _self_->tail->value = _new_;
    _self_->super.flag = CVX_FLAG_OK;
    return _old_;
}

///
///
/// ITERATOR
///
///

struct ITERATOR FUNC(_iter_init_start)(struct SNAME *_target_)
{
    struct ITERATOR _res_ = { 0 };

    _res_.super.tag = ITER_TAG;
    _res_.target = _target_;
    _res_.index = 0;
    _res_.cursor = _target_->head;
    _res_.super.flag = CVX_FLAG_OK;

    return _res_;
}

struct ITERATOR FUNC(_iter_init_end)(struct SNAME *_target_)
{
    struct ITERATOR _res_ = { 0 };

    _res_.super.tag = ITER_TAG;
    _res_.target = _target_;
    _res_.index = _target_->count;
    _res_.cursor = NULL;
    _res_.super.flag = CVX_FLAG_OK;

    return _res_;
}

struct ITERATOR *FUNC(_iter_start)(struct SNAME *_target_)
{
    struct ITERATOR *_res_ = malloc(sizeof(struct ITERATOR));

    if (!_res_)
        return NULL;

    _res_->super.tag = ITER_TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->index = 0;
    _res_->target = _target_;
    _res_->cursor = _target_->head;

    return _res_;
}

struct ITERATOR *FUNC(_iter_end)(struct SNAME *_target_)
{
    struct ITERATOR *_res_ = malloc(sizeof(struct ITERATOR));

    if (!_res_)
        return NULL;

    _res_->super.tag = ITER_TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->target = _target_;
    _res_->index = _target_->count;
    _res_->cursor = NULL;

    return _res_;
}

void FUNC(_iter_drop)(struct ITERATOR *_iter_)
{
    free(_iter_);
}

bool FUNC(_iter_at_start)(struct ITERATOR *_iter_)
{
    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->index == 0;
}

bool FUNC(_iter_at_end)(struct ITERATOR *_iter_)
{
    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->index == _iter_->target->count;
}

size_t FUNC(_iter_count)(struct ITERATOR *_iter_)
{
    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->target->count;
}

void FUNC(_iter_to_start)(struct ITERATOR *_iter_)
{
    _iter_->index = 0;
    _iter_->cursor = _iter_->target->head;
    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_to_end)(struct ITERATOR *_iter_)
{
    _iter_->index = _iter_->target->count;
    _iter_->cursor = NULL;
    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_next)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return;
    }

    _iter_->index++;
    _iter_->cursor = _iter_->cursor->next;
    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_prev)(struct ITERATOR *_iter_)
{
    if (_iter_->index == 0)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return;
    }

    _iter_->index--;
    if (_iter_->cursor == NULL)
        _iter_->cursor = _iter_->target->tail;
    else
        _iter_->cursor = _iter_->cursor->prev;

    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_forward)(struct ITERATOR *_iter_, size_t _steps_)
{
    size_t _remaining_ = _iter_->target->count - _iter_->index;
    size_t _actual_ = _steps_ < _remaining_ ? _steps_ : _remaining_;

    for (size_t i = 0; i < _actual_; i++)
        _iter_->cursor = _iter_->cursor->next;

    _iter_->index += _actual_;
    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_backward)(struct ITERATOR *_iter_, size_t _steps_)
{
    size_t _actual_ = _steps_ < _iter_->index ? _steps_ : _iter_->index;

    for (size_t i = 0; i < _actual_; i++)
    {
        if (_iter_->cursor == NULL)
            _iter_->cursor = _iter_->target->tail;
        else
            _iter_->cursor = _iter_->cursor->prev;
    }

    _iter_->index -= _actual_;
    _iter_->super.flag = CVX_FLAG_OK;
}

V FUNC(_iter_value)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->cursor->value;
}

size_t FUNC(_iter_index)(struct ITERATOR *_iter_)
{
    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->index;
}

///
///
/// PROXIES
///
///

// clang-format off
cvx_container *FUNC_PROXY(_new)(void) { return (cvx_container *)FUNC(_new)(); }
cvx_container *FUNC_PROXY(_new_with)(struct VTAB_V *_vtabv_) { return (cvx_container *)FUNC(_new_with)(_vtabv_); }
cvx_container *FUNC_PROXY(_clone)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, NULL); return (cvx_container *)FUNC(_clone)((struct SNAME *)_col_); }
void FUNC_PROXY(_drop)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_drop)((struct SNAME *)_col_); }
void FUNC_PROXY(_clear)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_clear)((struct SNAME *)_col_); }
size_t FUNC_PROXY(_count)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, 0); return FUNC(_count)((struct SNAME *)_col_); }
bool FUNC_PROXY(_empty)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_empty)((struct SNAME *)_col_); }
V FUNC_PROXY(_front)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_front)((struct SNAME *)_col_); }
V FUNC_PROXY(_back)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_back)((struct SNAME *)_col_); }
V FUNC_PROXY(_get)(cvx_container *_col_, size_t _index_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_get)((struct SNAME *)_col_, _index_); }
void FUNC_PROXY(_push_front)(cvx_container *_col_, V _item_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_push_front)((struct SNAME *)_col_, _item_); }
void FUNC_PROXY(_push_back)(cvx_container *_col_, V _item_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_push_back)((struct SNAME *)_col_, _item_); }
void FUNC_PROXY(_push_at)(cvx_container *_col_, V _item_, size_t _index_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_push_at)((struct SNAME *)_col_, _item_, _index_); }
V FUNC_PROXY(_pop_front)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_pop_front)((struct SNAME *)_col_); }
V FUNC_PROXY(_pop_back)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_pop_back)((struct SNAME *)_col_); }
V FUNC_PROXY(_pop_at)(cvx_container *_col_, size_t _index_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_pop_at)((struct SNAME *)_col_, _index_); }
V FUNC_PROXY(_replace_front)(cvx_container *_col_, V _new_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_replace_front)((struct SNAME *)_col_, _new_); }
V FUNC_PROXY(_replace_back)(cvx_container *_col_, V _new_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_replace_back)((struct SNAME *)_col_, _new_); }

// ITERATORS

cvx_container *FUNC_PROXY(_iter_start)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, NULL); return (cvx_container *)FUNC(_iter_start)((struct SNAME *)_col_); }
cvx_container *FUNC_PROXY(_iter_end)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, NULL); return (cvx_container *)FUNC(_iter_end)((struct SNAME *)_col_); }
void FUNC_PROXY(_iter_drop)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, ); FUNC(_iter_drop)((struct ITERATOR *)_col_); }
bool FUNC_PROXY(_iter_at_start)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, false); return FUNC(_iter_at_start)((struct ITERATOR *)_col_); }
bool FUNC_PROXY(_iter_at_end)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, false); return FUNC(_iter_at_end)((struct ITERATOR *)_col_); }
size_t FUNC_PROXY(_iter_count)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, 0); return FUNC(_iter_count)((struct ITERATOR *)_col_); }
void FUNC_PROXY(_iter_to_start)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, ); FUNC(_iter_to_start)((struct ITERATOR *)_col_); }
void FUNC_PROXY(_iter_to_end)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, ); FUNC(_iter_to_end)((struct ITERATOR *)_col_); }
void FUNC_PROXY(_iter_next)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, ); FUNC(_iter_next)((struct ITERATOR *)_col_); }
void FUNC_PROXY(_iter_prev)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, ); FUNC(_iter_prev)((struct ITERATOR *)_col_); }
void FUNC_PROXY(_iter_forward)(cvx_container *_col_, size_t _steps_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, ); FUNC(_iter_forward)((struct ITERATOR *)_col_, _steps_); }
void FUNC_PROXY(_iter_backward)(cvx_container *_col_, size_t _steps_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, ); FUNC(_iter_backward)((struct ITERATOR *)_col_, _steps_); }
V FUNC_PROXY(_iter_value)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, (V){ 0 }); return FUNC(_iter_value)((struct ITERATOR *)_col_); }
size_t FUNC_PROXY(_iter_index)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, 0); return FUNC(_iter_index)((struct ITERATOR *)_col_); }
struct ITERATOR FUNC_PROXY(_iter_init_start)(cvx_container *_col_) { struct ITERATOR _err_ = {0}; if (_col_->tag != TAG) { _col_->flag = CVX_FLAG_WRONG_TAG; _err_.super.flag = CVX_FLAG_WRONG_TAG; return _err_; } return FUNC(_iter_init_start)((struct SNAME *)_col_); }
struct ITERATOR FUNC_PROXY(_iter_init_end)(cvx_container *_col_) { struct ITERATOR _err_ = {0}; if (_col_->tag != TAG) { _col_->flag = CVX_FLAG_WRONG_TAG; _err_.super.flag = CVX_FLAG_WRONG_TAG; return _err_; } return FUNC(_iter_init_end)((struct SNAME *)_col_); }
// clang-format on

#ifdef IMPL_STACK
#define INTERFACE IMPL_STACK

#define IMPL_NEW FUNC_PROXY(_new)
#define IMPL_DROP FUNC_PROXY(_drop)
#define IMPL_CLONE FUNC_PROXY(_clone)
#define IMPL_PUSH FUNC_PROXY(_push_front)
#define IMPL_POP FUNC_PROXY(_pop_front)
#define IMPL_COUNT FUNC_PROXY(_count)
#define IMPL_PEEK FUNC_PROXY(_front)
#define IMPL_REPLACE FUNC_PROXY(_replace_front)

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

#define IMPL_NEW FUNC_PROXY(_new)
#define IMPL_DROP FUNC_PROXY(_drop)
#define IMPL_CLONE FUNC_PROXY(_clone)
#define IMPL_ENQUEUE FUNC_PROXY(_push_back)
#define IMPL_DEQUEUE FUNC_PROXY(_pop_front)
#define IMPL_COUNT FUNC_PROXY(_count)

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

#define IMPL_NEW FUNC_PROXY(_new)
#define IMPL_CLONE FUNC_PROXY(_clone)
#define IMPL_DROP FUNC_PROXY(_drop)
#define IMPL_PUSH_FRONT FUNC_PROXY(_push_front)
#define IMPL_PUSH_BACK FUNC_PROXY(_push_back)
#define IMPL_POP_FRONT FUNC_PROXY(_pop_front)
#define IMPL_POP_BACK FUNC_PROXY(_pop_back)
#define IMPL_PEEK_FRONT FUNC_PROXY(_front)
#define IMPL_PEEK_BACK FUNC_PROXY(_back)
#define IMPL_COUNT FUNC_PROXY(_count)

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

#define IMPL_START FUNC_PROXY(_iter_start)
#define IMPL_DROP FUNC_PROXY(_iter_drop)
#define IMPL_AT_START FUNC_PROXY(_iter_at_start)
#define IMPL_AT_END FUNC_PROXY(_iter_at_end)
#define IMPL_COUNT FUNC_PROXY(_iter_count)
#define IMPL_TO_START FUNC_PROXY(_iter_to_start)
#define IMPL_NEXT FUNC_PROXY(_iter_next)
#define IMPL_FORWARD FUNC_PROXY(_iter_forward)
#define IMPL_VALUE FUNC_PROXY(_iter_value)
#define IMPL_INDEX FUNC_PROXY(_iter_index)

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

#define IMPL_START FUNC_PROXY(_iter_start)
#define IMPL_END FUNC_PROXY(_iter_end)
#define IMPL_DROP FUNC_PROXY(_iter_drop)
#define IMPL_AT_START FUNC_PROXY(_iter_at_start)
#define IMPL_AT_END FUNC_PROXY(_iter_at_end)
#define IMPL_COUNT FUNC_PROXY(_iter_count)
#define IMPL_TO_START FUNC_PROXY(_iter_to_start)
#define IMPL_TO_END FUNC_PROXY(_iter_to_end)
#define IMPL_NEXT FUNC_PROXY(_iter_next)
#define IMPL_PREV FUNC_PROXY(_iter_prev)
#define IMPL_FORWARD FUNC_PROXY(_iter_forward)
#define IMPL_BACKWARD FUNC_PROXY(_iter_backward)
#define IMPL_VALUE FUNC_PROXY(_iter_value)
#define IMPL_INDEX FUNC_PROXY(_iter_index)

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
