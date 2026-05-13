/// interval_map.h
///
/// Status
///
///   [x] concept
///   [ ] v1
///   [ ] tests
///   [ ] refine
///   [ ] stabilize
///
/// Interval map with right-open intervals [lo, hi).
///
/// Maps each [lo, hi) → V, with no overlapping intervals.  On _add, existing
/// intervals that overlap or touch (with the same value, when vtabv->comp is
/// set) the inserted range are merged; partial overlaps with a different value
/// produce left/right residual intervals.  On _remove, intervals overlapping
/// the erased range are split if necessary.
///
/// vtabk->comp is required.  vtabk->clone / vtabk->drop apply to K boundary
/// values.  vtabv->comp, if set, enables joining of adjacent equal-value
/// intervals.  vtabv->clone / vtabv->drop apply to V values.
///
/// _iter_entry returns struct ENTRY {K lo; K hi; V val;}.  To cast to a
/// generic iterator interface, declare it with V = struct CVX_(SNAME,_entry)
/// BEFORE including this header.

#include "cvx/fallback.h"

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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cvx/core.h"

#define FUNC(X) CVX_(PFX, X)
#define FUNC_PROXY(X) CVX_(PFX, CVX_(__proxy, X))
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

void FUNC(_init)(struct SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv);
void FUNC(_drop)(struct SNAME *_self_);
void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone);
// ---- Getters ----
enum cvx_flags FUNC(_flag)(struct SNAME *_self_);
size_t FUNC(_count)(struct SNAME *_self_);
bool FUNC(_empty)(struct SNAME *_self_);

// ---- Operations ----
void FUNC(_add)(struct SNAME *_self_, K _lo_, K _hi_, V _val_);
void FUNC(_remove)(struct SNAME *_self_, K _lo_, K _hi_);
V FUNC(_get)(struct SNAME *_self_, K _key_);
bool FUNC(_contains_key)(struct SNAME *_self_, K _key_);
bool FUNC(_contains_interval)(struct SNAME *_self_, K _lo_, K _hi_);
bool FUNC(_overlaps)(struct SNAME *_self_, K _lo_, K _hi_);

// ---- Iterator constructors ----
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

// ---- Iterator access ----
struct ENTRY FUNC(_iter_entry)(struct ITERATOR *_iter_);
K FUNC(_iter_value_lo)(struct ITERATOR *_iter_);
K FUNC(_iter_value_hi)(struct ITERATOR *_iter_);
V FUNC(_iter_value_val)(struct ITERATOR *_iter_);
size_t FUNC(_iter_index)(struct ITERATOR *_iter_);

// ---- Private helpers ----
bool FUNC(__assert_capacity)(struct SNAME *_self_);
size_t FUNC(__lower_bound_k)(struct SNAME *_self_, K _lo_);

///
///
/// IMPLEMENTATIONS
///
///

void FUNC(_init)(struct SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv)
{
    *self = (struct SNAME){ 0 };
    if (!vtabk || !vtabk->comp)
    {
        self->super.flag = CVX_FLAG_VTAB;
        return;
    }
    self->super.tag = TAG;
    self->super.flag = CVX_FLAG_OK;
    self->vtabk = vtabk;
    self->vtabv = vtabv;
}

void FUNC(_drop)(struct SNAME *_self_)
{
    if (!_self_)
        return;
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
}

void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone)
{
    FUNC(_init)(clone, orig->vtabk, orig->vtabv);
    if (clone->super.flag == CVX_FLAG_VTAB)
        return;

    if (orig->count == 0)
        return;

    struct ENTRY *_buf_ = malloc(sizeof(struct ENTRY) * orig->count);
    if (!_buf_)
    {
        clone->super.flag = CVX_FLAG_ALLOC;
        orig->super.flag = CVX_FLAG_ALLOC;
        return;
    }

    for (size_t _i_ = 0; _i_ < orig->count; _i_++)
    {
        _buf_[_i_].lo = (orig->vtabk && orig->vtabk->clone)
                            ? orig->vtabk->clone(orig->buffer[_i_].lo)
                            : orig->buffer[_i_].lo;
        _buf_[_i_].hi = (orig->vtabk && orig->vtabk->clone)
                            ? orig->vtabk->clone(orig->buffer[_i_].hi)
                            : orig->buffer[_i_].hi;
        _buf_[_i_].val = (orig->vtabv && orig->vtabv->clone)
                             ? orig->vtabv->clone(orig->buffer[_i_].val)
                             : orig->buffer[_i_].val;
    }

    clone->buffer = _buf_;
    clone->count = orig->count;
    clone->capacity = orig->count;
    orig->super.flag = CVX_FLAG_OK;
}

enum cvx_flags FUNC(_flag)(struct SNAME *_self_)
{
    return _self_->super.flag;
}

size_t FUNC(_count)(struct SNAME *_self_)
{
    return _self_->count;
}

bool FUNC(_empty)(struct SNAME *_self_)
{
    return _self_->count == 0;
}

void FUNC(_add)(struct SNAME *_self_, K _lo_, K _hi_, V _val_)
{
    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return;
    }

    if (_self_->vtabk->comp(_lo_, _hi_) >= 0)
    {
        _self_->super.flag = CVX_FLAG_INVALID;
        return;
    }

    if (!FUNC(__assert_capacity)(_self_))
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
        _self_->super.flag = CVX_FLAG_OK;
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
        _effective_lo_ = (_self_->vtabk && _self_->vtabk->clone)
                             ? _self_->vtabk->clone(_self_->buffer[_start_].lo)
                             : _self_->buffer[_start_].lo;
    }

    K _effective_hi_ = _hi_;
    if (_has_right_ && _same_val_right_)
    {
        _effective_hi_ = (_self_->vtabk && _self_->vtabk->clone)
                             ? _self_->vtabk->clone(_self_->buffer[_last_].hi)
                             : _self_->buffer[_last_].hi;
    }

    // Build left residual (different value, partially overlapping from left).
    K _left_lo_, _left_hi_;
    V _left_val_;
    bool _build_left_ = _has_left_ && !_same_val_left_;
    if (_build_left_)
    {
        _left_lo_ = (_self_->vtabk && _self_->vtabk->clone)
                        ? _self_->vtabk->clone(_self_->buffer[_start_].lo)
                        : _self_->buffer[_start_].lo;
        _left_hi_ = (_self_->vtabk && _self_->vtabk->clone) ? _self_->vtabk->clone(_lo_) : _lo_;
        _left_val_ = (_self_->vtabv && _self_->vtabv->clone)
                         ? _self_->vtabv->clone(_self_->buffer[_start_].val)
                         : _self_->buffer[_start_].val;
    }

    // Build right residual (different value, partially overlapping from right).
    K _right_lo_, _right_hi_;
    V _right_val_;
    bool _build_right_ = _has_right_ && !_same_val_right_;
    if (_build_right_)
    {
        _right_lo_ = (_self_->vtabk && _self_->vtabk->clone) ? _self_->vtabk->clone(_hi_) : _hi_;
        _right_hi_ = (_self_->vtabk && _self_->vtabk->clone)
                         ? _self_->vtabk->clone(_self_->buffer[_last_].hi)
                         : _self_->buffer[_last_].hi;
        _right_val_ = (_self_->vtabv && _self_->vtabv->clone)
                          ? _self_->vtabv->clone(_self_->buffer[_last_].val)
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

    _self_->super.flag = CVX_FLAG_OK;
}

void FUNC(_remove)(struct SNAME *_self_, K _lo_, K _hi_)
{
    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return;
    }

    // Empty range is a no-op.
    if (_self_->vtabk->comp(_lo_, _hi_) >= 0)
    {
        _self_->super.flag = CVX_FLAG_OK;
        return;
    }

    // Ensure capacity for a potential split (net +1 entry).
    if (!FUNC(__assert_capacity)(_self_))
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
        _self_->super.flag = CVX_FLAG_OK;
        return;
    }

    size_t _last_ = _end_ - 1;
    bool _has_left_ = _self_->vtabk->comp(_self_->buffer[_start_].lo, _lo_) < 0;
    bool _has_right_ = _self_->vtabk->comp(_hi_, _self_->buffer[_last_].hi) < 0;

    K _left_lo_, _left_hi_;
    V _left_val_;
    if (_has_left_)
    {
        _left_lo_ = (_self_->vtabk && _self_->vtabk->clone)
                        ? _self_->vtabk->clone(_self_->buffer[_start_].lo)
                        : _self_->buffer[_start_].lo;
        _left_hi_ = (_self_->vtabk && _self_->vtabk->clone) ? _self_->vtabk->clone(_lo_) : _lo_;
        _left_val_ = (_self_->vtabv && _self_->vtabv->clone)
                         ? _self_->vtabv->clone(_self_->buffer[_start_].val)
                         : _self_->buffer[_start_].val;
    }

    K _right_lo_, _right_hi_;
    V _right_val_;
    if (_has_right_)
    {
        _right_lo_ = (_self_->vtabk && _self_->vtabk->clone) ? _self_->vtabk->clone(_hi_) : _hi_;
        _right_hi_ = (_self_->vtabk && _self_->vtabk->clone)
                         ? _self_->vtabk->clone(_self_->buffer[_last_].hi)
                         : _self_->buffer[_last_].hi;
        _right_val_ = (_self_->vtabv && _self_->vtabv->clone)
                          ? _self_->vtabv->clone(_self_->buffer[_last_].val)
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

    _self_->super.flag = CVX_FLAG_OK;
}

V FUNC(_get)(struct SNAME *_self_, K _key_)
{
    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return (V){ 0 };
    }

    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_NOT_FOUND;
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
        _self_->super.flag = CVX_FLAG_NOT_FOUND;
        return (V){ 0 };
    }

    size_t _i_ = _lo_idx_ - 1;
    if (_self_->vtabk->comp(_key_, _self_->buffer[_i_].hi) < 0)
    {
        _self_->super.flag = CVX_FLAG_OK;
        return _self_->buffer[_i_].val;
    }

    _self_->super.flag = CVX_FLAG_NOT_FOUND;
    return (V){ 0 };
}

bool FUNC(_contains_key)(struct SNAME *_self_, K _key_)
{
    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return false;
    }

    _self_->super.flag = CVX_FLAG_OK;

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

bool FUNC(_contains_interval)(struct SNAME *_self_, K _lo_, K _hi_)
{
    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return false;
    }

    _self_->super.flag = CVX_FLAG_OK;

    if (_self_->vtabk->comp(_lo_, _hi_) >= 0 || _self_->count == 0)
        return false;

    size_t _start_ = FUNC(__lower_bound_k)(_self_, _lo_);

    if (_start_ >= _self_->count)
        return false;

    return _self_->vtabk->comp(_self_->buffer[_start_].lo, _lo_) <= 0 &&
           _self_->vtabk->comp(_hi_, _self_->buffer[_start_].hi) <= 0;
}

bool FUNC(_overlaps)(struct SNAME *_self_, K _lo_, K _hi_)
{
    if (!_self_->vtabk || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return false;
    }

    _self_->super.flag = CVX_FLAG_OK;

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
    size_t _actual_ = _steps_ < _remaining_ ? _steps_ : _remaining_;
    _iter_->index += _actual_;
    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_backward)(struct ITERATOR *_iter_, size_t _steps_)
{
    size_t _actual_ = _steps_ < _iter_->index ? _steps_ : _iter_->index;
    _iter_->index -= _actual_;
    _iter_->super.flag = CVX_FLAG_OK;
}

struct ENTRY FUNC(_iter_entry)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return (struct ENTRY){ 0 };
    }

    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->target->buffer[_iter_->index];
}

K FUNC(_iter_value_lo)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return (K){ 0 };
    }

    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->target->buffer[_iter_->index].lo;
}

K FUNC(_iter_value_hi)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return (K){ 0 };
    }

    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->target->buffer[_iter_->index].hi;
}

V FUNC(_iter_value_val)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->target->buffer[_iter_->index].val;
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

// Ensures the buffer has room for at least count+2 entries (worst case for
// _add: one entry split into three).  Returns false and sets CVX_FLAG_ALLOC on
// allocation failure.
bool FUNC(__assert_capacity)(struct SNAME *_self_)
{
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
            _self_->super.flag = CVX_FLAG_ALLOC;
            return false;
        }
        _self_->buffer = _buf_;
    }
    else
    {
        struct ENTRY *_buf_ = realloc(_self_->buffer, sizeof(struct ENTRY) * _new_cap_);
        if (!_buf_)
        {
            _self_->super.flag = CVX_FLAG_ALLOC;
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
/// PROXIES
///
///

// clang-format off
void FUNC_PROXY(_clone)(cvx_container *_orig_, cvx_container *_clone_) { CVX_CONTAINER_GUARDS(TAG, _orig_, ); FUNC(_clone)((struct SNAME *)_orig_, (struct SNAME *)_clone_); }
void FUNC_PROXY(_drop)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_drop)((struct SNAME *)_col_); free(_col_); }
size_t FUNC_PROXY(_count)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, 0); return FUNC(_count)((struct SNAME *)_col_); }
bool FUNC_PROXY(_empty)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_empty)((struct SNAME *)_col_); }
void FUNC_PROXY(_add)(cvx_container *_col_, K _lo_, K _hi_, V _val_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_add)((struct SNAME *)_col_, _lo_, _hi_, _val_); }
void FUNC_PROXY(_remove)(cvx_container *_col_, K _lo_, K _hi_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_remove)((struct SNAME *)_col_, _lo_, _hi_); }
V FUNC_PROXY(_get)(cvx_container *_col_, K _key_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_get)((struct SNAME *)_col_, _key_); }
bool FUNC_PROXY(_contains_key)(cvx_container *_col_, K _key_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_contains_key)((struct SNAME *)_col_, _key_); }
bool FUNC_PROXY(_contains_interval)(cvx_container *_col_, K _lo_, K _hi_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_contains_interval)((struct SNAME *)_col_, _lo_, _hi_); }
bool FUNC_PROXY(_overlaps)(cvx_container *_col_, K _lo_, K _hi_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_overlaps)((struct SNAME *)_col_, _lo_, _hi_); }
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
struct ENTRY FUNC_PROXY(_iter_entry)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, (struct ENTRY){ 0 }); return FUNC(_iter_entry)((struct ITERATOR *)_col_); }
K FUNC_PROXY(_iter_value_lo)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, (K){ 0 }); return FUNC(_iter_value_lo)((struct ITERATOR *)_col_); }
K FUNC_PROXY(_iter_value_hi)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, (K){ 0 }); return FUNC(_iter_value_hi)((struct ITERATOR *)_col_); }
V FUNC_PROXY(_iter_value_val)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, (V){ 0 }); return FUNC(_iter_value_val)((struct ITERATOR *)_col_); }
size_t FUNC_PROXY(_iter_index)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, 0); return FUNC(_iter_index)((struct ITERATOR *)_col_); }
// clang-format on

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
// before this header, because _iter_entry returns struct ENTRY.
#define INTERFACE IMPL_FORWARD_ITER

#define IMPL_START FUNC_PROXY(_iter_start)
#define IMPL_DROP FUNC_PROXY(_iter_drop)
#define IMPL_AT_START FUNC_PROXY(_iter_at_start)
#define IMPL_AT_END FUNC_PROXY(_iter_at_end)
#define IMPL_COUNT FUNC_PROXY(_iter_count)
#define IMPL_TO_START FUNC_PROXY(_iter_to_start)
#define IMPL_NEXT FUNC_PROXY(_iter_next)
#define IMPL_FORWARD FUNC_PROXY(_iter_forward)
#define IMPL_VALUE FUNC_PROXY(_iter_entry)
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
// NOTE: the IMPL_BIDIRECTIONAL_ITER interface must have been declared with
//   #define V  struct CVX_(SNAME,_entry)
//   #define INTERFACE <name>
//   #include "cvx/iter/bidirectional_iterator.h"
// before this header, because _iter_entry returns struct ENTRY.
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
#define IMPL_VALUE FUNC_PROXY(_iter_entry)
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
