/// dynamic_array.h
///
/// Status
///
///   [x] concept
///   [x] v1
///   [ ] tests
///   [ ] refine
///   [ ] stabilize
///

#include "cvx/fallback.h"

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

// Initializers and destructors
void FUNC(_init)(struct SNAME *self, struct VTAB_V *vtabv, size_t capacity);
void FUNC(_drop)(struct SNAME *self);
void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone);
// Getters
enum cvx_flags FUNC(_flag)(struct SNAME *_self_);
size_t FUNC(_count)(struct SNAME *_self_);
size_t FUNC(_capacity)(struct SNAME *_self_);
// Operations
bool FUNC(_empty)(struct SNAME *_self_);
bool FUNC(_full)(struct SNAME *_self_);
V FUNC(_front)(struct SNAME *_self_);
V FUNC(_back)(struct SNAME *_self_);
V FUNC(_get)(struct SNAME *_self_, size_t _index_);
void FUNC(_push_front)(struct SNAME *_self_, V _item_);
void FUNC(_push_at)(struct SNAME *_self_, V _item_, size_t _index_);
void FUNC(_push_back)(struct SNAME *_self_, V _item_);
V FUNC(_pop_front)(struct SNAME *_self_);
V FUNC(_pop_at)(struct SNAME *_self_, size_t _index_);
V FUNC(_pop_back)(struct SNAME *_self_);
V FUNC(_replace_front)(struct SNAME *_self_, V _new_);
V FUNC(_replace_back)(struct SNAME *_self_, V _new_);
void FUNC(_swap)(struct SNAME *_self_, size_t _idx1_, size_t _idx2_);
// Extras
int FUNC(_compare)(struct SNAME *_left_, struct SNAME *_right_);
void FUNC(_sort)(struct SNAME *_self_);

// TODO:
// void FUNC(_seq_push_front)(cvx_container *_col_, V *values, size_t size);
// void FUNC(_seq_push_back)(cvx_container *_col_, V *values, size_t size);
// cvx_contaier *FUNC(_sublist)(cvx_container *_col_, size_t from, size_t to);
// bool FUNC(_contains)(cvx_container *_col_, V item);
// void FUNC(_index_of)(cvx_container *_col_, V item, size_t *out);

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
void FUNC(_iter_go_to)(struct ITERATOR *_iter_, size_t _index_);
// Iterator access
V FUNC(_iter_value)(struct ITERATOR *_iter_);
size_t FUNC(_iter_index)(struct ITERATOR *_iter_);

// Private functions
bool FUNC(__assert_capacity)(struct SNAME *_self_);
bool FUNC(__assert_buffer)(struct SNAME *self, size_t capacity);

void FUNC(_init)(struct SNAME *self, struct VTAB_V *vtabv, size_t capacity)
{
    *self = (struct SNAME){ 0 };
    self->vtabv = vtabv;
    if (0 == capacity)
        self->buffer = NULL;
    else
    {
        if (!FUNC(__assert_buffer)(self, capacity))
            return;
    }
    self->super.tag = TAG;
    self->super.flag = CVX_FLAG_OK;
}

void FUNC(_drop)(struct SNAME *self)
{
    if (!self)
        return;
    if (self->buffer)
    {
        if (self->vtabv && self->vtabv->drop)
        {
            for (size_t i = 0; i < self->count; i++)
                self->vtabv->drop(self->buffer[i]);
        }
        free(self->buffer);
        self->buffer = NULL;
    }
}

void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone)
{
    FUNC(_init)(clone, orig->vtabv, orig->capacity);
    orig->super.flag = CVX_FLAG_OK;
    if (!orig->buffer || !clone->buffer)
        return;
    if (clone->vtabv && clone->vtabv->clone)
    {
        for (size_t i = 0; i < orig->count; i++)
            clone->buffer[i] = clone->vtabv->clone(orig->buffer[i]);
    }
    else
    {
        memcpy(clone->buffer, orig->buffer, orig->count * sizeof(V));
    }
    clone->count = orig->count;
}

enum cvx_flags FUNC(_flag)(struct SNAME *_self_)
{
    return _self_->super.flag;
}

size_t FUNC(_count)(struct SNAME *_self_)
{
    return _self_->count;
}

size_t FUNC(_capacity)(struct SNAME *_self_)
{
    return _self_->capacity;
}

bool FUNC(_empty)(struct SNAME *_self_)
{
    return _self_->count == 0;
}

bool FUNC(_full)(struct SNAME *_self_)
{
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

struct ITERATOR FUNC(_iter_init_start)(struct SNAME *_target_)
{
    struct ITERATOR _res_ = { 0 };

    _res_.super.tag = ITER_TAG;
    _res_.target = _target_;
    _res_.index = 0;
    _res_.super.flag = CVX_FLAG_OK;

    return _res_;
}

struct ITERATOR FUNC(_iter_init_end)(struct SNAME *_target_)
{
    struct ITERATOR _res_ = { 0 };

    _res_.super.tag = ITER_TAG;
    _res_.target = _target_;
    _res_.index = _target_->count;
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
    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_to_end)(struct ITERATOR *_iter_)
{
    _iter_->index = _iter_->target->count;
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
    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_forward)(struct ITERATOR *_iter_, size_t _steps_)
{
    size_t _remaining_ = _iter_->target->count - _iter_->index;
    _iter_->index += (_steps_ < _remaining_) ? _steps_ : _remaining_;
    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_backward)(struct ITERATOR *_iter_, size_t _steps_)
{
    _iter_->index -= (_steps_ < _iter_->index) ? _steps_ : _iter_->index;
    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_go_to)(struct ITERATOR *_iter_, size_t _index_)
{
    if (_index_ > _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return;
    }

    _iter_->index = _index_;
    _iter_->super.flag = CVX_FLAG_OK;
}

// Iterator access
V FUNC(_iter_value)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->target->buffer[_iter_->index];
}

size_t FUNC(_iter_index)(struct ITERATOR *_iter_)
{
    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->index;
}

///
///
/// PRIVATE FUNCTIONS
///
///

bool FUNC(__assert_capacity)(struct SNAME *self)
{
    if (self->count < self->capacity)
        return true;
    size_t capacity = (size_t)(self->capacity * (CVX_BUFFER_GROWTH_RATE));
    if (capacity < CVX_BUFFER_MIN_SIZE)
        capacity = CVX_BUFFER_MIN_SIZE;
    return FUNC(__assert_buffer)(self, capacity);
}

bool FUNC(__assert_buffer)(struct SNAME *self, size_t capacity)
{
    if (!self->buffer)
    {
        self->buffer = malloc(sizeof(V) * capacity);
        if (!self->buffer)
        {
            self->super.flag = CVX_FLAG_ALLOC;
            return false;
        }
    }
    else
    {
        V *new_buffer = realloc(self->buffer, sizeof(V) * capacity);
        if (!new_buffer)
        {
            self->super.flag = CVX_FLAG_ALLOC;
            return false;
        }
        self->buffer = new_buffer;
    }
    self->capacity = capacity;
    return true;
}

#include "cvx/undef.h"
