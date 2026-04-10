// clang-format off
#ifndef K
#error "cvx/interval_map.h requires K to be defined (the domain/key type, e.g. #define K int)"
#endif
#ifndef V
#error "cvx/interval_map.h requires V to be defined (the value/codomain type, e.g. #define V int)"
#endif
#ifndef SNAME
#error "cvx/interval_map.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_imap)"
#endif
#ifndef PFX
#error "cvx/interval_map.h requires PFX to be defined (the function prefix, e.g. #define PFX mi)"
#endif
#ifndef TAG
#error "cvx/interval_map.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
#endif
// clang-format on

// Interval map with right-open intervals [lo, hi).
//
// Maps each [lo, hi) → V, with no overlapping intervals.  On _add, existing
// intervals that overlap or touch (with the same value, when vtabv->comp is
// set) the inserted range are merged; partial overlaps with a different value
// produce left/right residual intervals.  On _remove, intervals overlapping
// the erased range are split if necessary.
//
// vtabk->comp is required.  vtabk->copy / vtabk->drop apply to K boundary
// values.  vtabv->comp, if set, enables joining of adjacent equal-value
// intervals.  vtabv->copy / vtabv->drop apply to V values.
//
// _iter_value returns struct ENTRY {K lo; K hi; V val;}.  To cast to a
// generic iterator interface, declare it with V = struct CVX_(SNAME,_entry)
// BEFORE including this header.

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cvx/core.h"

#define FUNC(X) CVX_(PFX, X)
#define ENTRY CVX_(SNAME, _entry)
#define ITERATOR CVX_(SNAME, _iter)
#define ITER_TAG (TAG * CVX_ITER_TAG_MULT)
#define VTAB_K CVX_(SNAME, _vtabk)
#define VTAB_V CVX_(SNAME, _vtabv)

struct VTAB_K
{
    CVX_VTAB_DEFINITION(K)
};

struct VTAB_V
{
    CVX_VTAB_DEFINITION(V)
};

// An interval [lo, hi) together with the value mapped to it.
struct ENTRY
{
    K lo;
    K hi;
    V val;
};

struct SNAME
{
    cvx_container super;
    size_t capacity;      // allocated slots in buffer
    size_t count;         // number of intervals stored
    struct VTAB_K *vtabk; // vtabk->comp is required
    struct VTAB_V *vtabv; // optional; comp enables joining
    struct ENTRY *buffer; // sorted, non-overlapping entries
};

struct ITERATOR
{
    cvx_container super; // tag = ITER_TAG
    size_t index;        // current position (count = past-end)
    struct SNAME *target;
};

// ---- Non-allocating initializers ----
struct SNAME FUNC(_init)(struct VTAB_K *_vtabk_, struct VTAB_V *_vtabv_);
struct SNAME FUNC(_copy)(struct SNAME *_self_);

// ---- Allocating initializers ----
cvx_container *FUNC(_new)(void);
cvx_container *FUNC(_new_with)(struct VTAB_K *_vtabk_, struct VTAB_V *_vtabv_);
cvx_container *FUNC(_clone)(cvx_container *_col_);

// ---- Destructors ----
void FUNC(_drop)(cvx_container *_col_);
void FUNC(_clear)(cvx_container *_col_);

// ---- Getters ----
size_t FUNC(_count)(cvx_container *_col_);
bool FUNC(_empty)(cvx_container *_col_);

// ---- Operations ----
void FUNC(_add)(cvx_container *_col_, K _lo_, K _hi_, V _val_);
void FUNC(_remove)(cvx_container *_col_, K _lo_, K _hi_);
V FUNC(_get)(cvx_container *_col_, K _key_);
bool FUNC(_contains_key)(cvx_container *_col_, K _key_);
bool FUNC(_contains_interval)(cvx_container *_col_, K _lo_, K _hi_);
bool FUNC(_overlaps)(cvx_container *_col_, K _lo_, K _hi_);

// ---- Iterator constructors ----
struct ITERATOR FUNC(_iter_init_start)(cvx_container *_target_);
struct ITERATOR FUNC(_iter_init_end)(cvx_container *_target_);
cvx_container *FUNC(_iter_start)(cvx_container *_target_);
cvx_container *FUNC(_iter_end)(cvx_container *_target_);
void FUNC(_iter_drop)(cvx_container *_iter_);

// ---- Iterator state ----
bool FUNC(_iter_at_start)(cvx_container *_iter_);
bool FUNC(_iter_at_end)(cvx_container *_iter_);
size_t FUNC(_iter_count)(cvx_container *_iter_);

// ---- Iterator movement ----
void FUNC(_iter_to_start)(cvx_container *_iter_);
void FUNC(_iter_to_end)(cvx_container *_iter_);
void FUNC(_iter_next)(cvx_container *_iter_);
void FUNC(_iter_prev)(cvx_container *_iter_);
void FUNC(_iter_forward)(cvx_container *_iter_, size_t _steps_);
void FUNC(_iter_backward)(cvx_container *_iter_, size_t _steps_);

// ---- Iterator access ----
struct ENTRY FUNC(_iter_value)(cvx_container *_iter_);
K FUNC(_iter_value_lo)(cvx_container *_iter_);
K FUNC(_iter_value_hi)(cvx_container *_iter_);
V FUNC(_iter_value_val)(cvx_container *_iter_);
size_t FUNC(_iter_index)(cvx_container *_iter_);

// ---- Private helpers ----
bool FUNC(__assert_capacity)(cvx_container *_col_);
size_t FUNC(__lower_bound_k)(struct SNAME *_self_, K _lo_);

///
///
/// IMPLEMENTATIONS
///
///

struct SNAME FUNC(_init)(struct VTAB_K *_vtabk_, struct VTAB_V *_vtabv_)
{
    struct SNAME _res_ = (struct SNAME){ 0 };

    if (!_vtabk_ || !_vtabk_->comp)
    {
        _res_.super.flag = CVX_FLAG_VTAB;
        return _res_;
    }

    _res_.super.tag = TAG;
    _res_.super.flag = CVX_FLAG_OK;
    _res_.vtabk = _vtabk_;
    _res_.vtabv = _vtabv_;
    return _res_;
}

struct SNAME FUNC(_copy)(struct SNAME *_self_)
{
    struct SNAME _res_ = FUNC(_init)(_self_->vtabk, _self_->vtabv);
    _res_.super.flag = CVX_FLAG_OK;

    if (_self_->count == 0)
        return _res_;

    struct ENTRY *_buf_ = malloc(sizeof(struct ENTRY) * _self_->count);
    if (!_buf_)
    {
        _res_.super.flag = CVX_FLAG_ALLOC;
        return _res_;
    }

    for (size_t _i_ = 0; _i_ < _self_->count; _i_++)
    {
        _buf_[_i_].lo = (_self_->vtabk && _self_->vtabk->copy)
                            ? _self_->vtabk->copy(_self_->buffer[_i_].lo)
                            : _self_->buffer[_i_].lo;
        _buf_[_i_].hi = (_self_->vtabk && _self_->vtabk->copy)
                            ? _self_->vtabk->copy(_self_->buffer[_i_].hi)
                            : _self_->buffer[_i_].hi;
        _buf_[_i_].val = (_self_->vtabv && _self_->vtabv->copy)
                             ? _self_->vtabv->copy(_self_->buffer[_i_].val)
                             : _self_->buffer[_i_].val;
    }

    _res_.buffer = _buf_;
    _res_.count = _self_->count;
    _res_.capacity = _self_->count;
    return _res_;
}

cvx_container *FUNC(_new)(void)
{
    struct SNAME *_res_ = malloc(sizeof(struct SNAME));
    if (!_res_)
        return NULL;

    _res_->super.tag = TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->capacity = 0;
    _res_->count = 0;
    _res_->vtabk = NULL;
    _res_->vtabv = NULL;
    _res_->buffer = NULL;
    return (cvx_container *)_res_;
}

cvx_container *FUNC(_new_with)(struct VTAB_K *_vtabk_, struct VTAB_V *_vtabv_)
{
    struct SNAME *_res_ = malloc(sizeof(struct SNAME));
    if (!_res_)
        return NULL;

    _res_->super.tag = TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->capacity = 0;
    _res_->count = 0;
    _res_->vtabk = _vtabk_;
    _res_->vtabv = _vtabv_;
    _res_->buffer = NULL;
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
    _copy_->vtabk = _orig_->vtabk;
    _copy_->vtabv = _orig_->vtabv;

    if (_orig_->count == 0)
    {
        _res_->flag = CVX_FLAG_OK;
        return _res_;
    }

    struct ENTRY *_buf_ = malloc(sizeof(struct ENTRY) * _orig_->count);
    if (!_buf_)
    {
        FUNC(_drop)(_res_);
        return NULL;
    }

    for (size_t _i_ = 0; _i_ < _orig_->count; _i_++)
    {
        _buf_[_i_].lo = (_orig_->vtabk && _orig_->vtabk->copy)
                            ? _orig_->vtabk->copy(_orig_->buffer[_i_].lo)
                            : _orig_->buffer[_i_].lo;
        _buf_[_i_].hi = (_orig_->vtabk && _orig_->vtabk->copy)
                            ? _orig_->vtabk->copy(_orig_->buffer[_i_].hi)
                            : _orig_->buffer[_i_].hi;
        _buf_[_i_].val = (_orig_->vtabv && _orig_->vtabv->copy)
                             ? _orig_->vtabv->copy(_orig_->buffer[_i_].val)
                             : _orig_->buffer[_i_].val;
    }

    _copy_->buffer = _buf_;
    _copy_->count = _orig_->count;
    _copy_->capacity = _orig_->count;
    _res_->flag = CVX_FLAG_OK;
    return _res_;
}

void FUNC(_drop)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    for (size_t _i_ = 0; _i_ < _self_->count; _i_++)
    {
        if (_self_->vtabk && _self_->vtabk->drop)
        {
            _self_->vtabk->drop(_self_->buffer[_i_].lo);
            _self_->vtabk->drop(_self_->buffer[_i_].hi);
        }
        if (_self_->vtabv && _self_->vtabv->drop)
            _self_->vtabv->drop(_self_->buffer[_i_].val);
    }

    free(_self_->buffer);
    free(_self_);
}

void FUNC(_clear)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    for (size_t _i_ = 0; _i_ < _self_->count; _i_++)
    {
        if (_self_->vtabk && _self_->vtabk->drop)
        {
            _self_->vtabk->drop(_self_->buffer[_i_].lo);
            _self_->vtabk->drop(_self_->buffer[_i_].hi);
        }
        if (_self_->vtabv && _self_->vtabv->drop)
            _self_->vtabv->drop(_self_->buffer[_i_].val);
    }

    free(_self_->buffer);
    _self_->buffer = NULL;
    _self_->capacity = 0;

    _self_->count = 0;
    _col_->flag = CVX_FLAG_OK;
}

size_t FUNC(_count)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, 0);

    _col_->flag = CVX_FLAG_OK;
    return ((struct SNAME *)_col_)->count;
}

bool FUNC(_empty)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, false);

    _col_->flag = CVX_FLAG_OK;
    return ((struct SNAME *)_col_)->count == 0;
}

void FUNC(_add)(cvx_container *_col_, K _lo_, K _hi_, V _val_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _col_->flag = CVX_FLAG_VTAB;
        return;
    }

    if (_self_->vtabk->comp(_lo_, _hi_) >= 0)
    {
        _col_->flag = CVX_FLAG_INVALID;
        return;
    }

    if (!FUNC(__assert_capacity)(_col_))
        return;

    // Find first buffer entry whose hi >= lo (could overlap or touch [lo,hi)).
    size_t _start_ = FUNC(__lower_bound_k)(_self_, _lo_);

    // If buffer[start].hi == lo (left-touching) and values differ: skip it.
    if (_start_ < _self_->count && _self_->vtabk->comp(_self_->buffer[_start_].hi, _lo_) == 0 &&
        (!_self_->vtabv || !_self_->vtabv->comp ||
         _self_->vtabv->comp(_self_->buffer[_start_].val, _val_) != 0))
    {
        _start_++;
    }

    // Scan forward: collect entries that overlap OR touch with same value.
    size_t _end_ = _start_;
    while (_end_ < _self_->count)
    {
        int _cmp_ = _self_->vtabk->comp(_self_->buffer[_end_].lo, _hi_);
        if (_cmp_ > 0)
            break;
        if (_cmp_ == 0 && (!_self_->vtabv || !_self_->vtabv->comp ||
                           _self_->vtabv->comp(_self_->buffer[_end_].val, _val_) != 0))
            break;
        _end_++;
    }

    if (_end_ == _start_)
    {
        // Pure insert: no existing entries interact with [lo, hi).
        memmove(&_self_->buffer[_start_ + 1], &_self_->buffer[_start_],
                (_self_->count - _start_) * sizeof(struct ENTRY));
        _self_->buffer[_start_] = (struct ENTRY){ .lo = _lo_, .hi = _hi_, .val = _val_ };
        _self_->count++;
        _col_->flag = CVX_FLAG_OK;
        return;
    }

    size_t _last_ = _end_ - 1;

    // Determine whether the boundary entries partially extend outside [lo,hi).
    bool _has_left_ = _self_->vtabk->comp(_self_->buffer[_start_].lo, _lo_) < 0;
    bool _same_val_left_ = _has_left_ && _self_->vtabv && _self_->vtabv->comp &&
                           _self_->vtabv->comp(_self_->buffer[_start_].val, _val_) == 0;
    bool _has_right_ = _self_->vtabk->comp(_hi_, _self_->buffer[_last_].hi) < 0;
    bool _same_val_right_ = _has_right_ && _self_->vtabv && _self_->vtabv->comp &&
                            _self_->vtabv->comp(_self_->buffer[_last_].val, _val_) == 0;

    // Effective lo/hi of the merged entry.
    // When absorbing (same value), copy the boundary before the drop loop.
    K _effective_lo_ = _lo_;
    if (_has_left_ && _same_val_left_)
    {
        _effective_lo_ = (_self_->vtabk && _self_->vtabk->copy)
                             ? _self_->vtabk->copy(_self_->buffer[_start_].lo)
                             : _self_->buffer[_start_].lo;
    }

    K _effective_hi_ = _hi_;
    if (_has_right_ && _same_val_right_)
    {
        _effective_hi_ = (_self_->vtabk && _self_->vtabk->copy)
                             ? _self_->vtabk->copy(_self_->buffer[_last_].hi)
                             : _self_->buffer[_last_].hi;
    }

    // Build left residual (different value, partially overlapping from left).
    K _left_lo_, _left_hi_;
    V _left_val_;
    bool _build_left_ = _has_left_ && !_same_val_left_;
    if (_build_left_)
    {
        _left_lo_ = (_self_->vtabk && _self_->vtabk->copy)
                        ? _self_->vtabk->copy(_self_->buffer[_start_].lo)
                        : _self_->buffer[_start_].lo;
        _left_hi_ = (_self_->vtabk && _self_->vtabk->copy) ? _self_->vtabk->copy(_lo_) : _lo_;
        _left_val_ = (_self_->vtabv && _self_->vtabv->copy)
                         ? _self_->vtabv->copy(_self_->buffer[_start_].val)
                         : _self_->buffer[_start_].val;
    }

    // Build right residual (different value, partially overlapping from right).
    K _right_lo_, _right_hi_;
    V _right_val_;
    bool _build_right_ = _has_right_ && !_same_val_right_;
    if (_build_right_)
    {
        _right_lo_ = (_self_->vtabk && _self_->vtabk->copy) ? _self_->vtabk->copy(_hi_) : _hi_;
        _right_hi_ = (_self_->vtabk && _self_->vtabk->copy)
                         ? _self_->vtabk->copy(_self_->buffer[_last_].hi)
                         : _self_->buffer[_last_].hi;
        _right_val_ = (_self_->vtabv && _self_->vtabv->copy)
                          ? _self_->vtabv->copy(_self_->buffer[_last_].val)
                          : _self_->buffer[_last_].val;
    }

    // Drop every entry in buffer[start..last].
    // Residuals and effective boundaries were already copied above (or are
    // shallow copies when copy is NULL — the caller accepts aliasing in that
    // case).
    for (size_t _i_ = _start_; _i_ <= _last_; _i_++)
    {
        if (_self_->vtabk && _self_->vtabk->drop)
        {
            _self_->vtabk->drop(_self_->buffer[_i_].lo);
            _self_->vtabk->drop(_self_->buffer[_i_].hi);
        }
        if (_self_->vtabv && _self_->vtabv->drop)
            _self_->vtabv->drop(_self_->buffer[_i_].val);
    }

    size_t _n_remove_ = _end_ - _start_;
    size_t _n_insert_ = 1u + (_build_left_ ? 1u : 0u) + (_build_right_ ? 1u : 0u);

    // Shift the tail of the buffer to its final position.
    memmove(&_self_->buffer[_start_ + _n_insert_], &_self_->buffer[_end_],
            (_self_->count - _end_) * sizeof(struct ENTRY));

    _self_->count = _self_->count - _n_remove_ + _n_insert_;

    // Fill the inserted entries.
    size_t _idx_ = _start_;
    if (_build_left_)
        _self_->buffer[_idx_++] =
            (struct ENTRY){ .lo = _left_lo_, .hi = _left_hi_, .val = _left_val_ };

    _self_->buffer[_idx_++] =
        (struct ENTRY){ .lo = _effective_lo_, .hi = _effective_hi_, .val = _val_ };

    if (_build_right_)
        _self_->buffer[_idx_++] =
            (struct ENTRY){ .lo = _right_lo_, .hi = _right_hi_, .val = _right_val_ };

    _col_->flag = CVX_FLAG_OK;
}

void FUNC(_remove)(cvx_container *_col_, K _lo_, K _hi_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _col_->flag = CVX_FLAG_VTAB;
        return;
    }

    // Empty range is a no-op.
    if (_self_->vtabk->comp(_lo_, _hi_) >= 0)
    {
        _col_->flag = CVX_FLAG_OK;
        return;
    }

    // Ensure capacity for a potential split (net +1 entry).
    if (!FUNC(__assert_capacity)(_col_))
        return;

    size_t _start_ = FUNC(__lower_bound_k)(_self_, _lo_);

    // Strict overlap: buffer[end].lo < hi.
    size_t _end_ = _start_;
    while (_end_ < _self_->count && _self_->vtabk->comp(_self_->buffer[_end_].lo, _hi_) < 0)
    {
        _end_++;
    }

    if (_end_ == _start_)
    {
        _col_->flag = CVX_FLAG_OK;
        return;
    }

    size_t _last_ = _end_ - 1;
    bool _has_left_ = _self_->vtabk->comp(_self_->buffer[_start_].lo, _lo_) < 0;
    bool _has_right_ = _self_->vtabk->comp(_hi_, _self_->buffer[_last_].hi) < 0;

    K _left_lo_, _left_hi_;
    V _left_val_;
    if (_has_left_)
    {
        _left_lo_ = (_self_->vtabk && _self_->vtabk->copy)
                        ? _self_->vtabk->copy(_self_->buffer[_start_].lo)
                        : _self_->buffer[_start_].lo;
        _left_hi_ = (_self_->vtabk && _self_->vtabk->copy) ? _self_->vtabk->copy(_lo_) : _lo_;
        _left_val_ = (_self_->vtabv && _self_->vtabv->copy)
                         ? _self_->vtabv->copy(_self_->buffer[_start_].val)
                         : _self_->buffer[_start_].val;
    }

    K _right_lo_, _right_hi_;
    V _right_val_;
    if (_has_right_)
    {
        _right_lo_ = (_self_->vtabk && _self_->vtabk->copy) ? _self_->vtabk->copy(_hi_) : _hi_;
        _right_hi_ = (_self_->vtabk && _self_->vtabk->copy)
                         ? _self_->vtabk->copy(_self_->buffer[_last_].hi)
                         : _self_->buffer[_last_].hi;
        _right_val_ = (_self_->vtabv && _self_->vtabv->copy)
                          ? _self_->vtabv->copy(_self_->buffer[_last_].val)
                          : _self_->buffer[_last_].val;
    }

    // Drop all overlapping entries.
    for (size_t _i_ = _start_; _i_ < _end_; _i_++)
    {
        if (_self_->vtabk && _self_->vtabk->drop)
        {
            _self_->vtabk->drop(_self_->buffer[_i_].lo);
            _self_->vtabk->drop(_self_->buffer[_i_].hi);
        }
        if (_self_->vtabv && _self_->vtabv->drop)
            _self_->vtabv->drop(_self_->buffer[_i_].val);
    }

    size_t _n_remove_ = _end_ - _start_;
    size_t _n_insert_ = (_has_left_ ? 1u : 0u) + (_has_right_ ? 1u : 0u);

    memmove(&_self_->buffer[_start_ + _n_insert_], &_self_->buffer[_end_],
            (_self_->count - _end_) * sizeof(struct ENTRY));

    _self_->count = _self_->count - _n_remove_ + _n_insert_;

    size_t _idx_ = _start_;
    if (_has_left_)
        _self_->buffer[_idx_++] =
            (struct ENTRY){ .lo = _left_lo_, .hi = _left_hi_, .val = _left_val_ };
    if (_has_right_)
        _self_->buffer[_idx_++] =
            (struct ENTRY){ .lo = _right_lo_, .hi = _right_hi_, .val = _right_val_ };

    _col_->flag = CVX_FLAG_OK;
}

V FUNC(_get)(cvx_container *_col_, K _key_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _col_->flag = CVX_FLAG_VTAB;
        return (V){ 0 };
    }

    if (_self_->count == 0)
    {
        _col_->flag = CVX_FLAG_NOT_FOUND;
        return (V){ 0 };
    }

    // Upper-bound on lo: first index where buffer[i].lo > key.
    size_t _lo_idx_ = 0, _hi_idx_ = _self_->count;
    while (_lo_idx_ < _hi_idx_)
    {
        size_t _mid_ = _lo_idx_ + (_hi_idx_ - _lo_idx_) / 2;
        if (_self_->vtabk->comp(_self_->buffer[_mid_].lo, _key_) <= 0)
            _lo_idx_ = _mid_ + 1;
        else
            _hi_idx_ = _mid_;
    }

    if (_lo_idx_ == 0)
    {
        _col_->flag = CVX_FLAG_NOT_FOUND;
        return (V){ 0 };
    }

    size_t _i_ = _lo_idx_ - 1;
    if (_self_->vtabk->comp(_key_, _self_->buffer[_i_].hi) < 0)
    {
        _col_->flag = CVX_FLAG_OK;
        return _self_->buffer[_i_].val;
    }

    _col_->flag = CVX_FLAG_NOT_FOUND;
    return (V){ 0 };
}

bool FUNC(_contains_key)(cvx_container *_col_, K _key_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, false);

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _col_->flag = CVX_FLAG_VTAB;
        return false;
    }

    _col_->flag = CVX_FLAG_OK;

    if (_self_->count == 0)
        return false;

    size_t _lo_idx_ = 0, _hi_idx_ = _self_->count;
    while (_lo_idx_ < _hi_idx_)
    {
        size_t _mid_ = _lo_idx_ + (_hi_idx_ - _lo_idx_) / 2;
        if (_self_->vtabk->comp(_self_->buffer[_mid_].lo, _key_) <= 0)
            _lo_idx_ = _mid_ + 1;
        else
            _hi_idx_ = _mid_;
    }

    if (_lo_idx_ == 0)
        return false;

    size_t _i_ = _lo_idx_ - 1;
    return _self_->vtabk->comp(_key_, _self_->buffer[_i_].hi) < 0;
}

bool FUNC(_contains_interval)(cvx_container *_col_, K _lo_, K _hi_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, false);

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _col_->flag = CVX_FLAG_VTAB;
        return false;
    }

    _col_->flag = CVX_FLAG_OK;

    if (_self_->vtabk->comp(_lo_, _hi_) >= 0 || _self_->count == 0)
        return false;

    size_t _start_ = FUNC(__lower_bound_k)(_self_, _lo_);

    if (_start_ >= _self_->count)
        return false;

    return _self_->vtabk->comp(_self_->buffer[_start_].lo, _lo_) <= 0 &&
           _self_->vtabk->comp(_hi_, _self_->buffer[_start_].hi) <= 0;
}

bool FUNC(_overlaps)(cvx_container *_col_, K _lo_, K _hi_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, false);

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _col_->flag = CVX_FLAG_VTAB;
        return false;
    }

    _col_->flag = CVX_FLAG_OK;

    if (_self_->vtabk->comp(_lo_, _hi_) >= 0 || _self_->count == 0)
        return false;

    size_t _start_ = FUNC(__lower_bound_k)(_self_, _lo_);

    if (_start_ >= _self_->count)
        return false;

    return _self_->vtabk->comp(_self_->buffer[_start_].hi, _lo_) > 0 &&
           _self_->vtabk->comp(_self_->buffer[_start_].lo, _hi_) < 0;
}

///
///
/// ITERATOR
///
///

struct ITERATOR FUNC(_iter_init_start)(cvx_container *_target_)
{
    CVX_CONTAINER_GUARDS(TAG, _target_, ((struct ITERATOR){ .super.flag = CVX_FLAG_WRONG_TAG }));

    struct SNAME *_self_ = (struct SNAME *)_target_;

    struct ITERATOR _res_ = { 0 };
    _res_.super.tag = ITER_TAG;
    _res_.super.flag = CVX_FLAG_OK;
    _res_.index = 0;
    _res_.target = _self_;
    return _res_;
}

struct ITERATOR FUNC(_iter_init_end)(cvx_container *_target_)
{
    CVX_CONTAINER_GUARDS(TAG, _target_, ((struct ITERATOR){ .super.flag = CVX_FLAG_WRONG_TAG }));

    struct SNAME *_self_ = (struct SNAME *)_target_;

    struct ITERATOR _res_ = { 0 };
    _res_.super.tag = ITER_TAG;
    _res_.super.flag = CVX_FLAG_OK;
    _res_.index = _self_->count;
    _res_.target = _self_;
    return _res_;
}

cvx_container *FUNC(_iter_start)(cvx_container *_target_)
{
    CVX_CONTAINER_GUARDS(TAG, _target_, NULL);

    struct SNAME *_self_ = (struct SNAME *)_target_;

    struct ITERATOR *_res_ = malloc(sizeof(struct ITERATOR));
    if (!_res_)
        return NULL;

    _res_->super.tag = ITER_TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->index = 0;
    _res_->target = _self_;
    return (cvx_container *)_res_;
}

cvx_container *FUNC(_iter_end)(cvx_container *_target_)
{
    CVX_CONTAINER_GUARDS(TAG, _target_, NULL);

    struct SNAME *_self_ = (struct SNAME *)_target_;

    struct ITERATOR *_res_ = malloc(sizeof(struct ITERATOR));
    if (!_res_)
        return NULL;

    _res_->super.tag = ITER_TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->index = _self_->count;
    _res_->target = _self_;
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
    size_t _actual_ = _steps_ < _remaining_ ? _steps_ : _remaining_;
    _self_->index += _actual_;
    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_backward)(cvx_container *_iter_, size_t _steps_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    size_t _actual_ = _steps_ < _self_->index ? _steps_ : _self_->index;
    _self_->index -= _actual_;
    _iter_->flag = CVX_FLAG_OK;
}

struct ENTRY FUNC(_iter_value)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, (struct ENTRY){ 0 });

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    if (_self_->index >= _self_->target->count)
    {
        _iter_->flag = CVX_FLAG_RANGE;
        return (struct ENTRY){ 0 };
    }

    _iter_->flag = CVX_FLAG_OK;
    return _self_->target->buffer[_self_->index];
}

K FUNC(_iter_value_lo)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, (K){ 0 });

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    if (_self_->index >= _self_->target->count)
    {
        _iter_->flag = CVX_FLAG_RANGE;
        return (K){ 0 };
    }

    _iter_->flag = CVX_FLAG_OK;
    return _self_->target->buffer[_self_->index].lo;
}

K FUNC(_iter_value_hi)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, (K){ 0 });

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    if (_self_->index >= _self_->target->count)
    {
        _iter_->flag = CVX_FLAG_RANGE;
        return (K){ 0 };
    }

    _iter_->flag = CVX_FLAG_OK;
    return _self_->target->buffer[_self_->index].hi;
}

V FUNC(_iter_value_val)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, (V){ 0 });

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    if (_self_->index >= _self_->target->count)
    {
        _iter_->flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    _iter_->flag = CVX_FLAG_OK;
    return _self_->target->buffer[_self_->index].val;
}

size_t FUNC(_iter_index)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, 0);

    _iter_->flag = CVX_FLAG_OK;
    return ((struct ITERATOR *)_iter_)->index;
}

///
///
/// PRIVATE FUNCTIONS
///
///

// Ensures the buffer has room for at least count+2 entries (worst case for
// _add: one entry split into three).  Returns false and sets CVX_FLAG_ALLOC on
// allocation failure.
bool FUNC(__assert_capacity)(cvx_container *_col_)
{
    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_self_->capacity >= _self_->count + 2)
        return true;

    size_t _new_cap_ = (_self_->capacity == 0) ? 16 : _self_->capacity * 2;
    // Make sure we always end up with at least count+2 slots.
    while (_new_cap_ < _self_->count + 2)
        _new_cap_ *= 2;

    if (!_self_->buffer)
    {
        struct ENTRY *_buf_ = malloc(sizeof(struct ENTRY) * _new_cap_);
        if (!_buf_)
        {
            _col_->flag = CVX_FLAG_ALLOC;
            return false;
        }
        _self_->buffer = _buf_;
    }
    else
    {
        struct ENTRY *_buf_ = realloc(_self_->buffer, sizeof(struct ENTRY) * _new_cap_);
        if (!_buf_)
        {
            _col_->flag = CVX_FLAG_ALLOC;
            return false;
        }
        _self_->buffer = _buf_;
    }

    _self_->capacity = _new_cap_;
    return true;
}

// Returns the index of the first entry whose hi >= lo (i.e., the first that
// could overlap or touch [lo, ...) from the left).
// Returns self->count if no such entry exists.
size_t FUNC(__lower_bound_k)(struct SNAME *_self_, K _lo_)
{
    size_t _lo_idx_ = 0, _hi_idx_ = _self_->count;
    while (_lo_idx_ < _hi_idx_)
    {
        size_t _mid_ = _lo_idx_ + (_hi_idx_ - _lo_idx_) / 2;
        if (_self_->vtabk->comp(_self_->buffer[_mid_].hi, _lo_) < 0)
            _lo_idx_ = _mid_ + 1;
        else
            _hi_idx_ = _mid_;
    }
    return _lo_idx_;
}

///
///
/// INTERFACE CASTS
///
///

#ifdef IMPL_FORWARD_ITER
// NOTE: the IMPL_FORWARD_ITER interface must have been declared with
//   #define V  struct CVX_(SNAME,_entry)
//   #define INTERFACE <name>
//   #include "cvx/iter/forward_iterator.h"
// before this header, because _iter_value returns struct ENTRY.
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
// NOTE: the IMPL_BIDIRECTIONAL_ITER interface must have been declared with
//   #define V  struct CVX_(SNAME,_entry)
//   #define INTERFACE <name>
//   #include "cvx/iter/bidirectional_iterator.h"
// before this header, because _iter_value returns struct ENTRY.
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
