// clang-format off
#ifndef V
#error "cvx/dynamic_array.h requires V to be defined (the element type, e.g. #define V int)"
#endif
#ifndef SNAME
#error "cvx/dynamic_array.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_array)"
#endif
#ifndef PFX
#error "cvx/dynamic_array.h requires PFX to be defined (the function prefix, e.g. #define PFX ma)"
#endif
#ifndef TAG
#error "cvx/dynamic_array.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
#endif
// clang-format on

#include <stdlib.h>
#include <string.h>

#include "cvx/core.h"

#define FUNC(X) CVX_(PFX, X)
#define FUNC_PROXY(X) CVX_(PFX, CVX_(__proxy, X))
#define ITERATOR CVX_(SNAME, _iter)
#define ITER_TAG (TAG * CVX_ITER_TAG_MULT)
#define VTAB_V CVX_(SNAME, _vtabv)

struct VTAB_V
{
    CVX_VTAB_DEFINITION(V)
};

struct SNAME
{
    cvx_container super;
    size_t capacity;
    size_t count;
    struct VTAB_V *vtabv;
    V *buffer;
};

struct ITERATOR
{
    cvx_container super;
    size_t index;
    struct SNAME *target;
};

// Non-allocating initializers
struct SNAME FUNC(_init)(struct VTAB_V *_vtabv_);
struct SNAME FUNC(_init_with)(struct VTAB_V *_vtabv_, size_t capacity);
struct SNAME FUNC(_copy)(struct SNAME *_self_);

// Allocating initializers
struct SNAME *FUNC(_new)(void);
struct SNAME *FUNC(_new_with)(struct VTAB_V *_vtabv_, size_t capacity);
struct SNAME *FUNC(_clone)(struct SNAME *_orig_);

// Destructors
void FUNC(_drop)(struct SNAME *_self_);
void FUNC(_clear)(struct SNAME *_self_);

// Getters
enum cvx_flags FUNC(_flag)(struct SNAME *_self_);
size_t FUNC(_count)(struct SNAME *_self_);
size_t FUNC(_capacity)(struct SNAME *_self_);
bool FUNC(_empty)(struct SNAME *_self_);
bool FUNC(_full)(struct SNAME *_self_);
V FUNC(_front)(struct SNAME *_self_);
V FUNC(_back)(struct SNAME *_self_);
V FUNC(_get)(struct SNAME *_self_, size_t _index_);

// Operations
void FUNC(_push_front)(struct SNAME *_self_, V _item_);
void FUNC(_push_at)(struct SNAME *_self_, V _item_, size_t _index_);
void FUNC(_push_back)(struct SNAME *_self_, V _item_);
V FUNC(_pop_front)(struct SNAME *_self_);
V FUNC(_pop_at)(struct SNAME *_self_, size_t _index_);
V FUNC(_pop_back)(struct SNAME *_self_);
V FUNC(_replace_front)(struct SNAME *_self_, V _new_);
V FUNC(_replace_back)(struct SNAME *_self_, V _new_);

void FUNC(_swap)(struct SNAME *_self_, size_t _idx1_, size_t _idx2_);
int FUNC(_compare)(struct SNAME *_left_, struct SNAME *_right_);
void FUNC(_sort)(struct SNAME *_self_);

// TODO:
// void FUNC(_seq_push_front)(cvx_container *_col_, V *values, size_t size);
// void FUNC(_seq_push_back)(cvx_container *_col_, V *values, size_t size);
// cvx_contaier *FUNC(_sublist)(cvx_container *_col_, size_t from, size_t to);
// bool FUNC(_contains)(cvx_container *_col_, V item);
// void FUNC(_index_of)(cvx_container *_col_, V item, size_t *out);

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
void FUNC(_iter_go_to)(cvx_container *_iter_, size_t _index_);
// Iterator access
V FUNC(_iter_value)(cvx_container *_iter_);
size_t FUNC(_iter_index)(cvx_container *_iter_);

// Private functions
bool FUNC(__assert_capacity)(struct SNAME *_self_);

struct SNAME FUNC(_init)(struct VTAB_V *_vtabv_)
{
    struct SNAME _res_ = (struct SNAME){ 0 };
    _res_.super.tag = TAG;
    _res_.vtabv = _vtabv_;

    return _res_;
}

struct SNAME FUNC(_init_with)(struct VTAB_V *_vtabv_, size_t capacity)
{
    struct SNAME _res_ = (struct SNAME){ 0 };
    _res_.super.tag = TAG;

    _res_.buffer = calloc(capacity, sizeof(V));

    if (!_res_.buffer)
    {
        _res_.super.flag = CVX_FLAG_ALLOC;
        return _res_;
    }

    _res_.capacity = capacity;
    _res_.vtabv = _vtabv_;

    return _res_;
}

struct SNAME FUNC(_copy)(struct SNAME *_self_)
{
    struct SNAME _res_ = FUNC(_init)(_self_->vtabv);
    _res_.super.flag = CVX_FLAG_OK;

    if (_self_->count == 0)
        return _res_;

    _res_.buffer = malloc(_self_->capacity * sizeof(V));
    if (!_res_.buffer)
    {
        _res_.super.flag = CVX_FLAG_ALLOC;
        return _res_;
    }

    _res_.capacity = _self_->capacity;
    _res_.count = _self_->count;

    if (_self_->vtabv && _self_->vtabv->copy)
    {
        for (size_t i = 0; i < _self_->count; i++)
            _res_.buffer[i] = _self_->vtabv->copy(_self_->buffer[i]);
    }
    else
    {
        memcpy(_res_.buffer, _self_->buffer, _self_->count * sizeof(V));
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
    _res_->capacity = 0;
    _res_->count = 0;
    _res_->buffer = NULL;
    _res_->vtabv = NULL;

    return _res_;
}

struct SNAME *FUNC(_new_with)(struct VTAB_V *_vtabv_, size_t capacity)
{
    if (capacity == 0)
        return NULL;

    struct SNAME *_res_ = malloc(sizeof(struct SNAME));

    if (!_res_)
        return NULL;

    _res_->buffer = calloc(capacity, sizeof(V));

    if (!_res_->buffer)
    {
        free(_res_);
        return NULL;
    }

    _res_->super.tag = TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->count = 0;
    _res_->capacity = capacity;
    _res_->vtabv = _vtabv_;

    return _res_;
}

struct SNAME *FUNC(_clone)(struct SNAME *_orig_)
{
    struct SNAME *_res_ = FUNC(_new)();

    if (!_res_)
        return NULL;

    struct SNAME *_copy_ = (struct SNAME *)_res_;
    _copy_->vtabv = _orig_->vtabv;

    if (_orig_->count == 0)
    {
        _res_->super.flag = CVX_FLAG_OK;
        return _res_;
    }

    _copy_->buffer = malloc(_orig_->capacity * sizeof(V));
    if (!_copy_->buffer)
    {
        free(_copy_);
        return NULL;
    }

    _copy_->capacity = _orig_->capacity;
    _copy_->count = _orig_->count;

    if (_copy_->vtabv && _copy_->vtabv->copy)
    {
        for (size_t i = 0; i < _orig_->count; i++)
            _copy_->buffer[i] = _copy_->vtabv->copy(_orig_->buffer[i]);
    }
    else
    {
        memcpy(_copy_->buffer, _orig_->buffer, _orig_->count * sizeof(V));
    }

    _res_->super.flag = CVX_FLAG_OK;
    return _res_;
}

void FUNC(_drop)(struct SNAME *_self_)
{
    if (_self_->vtabv && _self_->vtabv->drop)
    {
        for (size_t i = 0; i < _self_->count; i++)
            _self_->vtabv->drop(_self_->buffer[i]);
    }

    free(_self_->buffer);
    free(_self_);
}

void FUNC(_clear)(struct SNAME *_self_)
{
    if (_self_->vtabv && _self_->vtabv->drop)
    {
        for (size_t i = 0; i < _self_->count; i++)
        {
            _self_->vtabv->drop(_self_->buffer[i]);
        }
    }

    free(_self_->buffer);
    _self_->buffer = NULL;
    _self_->capacity = 0;
    _self_->count = 0;
    _self_->super.flag = CVX_FLAG_OK;
}

enum cvx_flags FUNC(_flag)(struct SNAME *_self_)
{
    return _self_->super.flag;
}

size_t FUNC(_count)(struct SNAME *_self_)
{
    _self_->super.flag = CVX_FLAG_OK;
    return _self_->count;
}

size_t FUNC(_capacity)(struct SNAME *_self_)
{
    _self_->super.flag = CVX_FLAG_OK;
    return _self_->capacity;
}

bool FUNC(_empty)(struct SNAME *_self_)
{
    _self_->super.flag = CVX_FLAG_OK;
    return _self_->count == 0;
}

bool FUNC(_full)(struct SNAME *_self_)
{
    _self_->super.flag = CVX_FLAG_OK;
    return _self_->count >= _self_->capacity;
}

V FUNC(_front)(struct SNAME *_self_)
{
    if (_self_->count == 0 || _self_->buffer == NULL)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    _self_->super.flag = CVX_FLAG_OK;
    return _self_->buffer[0];
}

V FUNC(_back)(struct SNAME *_self_)
{
    if (_self_->count == 0 || _self_->buffer == NULL)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    _self_->super.flag = CVX_FLAG_OK;
    return _self_->buffer[_self_->count - 1];
}

V FUNC(_get)(struct SNAME *_self_, size_t _index_)
{
    if (_index_ >= _self_->count)
    {
        _self_->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    _self_->super.flag = CVX_FLAG_OK;
    return _self_->buffer[_index_];
}

void FUNC(_push_front)(struct SNAME *_self_, V _item_)
{
    if (!FUNC(__assert_capacity)(_self_))
        return;

    if (_self_->count > 0)
    {
        memmove(_self_->buffer + 1, _self_->buffer, _self_->count * sizeof(V));
    }

    _self_->buffer[0] = _item_;
    _self_->count++;
}

void FUNC(_push_at)(struct SNAME *_self_, V _item_, size_t _index_)
{
    if (_index_ > _self_->count)
    {
        _self_->super.flag = CVX_FLAG_RANGE;
        return;
    }

    if (!FUNC(__assert_capacity)(_self_))
        return;

    memmove(_self_->buffer + _index_ + 1, _self_->buffer + _index_,
            (_self_->count - _index_) * sizeof(V));

    _self_->buffer[_index_] = _item_;
    _self_->count++;
    _self_->super.flag = CVX_FLAG_OK;
}

void FUNC(_push_back)(struct SNAME *_self_, V _item_)
{
    if (!FUNC(__assert_capacity)(_self_))
        return;

    _self_->buffer[_self_->count++] = _item_;
}

V FUNC(_pop_front)(struct SNAME *_self_)
{
    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _val_ = _self_->buffer[0];

    memmove(_self_->buffer, _self_->buffer + 1, (_self_->count - 1) * sizeof(V));

    _self_->buffer[_self_->count - 1] = (V){ 0 };
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

    V _val_ = _self_->buffer[_index_];

    memmove(_self_->buffer + _index_, _self_->buffer + _index_ + 1,
            (_self_->count - _index_ - 1) * sizeof(V));

    _self_->buffer[_self_->count - 1] = (V){ 0 };
    _self_->count--;
    _self_->super.flag = CVX_FLAG_OK;

    return _val_;
}

V FUNC(_pop_back)(struct SNAME *_self_)
{
    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _val_ = _self_->buffer[_self_->count - 1];

    _self_->buffer[_self_->count - 1] = (V){ 0 };
    _self_->count--;
    _self_->super.flag = CVX_FLAG_OK;

    return _val_;
}

V FUNC(_replace_front)(struct SNAME *_self_, V _new_)
{
    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = _self_->buffer[0];
    _self_->buffer[0] = _new_;
    _self_->super.flag = CVX_FLAG_OK;

    return _old_;
}

V FUNC(_replace_back)(struct SNAME *_self_, V _new_)
{
    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = _self_->buffer[_self_->count - 1];
    _self_->buffer[_self_->count - 1] = _new_;
    _self_->super.flag = CVX_FLAG_OK;

    return _old_;
}

int FUNC(_compare)(struct SNAME *_left_, struct SNAME *_right_)
{
    CVX_VTAB_COMP(cmp_func, V) = NULL;

    if (_left_->vtabv && _left_->vtabv->comp)
        cmp_func = _left_->vtabv->comp;
    else if (_right_->vtabv && _right_->vtabv->comp)
        cmp_func = _right_->vtabv->comp;
    else
    {
        _left_->super.flag = CVX_FLAG_VTAB;
        _right_->super.flag = CVX_FLAG_VTAB;
        return 0;
    }

    size_t _min_ = _left_->count < _right_->count ? _left_->count : _right_->count;

    for (size_t i = 0; i < _min_; i++)
    {
        size_t cmp = cmp_func(_left_->buffer[i], _right_->buffer[i]);
        if (cmp != 0)
            return cmp;
    }

    return _left_->count - _right_->count;
}

void FUNC(_sort)(struct SNAME *_self_)
{
    if (!_self_->vtabv || !_self_->vtabv->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return;
    }

    if (_self_->count <= 1)
        return;

    // TODO: optimize
    for (size_t i = 0; i < _self_->count - 1; i++)
    {
        for (size_t j = 0; j < _self_->count - 1 - i; j++)
        {
            if (_self_->vtabv->comp(_self_->buffer[j], _self_->buffer[j + 1]) > 0)
                FUNC(_swap)(_self_, j, j + 1);
        }
    }
}

void FUNC(_swap)(struct SNAME *_self_, size_t _idx1_, size_t _idx2_)
{
    if (_idx1_ > _self_->count || _idx2_ > _self_->count)
    {
        _self_->super.flag = CVX_FLAG_RANGE;
        return;
    }

    if (_idx1_ == _idx2_)
        return;

    V _tmp_ = _self_->buffer[_idx1_];
    _self_->buffer[_idx1_] = _self_->buffer[_idx2_];
    _self_->buffer[_idx2_] = _tmp_;
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

    _res_.super.tag = ITER_TAG;
    _res_.target = (struct SNAME *)_target_;
    _res_.index = 0;
    _res_.super.flag = CVX_FLAG_OK;

    return _res_;
}

struct ITERATOR FUNC(_iter_init_end)(cvx_container *_target_)
{
    CVX_CONTAINER_GUARDS(TAG, _target_, (struct ITERATOR){ 0 });

    struct ITERATOR _res_ = { 0 };

    _res_.super.tag = ITER_TAG;
    _res_.target = (struct SNAME *)_target_;
    _res_.index = _res_.target->count;
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
    _res_->index = _res_->target->count;

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
    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_to_end)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    _self_->index = _self_->target->count;
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
    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_forward)(cvx_container *_iter_, size_t _steps_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    size_t _remaining_ = _self_->target->count - _self_->index;
    _self_->index += (_steps_ < _remaining_) ? _steps_ : _remaining_;
    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_backward)(cvx_container *_iter_, size_t _steps_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    _self_->index -= (_steps_ < _self_->index) ? _steps_ : _self_->index;
    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_go_to)(cvx_container *_iter_, size_t _index_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    if (_index_ > _self_->target->count)
    {
        _iter_->flag = CVX_FLAG_RANGE;
        return;
    }

    _self_->index = _index_;
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

    _iter_->flag = CVX_FLAG_OK;
    return _self_->target->buffer[_self_->index];
}

size_t FUNC(_iter_index)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, 0);

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    _iter_->flag = CVX_FLAG_OK;
    return _self_->index;
}

///
///
/// PRIVATE FUNCTIONS
///
///

bool FUNC(__assert_capacity)(struct SNAME *_self_)
{
    if (_self_->count < _self_->capacity)
        return true;

    if (_self_->capacity == 0)
        _self_->capacity = 16;

    if (!_self_->buffer)
    {
        // First allocation
        _self_->buffer = malloc(sizeof(V) * _self_->capacity);
        if (!_self_->buffer)
        {
            _self_->super.flag = CVX_FLAG_ALLOC;
            return false;
        }
    }
    else
    {
        // Reallocation
        size_t _old_capacity_ = _self_->capacity;
        _self_->capacity *= 2;
        V *new_buffer = realloc(_self_->buffer, sizeof(V) * _self_->capacity);

        if (!new_buffer)
        {
            _self_->capacity = _old_capacity_;
            _self_->super.flag = CVX_FLAG_ALLOC;
            return false;
        }

        _self_->buffer = new_buffer;
    }

    return true;
}

///
///
/// PROXIES
///
///

// clang-format off
cvx_container *FUNC_PROXY(_new)(void) { return (cvx_container *)FUNC(_new)(); }
cvx_container *FUNC_PROXY(_clone)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, NULL); return (cvx_container *)FUNC(_clone)((struct SNAME *)_col_); }
void FUNC_PROXY(_drop)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_drop)((struct SNAME *)_col_); }
void FUNC_PROXY(_clear)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_clear)((struct SNAME *)_col_); }
enum cvx_flags FUNC_PROXY(_flag)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, CVX_FLAG_WRONG_TAG); return FUNC(_flag)((struct SNAME *)_col_); }
size_t FUNC_PROXY(_count)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, 0); return FUNC(_count)((struct SNAME *)_col_); }
size_t FUNC_PROXY(_capacity)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, 0); return FUNC(_capacity)((struct SNAME *)_col_); }
bool FUNC_PROXY(_empty)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_empty)((struct SNAME *)_col_); }
bool FUNC_PROXY(_full)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_full)((struct SNAME *)_col_); }
V FUNC_PROXY(_front)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_front)((struct SNAME *)_col_); }
V FUNC_PROXY(_back)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_back)((struct SNAME *)_col_); }
V FUNC_PROXY(_get)(cvx_container *_col_, size_t _index_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_get)((struct SNAME *)_col_, _index_); }
struct SNAME FUNC_PROXY(_copy)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (struct SNAME){ 0 }); return FUNC(_copy)((struct SNAME *)_col_); }
void FUNC_PROXY(_push_front)(cvx_container *_col_, V _item_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_push_front)((struct SNAME *)_col_, _item_); }
void FUNC_PROXY(_push_at)(cvx_container *_col_, V _item_, size_t _index_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_push_at)((struct SNAME *)_col_, _item_, _index_); }
void FUNC_PROXY(_push_back)(cvx_container *_col_, V _item_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_push_back)((struct SNAME *)_col_, _item_); }
V FUNC_PROXY(_pop_front)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_pop_front)((struct SNAME *)_col_); }
V FUNC_PROXY(_pop_at)(cvx_container *_col_, size_t _index_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_pop_at)((struct SNAME *)_col_, _index_); }
V FUNC_PROXY(_pop_back)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_pop_back)((struct SNAME *)_col_); }
V FUNC_PROXY(_replace_front)(cvx_container *_col_, V _new_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_replace_front)((struct SNAME *)_col_, _new_); }
V FUNC_PROXY(_replace_back)(cvx_container *_col_, V _new_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_replace_back)((struct SNAME *)_col_, _new_); }
void FUNC_PROXY(_swap)(cvx_container *_col_, size_t _idx1_, size_t _idx2_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_swap)((struct SNAME *)_col_, _idx1_, _idx2_); }
int FUNC_PROXY(_compare)(cvx_container *_col_, struct SNAME *_right_) { CVX_CONTAINER_GUARDS(TAG, _col_, 0); return FUNC(_compare)((struct SNAME *)_col_, _right_); }
void FUNC_PROXY(_sort)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_sort)((struct SNAME *)_col_); }
// clang-format on

#ifdef IMPL_STACK
#define INTERFACE IMPL_STACK

#define IMPL_NEW FUNC_PROXY(_new)
#define IMPL_DROP FUNC_PROXY(_drop)
#define IMPL_CLONE FUNC_PROXY(_clone)
#define IMPL_PUSH FUNC_PROXY(_push_back)
#define IMPL_POP FUNC_PROXY(_pop_back)
#define IMPL_COUNT FUNC_PROXY(_count)
#define IMPL_PEEK FUNC_PROXY(_back)
#define IMPL_REPLACE FUNC_PROXY(_replace_back)

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

#ifdef IMPL_RANDOM_ACCESS_ITER
#define INTERFACE IMPL_RANDOM_ACCESS_ITER

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
#define IMPL_GO_TO FUNC(_iter_go_to)
#define IMPL_VALUE FUNC(_iter_value)
#define IMPL_INDEX FUNC(_iter_index)

#include "cvx/iter/random_access_iterator_cast.h"
#undef IMPL_RANDOM_ACCESS_ITER

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
#undef IMPL_GO_TO
#undef IMPL_VALUE
#undef IMPL_INDEX
#endif

#include "cvx/undef.h"
