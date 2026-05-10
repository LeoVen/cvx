#include "cvx/fallback.h"

// clang-format off
#ifndef V
#error "cvx/circular_buffer.h requires V to be defined (the element type, e.g. #define V int)"
#endif
#ifndef SNAME
#error "cvx/circular_buffer.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_cbuf)"
#endif
#ifndef PFX
#error "cvx/circular_buffer.h requires PFX to be defined (the function prefix, e.g. #define PFX cb)"
#endif
#ifndef TAG
#error "cvx/circular_buffer.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
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
    size_t head;
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
struct SNAME FUNC(_init_with)(struct VTAB_V *_vtabv_, size_t _capacity_);
struct SNAME FUNC(_copy)(struct SNAME *_self_);

// Allocating initializers
struct SNAME *FUNC(_new)(void);
struct SNAME *FUNC(_new_with)(struct VTAB_V *_vtabv_, size_t _capacity_);
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
bool FUNC(_is_linearized)(struct SNAME *_self_);

// Operations
V FUNC(_front)(struct SNAME *_self_);
V FUNC(_back)(struct SNAME *_self_);
V FUNC(_get)(struct SNAME *_self_, size_t _index_);
void FUNC(_push_back)(struct SNAME *_self_, V _item_);
void FUNC(_push_front)(struct SNAME *_self_, V _item_);
V FUNC(_pop_back)(struct SNAME *_self_);
V FUNC(_pop_front)(struct SNAME *_self_);
void FUNC(_set_capacity)(struct SNAME *_self_, size_t _new_cap_);
void FUNC(_linearize)(struct SNAME *_self_);

// Iterators
struct ITERATOR FUNC(_iter_init_start)(struct SNAME *_target_);
struct ITERATOR FUNC(_iter_init_end)(struct SNAME *_target_);
struct ITERATOR *FUNC(_iter_start)(struct SNAME *_target_);
struct ITERATOR *FUNC(_iter_end)(struct SNAME *_target_);
void FUNC(_iter_drop)(struct ITERATOR *_iter_);
bool FUNC(_iter_at_start)(struct ITERATOR *_iter_);
bool FUNC(_iter_at_end)(struct ITERATOR *_iter_);
size_t FUNC(_iter_count)(struct ITERATOR *_iter_);
void FUNC(_iter_to_start)(struct ITERATOR *_iter_);
void FUNC(_iter_to_end)(struct ITERATOR *_iter_);
void FUNC(_iter_next)(struct ITERATOR *_iter_);
void FUNC(_iter_prev)(struct ITERATOR *_iter_);
void FUNC(_iter_forward)(struct ITERATOR *_iter_, size_t _steps_);
void FUNC(_iter_backward)(struct ITERATOR *_iter_, size_t _steps_);
void FUNC(_iter_go_to)(struct ITERATOR *_iter_, size_t _index_);
V FUNC(_iter_value)(struct ITERATOR *_iter_);
size_t FUNC(_iter_index)(struct ITERATOR *_iter_);

/// Initialize a Circular Buffer (stack-allocated, no internal buffer).
struct SNAME FUNC(_init)(struct VTAB_V *_vtabv_)
{
    struct SNAME _res_ = (struct SNAME){ 0 };
    _res_.super.tag = TAG;
    _res_.vtabv = _vtabv_;

    return _res_;
}

/// Initialize a Circular Buffer with a pre-allocated buffer of the given capacity.
struct SNAME FUNC(_init_with)(struct VTAB_V *_vtabv_, size_t _capacity_)
{
    struct SNAME _res_ = (struct SNAME){ 0 };
    _res_.super.tag = TAG;
    _res_.vtabv = _vtabv_;

    if (_capacity_ == 0)
        return _res_;

    _res_.buffer = calloc(_capacity_, sizeof(V));

    if (!_res_.buffer)
    {
        _res_.super.flag = CVX_FLAG_ALLOC;
        return _res_;
    }

    _res_.capacity = _capacity_;

    return _res_;
}

/// Create a deep copy of the buffer by value.
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
    _res_.head = 0;

    if (_self_->vtabv && _self_->vtabv->clone)
    {
        for (size_t i = 0; i < _self_->count; i++)
        {
            size_t _phys_ = (_self_->head + i) % _self_->capacity;
            _res_.buffer[i] = _self_->vtabv->clone(_self_->buffer[_phys_]);
        }
    }
    else
    {
        for (size_t i = 0; i < _self_->count; i++)
        {
            size_t _phys_ = (_self_->head + i) % _self_->capacity;
            _res_.buffer[i] = _self_->buffer[_phys_];
        }
    }

    return _res_;
}

/// Heap-allocate a Circular Buffer with capacity=0 and no vtabv.
struct SNAME *FUNC(_new)(void)
{
    struct SNAME *_res_ = malloc(sizeof(struct SNAME));

    if (!_res_)
        return NULL;

    _res_->super.tag = TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->capacity = 0;
    _res_->count = 0;
    _res_->head = 0;
    _res_->vtabv = NULL;
    _res_->buffer = NULL;

    return _res_;
}

/// Heap-allocate a Circular Buffer with a pre-allocated buffer.
struct SNAME *FUNC(_new_with)(struct VTAB_V *_vtabv_, size_t _capacity_)
{
    struct SNAME *_res_ = malloc(sizeof(struct SNAME));

    if (!_res_)
        return NULL;

    _res_->super.tag = TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->count = 0;
    _res_->head = 0;
    _res_->vtabv = _vtabv_;

    if (_capacity_ == 0)
    {
        _res_->capacity = 0;
        _res_->buffer = NULL;
        return _res_;
    }

    _res_->buffer = calloc(_capacity_, sizeof(V));

    if (!_res_->buffer)
    {
        free(_res_);
        return NULL;
    }

    _res_->capacity = _capacity_;

    return _res_;
}

/// Heap-allocate a deep copy of the buffer.
struct SNAME *FUNC(_clone)(struct SNAME *_orig_)
{
    struct SNAME *_res_ = malloc(sizeof(struct SNAME));

    if (!_res_)
        return NULL;

    _res_->super.tag = TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->vtabv = _orig_->vtabv;
    _res_->count = 0;
    _res_->head = 0;
    _res_->capacity = 0;
    _res_->buffer = NULL;

    if (_orig_->count == 0)
        return _res_;

    _res_->buffer = malloc(_orig_->capacity * sizeof(V));
    if (!_res_->buffer)
    {
        free(_res_);
        return NULL;
    }

    _res_->capacity = _orig_->capacity;
    _res_->count = _orig_->count;

    if (_res_->vtabv && _res_->vtabv->clone)
    {
        for (size_t i = 0; i < _orig_->count; i++)
        {
            size_t _phys_ = (_orig_->head + i) % _orig_->capacity;
            _res_->buffer[i] = _res_->vtabv->clone(_orig_->buffer[_phys_]);
        }
    }
    else
    {
        for (size_t i = 0; i < _orig_->count; i++)
        {
            size_t _phys_ = (_orig_->head + i) % _orig_->capacity;
            _res_->buffer[i] = _orig_->buffer[_phys_];
        }
    }

    return _res_;
}

/// Drop all elements and free the buffer and the struct pointer.
void FUNC(_drop)(struct SNAME *_self_)
{
    if (_self_->vtabv && _self_->vtabv->drop)
    {
        for (size_t i = 0; i < _self_->count; i++)
            _self_->vtabv->drop(_self_->buffer[(_self_->head + i) % _self_->capacity]);
    }

    free(_self_->buffer);
    free(_self_);
}

/// Drop all elements and free the buffer; the struct remains valid and empty.
void FUNC(_clear)(struct SNAME *_self_)
{
    if (_self_->vtabv && _self_->vtabv->drop)
    {
        for (size_t i = 0; i < _self_->count; i++)
            _self_->vtabv->drop(_self_->buffer[(_self_->head + i) % _self_->capacity]);
    }

    free(_self_->buffer);
    _self_->buffer = NULL;
    _self_->capacity = 0;
    _self_->count = 0;
    _self_->head = 0;
    _self_->super.flag = CVX_FLAG_OK;
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
    return _self_->count == _self_->capacity;
}

bool FUNC(_is_linearized)(struct SNAME *_self_)
{
    return _self_->count == 0 || _self_->head + _self_->count <= _self_->capacity;
}

V FUNC(_front)(struct SNAME *_self_)
{
    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    _self_->super.flag = CVX_FLAG_OK;
    return _self_->buffer[_self_->head];
}

V FUNC(_back)(struct SNAME *_self_)
{
    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    _self_->super.flag = CVX_FLAG_OK;
    return _self_->buffer[(_self_->head + _self_->count - 1) % _self_->capacity];
}

V FUNC(_get)(struct SNAME *_self_, size_t _index_)
{
    if (_index_ >= _self_->count)
    {
        _self_->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    _self_->super.flag = CVX_FLAG_OK;
    return _self_->buffer[(_self_->head + _index_) % _self_->capacity];
}

/// Push an element to the back. When full, overwrites the front element.
void FUNC(_push_back)(struct SNAME *_self_, V _item_)
{
    if (_self_->capacity == 0)
    {
        _self_->super.flag = CVX_FLAG_FULL;
        return;
    }

    if (_self_->count < _self_->capacity)
    {
        _self_->buffer[(_self_->head + _self_->count) % _self_->capacity] = _item_;
        _self_->count++;
    }
    else
    {
        // Full: overwrite front (head)
        if (_self_->vtabv && _self_->vtabv->drop)
            _self_->vtabv->drop(_self_->buffer[_self_->head]);

        _self_->buffer[_self_->head] = _item_;
        _self_->head = (_self_->head + 1) % _self_->capacity;
    }

    _self_->super.flag = CVX_FLAG_OK;
}

/// Push an element to the front. When full, overwrites the back element.
void FUNC(_push_front)(struct SNAME *_self_, V _item_)
{
    if (_self_->capacity == 0)
    {
        _self_->super.flag = CVX_FLAG_FULL;
        return;
    }

    if (_self_->count < _self_->capacity)
    {
        _self_->head = (_self_->head + _self_->capacity - 1) % _self_->capacity;
        _self_->buffer[_self_->head] = _item_;
        _self_->count++;
    }
    else
    {
        // Full: overwrite back
        _self_->head = (_self_->head + _self_->capacity - 1) % _self_->capacity;

        if (_self_->vtabv && _self_->vtabv->drop)
            _self_->vtabv->drop(_self_->buffer[_self_->head]);

        _self_->buffer[_self_->head] = _item_;
    }

    _self_->super.flag = CVX_FLAG_OK;
}

/// Remove and return the back element.
V FUNC(_pop_back)(struct SNAME *_self_)
{
    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    _self_->count--;
    size_t _phys_ = (_self_->head + _self_->count) % _self_->capacity;
    V _val_ = _self_->buffer[_phys_];
    _self_->buffer[_phys_] = (V){ 0 };
    _self_->super.flag = CVX_FLAG_OK;

    return _val_;
}

/// Remove and return the front element.
V FUNC(_pop_front)(struct SNAME *_self_)
{
    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _val_ = _self_->buffer[_self_->head];
    _self_->buffer[_self_->head] = (V){ 0 };
    _self_->head = (_self_->head + 1) % _self_->capacity;
    _self_->count--;
    _self_->super.flag = CVX_FLAG_OK;

    return _val_;
}

/// Resize the buffer. If shrinking below count, drops the excess front elements.
void FUNC(_set_capacity)(struct SNAME *_self_, size_t _new_cap_)
{
    if (_new_cap_ == 0)
    {
        if (_self_->vtabv && _self_->vtabv->drop)
        {
            for (size_t i = 0; i < _self_->count; i++)
                _self_->vtabv->drop(_self_->buffer[(_self_->head + i) % _self_->capacity]);
        }
        free(_self_->buffer);
        _self_->buffer = NULL;
        _self_->capacity = 0;
        _self_->count = 0;
        _self_->head = 0;
        _self_->super.flag = CVX_FLAG_OK;
        return;
    }

    // Drop excess front elements if shrinking
    while (_self_->count > _new_cap_)
    {
        if (_self_->vtabv && _self_->vtabv->drop)
            _self_->vtabv->drop(_self_->buffer[_self_->head]);
        _self_->buffer[_self_->head] = (V){ 0 };
        _self_->head = (_self_->head + 1) % _self_->capacity;
        _self_->count--;
    }

    V *_new_buf_ = malloc(_new_cap_ * sizeof(V));
    if (!_new_buf_)
    {
        _self_->super.flag = CVX_FLAG_ALLOC;
        return;
    }

    // Copy elements in logical order to the new buffer
    for (size_t i = 0; i < _self_->count; i++)
    {
        size_t _phys_ = (_self_->head + i) % _self_->capacity;
        _new_buf_[i] = _self_->buffer[_phys_];
    }

    free(_self_->buffer);
    _self_->buffer = _new_buf_;
    _self_->head = 0;
    _self_->capacity = _new_cap_;
    _self_->super.flag = CVX_FLAG_OK;
}

/// Rearrange the internal buffer so elements are contiguous starting at index 0.
void FUNC(_linearize)(struct SNAME *_self_)
{
    if (_self_->count == 0 || _self_->head + _self_->count <= _self_->capacity)
    {
        _self_->super.flag = CVX_FLAG_OK;
        return;
    }

    V *_tmp_ = malloc(_self_->capacity * sizeof(V));
    if (!_tmp_)
    {
        _self_->super.flag = CVX_FLAG_ALLOC;
        return;
    }

    for (size_t i = 0; i < _self_->count; i++)
    {
        size_t _phys_ = (_self_->head + i) % _self_->capacity;
        _tmp_[i] = _self_->buffer[_phys_];
    }

    free(_self_->buffer);
    _self_->buffer = _tmp_;
    _self_->head = 0;
    _self_->super.flag = CVX_FLAG_OK;
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
    _res_.super.flag = CVX_FLAG_OK;
    _res_.index = 0;
    _res_.target = _target_;

    return _res_;
}

struct ITERATOR FUNC(_iter_init_end)(struct SNAME *_target_)
{
    struct ITERATOR _res_ = { 0 };
    _res_.super.tag = ITER_TAG;
    _res_.super.flag = CVX_FLAG_OK;
    _res_.index = _target_->count;
    _res_.target = _target_;

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
    _res_->index = _target_->count;
    _res_->target = _target_;

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

V FUNC(_iter_value)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count || _iter_->target->capacity == 0)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    _iter_->super.flag = CVX_FLAG_OK;
    size_t _phys_ = (_iter_->target->head + _iter_->index) % _iter_->target->capacity;
    return _iter_->target->buffer[_phys_];
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
cvx_container *FUNC_PROXY(_clone)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, NULL); return (cvx_container *)FUNC(_clone)((struct SNAME *)_col_); }
void FUNC_PROXY(_drop)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_drop)((struct SNAME *)_col_); }
void FUNC_PROXY(_clear)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_clear)((struct SNAME *)_col_); }
enum cvx_flags FUNC_PROXY(_flag)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, CVX_FLAG_WRONG_TAG); return FUNC(_flag)((struct SNAME *)_col_); }
size_t FUNC_PROXY(_count)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, 0); return FUNC(_count)((struct SNAME *)_col_); }
size_t FUNC_PROXY(_capacity)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, 0); return FUNC(_capacity)((struct SNAME *)_col_); }
bool FUNC_PROXY(_empty)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_empty)((struct SNAME *)_col_); }
bool FUNC_PROXY(_full)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_full)((struct SNAME *)_col_); }
bool FUNC_PROXY(_is_linearized)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_is_linearized)((struct SNAME *)_col_); }
V FUNC_PROXY(_front)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_front)((struct SNAME *)_col_); }
V FUNC_PROXY(_back)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_back)((struct SNAME *)_col_); }
V FUNC_PROXY(_get)(cvx_container *_col_, size_t _index_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_get)((struct SNAME *)_col_, _index_); }
void FUNC_PROXY(_push_back)(cvx_container *_col_, V _item_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_push_back)((struct SNAME *)_col_, _item_); }
void FUNC_PROXY(_push_front)(cvx_container *_col_, V _item_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_push_front)((struct SNAME *)_col_, _item_); }
V FUNC_PROXY(_pop_back)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_pop_back)((struct SNAME *)_col_); }
V FUNC_PROXY(_pop_front)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_pop_front)((struct SNAME *)_col_); }
void FUNC_PROXY(_set_capacity)(cvx_container *_col_, size_t _new_cap_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_set_capacity)((struct SNAME *)_col_, _new_cap_); }
void FUNC_PROXY(_linearize)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_linearize)((struct SNAME *)_col_); }

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
void FUNC_PROXY(_iter_go_to)(cvx_container *_col_, size_t _index_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, ); FUNC(_iter_go_to)((struct ITERATOR *)_col_, _index_); }
V FUNC_PROXY(_iter_value)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, (V){ 0 }); return FUNC(_iter_value)((struct ITERATOR *)_col_); }
size_t FUNC_PROXY(_iter_index)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, 0); return FUNC(_iter_index)((struct ITERATOR *)_col_); }
// clang-format on

#ifdef IMPL_RANDOM_ACCESS_ITER
#define INTERFACE IMPL_RANDOM_ACCESS_ITER

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
#define IMPL_GO_TO FUNC_PROXY(_iter_go_to)
#define IMPL_VALUE FUNC_PROXY(_iter_value)
#define IMPL_INDEX FUNC_PROXY(_iter_index)

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
