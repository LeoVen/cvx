/// binary_heap.h
///
/// This is a Binary Heap implementation that uses a contiguous array to store
/// its elements. The indices are a simulated to form a binary tree.
///
/// Given a node I at an index i, you can find:
/// Parent(I) = (i - 1) / 2, { for i > 0 }
///   Left(I) = 2i + 1
///  Right(I) = 2i + 2
///
/// Visual structure (representation):
///
///
///               [ 95 ]                    <-- Root (Level 0)
///              /      \
///         [ 70 ]      [ 80 ]              <-- Level 1
///         /    \      /    \
///     [ 40 ] [ 50 ] [ 10 ] [ 30 ]         <-- Level 2
///
/// Array-based structure (actual structure in memory):
///
/// Index:    0      1      2      3      4      5      6
///         +------+------+------+------+------+------+------+
///  Array: |  95  |  70  |  80  |  40  |  50  |  10  |  30  |
///         +------+------+------+------+------+------+------+
///            ^      ^      ^      ^      ^      ^      ^
///          Root   L-Child R-Child ...    ...    ...    ...
///
/// The heap is by default a Max heap, but it can be initialized with an enum of
/// type `cvx_heap_order` that is going to multiply the result of your compare
/// function by 1 or -1. This is usefull to keep a single comparison function
/// for the custom type, while being able to check at runtime if a heap is MIN
/// or MAX.
///

#include "cvx/fallback.h"

// clang-format off
#ifndef V
#error "cvx/binary_heap.h requires V to be defined (the element type, e.g. #define V int)"
#endif
#ifndef SNAME
#error "cvx/binary_heap.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_heap)"
#endif
#ifndef PFX
#error "cvx/binary_heap.h requires PFX to be defined (the function prefix, e.g. #define PFX mh)"
#endif
#ifndef TAG
#error "cvx/binary_heap.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
#endif
// clang-format on

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
    enum cvx_heap_order HO;
    V *buffer;
};

struct ITERATOR
{
    cvx_container super;
    size_t index;
    struct SNAME *target;
};

// ---- Non-allocating initializers ----
struct SNAME FUNC(_init)(struct VTAB_V *_vtabv_);
struct SNAME FUNC(_init_with)(struct VTAB_V *_vtabv_, enum cvx_heap_order HO, size_t capacity);
struct SNAME FUNC(_copy)(struct SNAME *_self_);

// ---- Allocating initializers ----
struct SNAME *FUNC(_new)(void);
struct SNAME *FUNC(_new_with)(struct VTAB_V *_vtabv_, enum cvx_heap_order HO, size_t capacity);
struct SNAME *FUNC(_clone)(struct SNAME *_orig_);

// ---- Destructors ----
void FUNC(_drop)(struct SNAME *_self_);
void FUNC(_clear)(struct SNAME *_self_);

// ---- Getters ----
enum cvx_flags FUNC(_flag)(struct SNAME *_self_);
enum cvx_heap_order FUNC(_heap_order)(struct SNAME *_self_);
size_t FUNC(_count)(struct SNAME *_self_);
size_t FUNC(_capacity)(struct SNAME *_self_);
bool FUNC(_empty)(struct SNAME *_self_);
bool FUNC(_full)(struct SNAME *_self_);

// ---- Operations ----
void FUNC(_push)(struct SNAME *_self_, V _item_);
V FUNC(_pop)(struct SNAME *_self_);
V FUNC(_peek)(struct SNAME *_self_);

// ---- Iterator constructors and destructors ----
struct ITERATOR FUNC(_iter_init_start)(struct SNAME *_target_);
struct ITERATOR FUNC(_iter_init_end)(struct SNAME *_target_);
struct ITERATOR *FUNC(_iter_start)(struct SNAME *_target_);
struct ITERATOR *FUNC(_iter_end)(struct SNAME *_target_);
void FUNC(_iter_drop)(struct ITERATOR *_iter_);

// ---- Iterator state ----
bool FUNC(_iter_at_start)(struct ITERATOR *_iter_);
bool FUNC(_iter_at_end)(struct ITERATOR *_iter_);
size_t FUNC(_iter_count)(struct ITERATOR *_iter_);

// ---- Iterator movement ----
void FUNC(_iter_to_start)(struct ITERATOR *_iter_);
void FUNC(_iter_to_end)(struct ITERATOR *_iter_);
void FUNC(_iter_next)(struct ITERATOR *_iter_);
void FUNC(_iter_prev)(struct ITERATOR *_iter_);
void FUNC(_iter_forward)(struct ITERATOR *_iter_, size_t _steps_);
void FUNC(_iter_backward)(struct ITERATOR *_iter_, size_t _steps_);
void FUNC(_iter_go_to)(struct ITERATOR *_iter_, size_t _index_);

// ---- Iterator access ----
V FUNC(_iter_value)(struct ITERATOR *_iter_);
size_t FUNC(_iter_index)(struct ITERATOR *_iter_);

// ---- Private helpers ----
void FUNC(__float_up)(struct SNAME *_self_, size_t index);
void FUNC(__float_down)(struct SNAME *_self_, size_t index);
bool FUNC(__assert_capacity)(struct SNAME *_self_);

///
///
/// IMPLEMENTATIONS
///
///

struct SNAME FUNC(_init)(struct VTAB_V *_vtabv_)
{
    struct SNAME _res_ = (struct SNAME){ 0 };

    if (!_vtabv_ || !_vtabv_->comp)
    {
        _res_.super.flag = CVX_FLAG_VTAB;
        return _res_;
    }

    _res_.super.tag = TAG;
    _res_.vtabv = _vtabv_;
    _res_.HO = CVX_MAX_HEAP;

    return _res_;
}

struct SNAME FUNC(_init_with)(struct VTAB_V *_vtabv_, enum cvx_heap_order HO, size_t capacity)
{
    struct SNAME _res_ = (struct SNAME){ 0 };

    if (!_vtabv_ || !_vtabv_->comp)
    {
        _res_.super.flag = CVX_FLAG_VTAB;
        return _res_;
    }

    if (capacity > 0)
    {
        _res_.buffer = malloc(sizeof(V) * capacity);
        if (!_res_.buffer)
        {
            _res_.super.flag = CVX_FLAG_ALLOC;
            return _res_;
        }
        _res_.capacity = capacity;
    }

    _res_.super.tag = TAG;
    _res_.super.flag = CVX_FLAG_OK;
    _res_.vtabv = _vtabv_;
    _res_.HO = HO;

    return _res_;
}

struct SNAME FUNC(_copy)(struct SNAME *_self_)
{
    struct SNAME _res_ = (struct SNAME){ 0 };

    _res_.super.tag = TAG;
    _res_.super.flag = CVX_FLAG_OK;
    _res_.vtabv = _self_->vtabv;
    _res_.HO = _self_->HO;

    if (_self_->count == 0)
        return _res_;

    _res_.buffer = malloc(sizeof(V) * _self_->count);
    if (!_res_.buffer)
    {
        _res_.super.flag = CVX_FLAG_ALLOC;
        return _res_;
    }

    for (size_t _i_ = 0; _i_ < _self_->count; _i_++)
    {
        if (_self_->vtabv && _self_->vtabv->copy)
            _res_.buffer[_i_] = _self_->vtabv->copy(_self_->buffer[_i_]);
        else
            _res_.buffer[_i_] = _self_->buffer[_i_];
    }

    _res_.count = _self_->count;
    _res_.capacity = _self_->count;

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
    _res_->HO = CVX_MAX_HEAP;

    return _res_;
}

struct SNAME *FUNC(_new_with)(struct VTAB_V *_vtabv_, enum cvx_heap_order HO, size_t capacity)
{
    struct SNAME *_res_ = malloc(sizeof(struct SNAME));

    if (!_res_)
        return NULL;

    if (capacity > 0)
    {
        _res_->buffer = malloc(sizeof(V) * capacity);
        if (!_res_->buffer)
        {
            free(_res_);
            return NULL;
        }
        _res_->capacity = capacity;
    }
    else
    {
        _res_->capacity = 0;
        _res_->buffer = NULL;
    }

    _res_->super.tag = TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->count = 0;
    _res_->vtabv = _vtabv_;
    _res_->HO = HO;

    return _res_;
}

struct SNAME *FUNC(_clone)(struct SNAME *_orig_)
{
    struct SNAME *_copy_ = FUNC(_new)();
    if (!_copy_)
        return NULL;

    _copy_->vtabv = _orig_->vtabv;
    _copy_->HO = _orig_->HO;

    if (_orig_->count == 0)
    {
        _copy_->super.flag = CVX_FLAG_OK;
        return _copy_;
    }

    _copy_->buffer = malloc(sizeof(V) * _orig_->count);
    if (!_copy_->buffer)
    {
        free(_copy_);
        return NULL;
    }

    for (size_t _i_ = 0; _i_ < _orig_->count; _i_++)
    {
        if (_orig_->vtabv && _orig_->vtabv->copy)
            _copy_->buffer[_i_] = _orig_->vtabv->copy(_orig_->buffer[_i_]);
        else
            _copy_->buffer[_i_] = _orig_->buffer[_i_];
    }

    _copy_->count = _orig_->count;
    _copy_->capacity = _orig_->count;
    _copy_->super.flag = CVX_FLAG_OK;

    return _copy_;
}

void FUNC(_drop)(struct SNAME *_self_)
{
    if (_self_->vtabv && _self_->vtabv->drop)
    {
        for (size_t _i_ = 0; _i_ < _self_->count; _i_++)
            _self_->vtabv->drop(_self_->buffer[_i_]);
    }

    free(_self_->buffer);
    free(_self_);
}

void FUNC(_clear)(struct SNAME *_self_)
{
    if (_self_->vtabv && _self_->vtabv->drop)
    {
        for (size_t _i_ = 0; _i_ < _self_->count; _i_++)
            _self_->vtabv->drop(_self_->buffer[_i_]);
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

enum cvx_heap_order FUNC(_heap_order)(struct SNAME *_self_)
{
    return _self_->HO;
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

void FUNC(_push)(struct SNAME *_self_, V _item_)
{
    if (!FUNC(__assert_capacity)(_self_))
        return;

    _self_->buffer[_self_->count++] = _item_;

    if (_self_->count > 1)
        FUNC(__float_up)(_self_, _self_->count - 1);

    _self_->super.flag = CVX_FLAG_OK;
}

V FUNC(_pop)(struct SNAME *_self_)
{
    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _result_ = _self_->buffer[0];
    _self_->buffer[0] = _self_->buffer[_self_->count - 1];
    _self_->buffer[_self_->count - 1] = (V){ 0 };

    _self_->count--;

    if (_self_->count > 0)
        FUNC(__float_down)(_self_, 0);

    _self_->super.flag = CVX_FLAG_OK;
    return _result_;
}

V FUNC(_peek)(struct SNAME *_self_)
{
    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    _self_->super.flag = CVX_FLAG_OK;
    return _self_->buffer[0];
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

bool FUNC(__assert_capacity)(struct SNAME *_self_)
{
    if (_self_->count < _self_->capacity)
        return true;

    size_t _new_cap_ = (_self_->capacity == 0) ? 16 : _self_->capacity * 2;

    if (!_self_->buffer)
    {
        _self_->buffer = malloc(sizeof(V) * _new_cap_);
        if (!_self_->buffer)
        {
            _self_->super.flag = CVX_FLAG_ALLOC;
            return false;
        }
    }
    else
    {
        V *_new_buf_ = realloc(_self_->buffer, sizeof(V) * _new_cap_);
        if (!_new_buf_)
        {
            _self_->super.flag = CVX_FLAG_ALLOC;
            return false;
        }
        _self_->buffer = _new_buf_;
    }

    _self_->capacity = _new_cap_;
    return true;
}

void FUNC(__float_up)(struct SNAME *_self_, size_t index)
{
    size_t C = index;
    V child = _self_->buffer[C];
    V parent = _self_->buffer[(index - 1) / 2];

    int mod = _self_->HO;

    while (C > 0 && _self_->vtabv->comp(child, parent) * mod > 0)
    {
        V tmp = _self_->buffer[C];
        _self_->buffer[C] = _self_->buffer[(C - 1) / 2];
        _self_->buffer[(C - 1) / 2] = tmp;

        C = (C - 1) / 2;

        if (C == 0)
            break;

        child = _self_->buffer[C];
        parent = _self_->buffer[(C - 1) / 2];
    }
}

void FUNC(__float_down)(struct SNAME *_self_, size_t index)
{
    int mod = _self_->HO;

    while (index < _self_->count)
    {
        size_t L = 2 * index + 1;
        size_t R = 2 * index + 2;
        size_t C = index;

        if (L < _self_->count &&
            _self_->vtabv->comp(_self_->buffer[L], _self_->buffer[C]) * mod > 0)
        {
            C = L;
        }

        if (R < _self_->count &&
            _self_->vtabv->comp(_self_->buffer[R], _self_->buffer[C]) * mod > 0)
        {
            C = R;
        }

        if (C != index)
        {
            V tmp = _self_->buffer[index];
            _self_->buffer[index] = _self_->buffer[C];
            _self_->buffer[C] = tmp;

            index = C;
        }
        else
            break;
    }
}

///
///
/// PROXY FUNCTIONS (cvx_container * interface with tag guards)
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
void FUNC_PROXY(_push)(cvx_container *_col_, V _item_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_push)((struct SNAME *)_col_, _item_); }
V FUNC_PROXY(_pop)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_pop)((struct SNAME *)_col_); }
V FUNC_PROXY(_peek)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_peek)((struct SNAME *)_col_); }

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
struct ITERATOR FUNC_PROXY(_iter_init_start)(cvx_container *_col_) { struct ITERATOR _err_ = {0}; if (_col_->tag != TAG) { _col_->flag = CVX_FLAG_WRONG_TAG; _err_.super.flag = CVX_FLAG_WRONG_TAG; return _err_; } return FUNC(_iter_init_start)((struct SNAME *)_col_); }
struct ITERATOR FUNC_PROXY(_iter_init_end)(cvx_container *_col_) { struct ITERATOR _err_ = {0}; if (_col_->tag != TAG) { _col_->flag = CVX_FLAG_WRONG_TAG; _err_.super.flag = CVX_FLAG_WRONG_TAG; return _err_; } return FUNC(_iter_init_end)((struct SNAME *)_col_); }
// clang-format on

///
///
/// INTERFACE CASTS
///
///

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
