/// interval_set.h
///
/// Status
///
///   [x] concept
///   [ ] v1
///   [ ] tests
///   [ ] refine
///   [ ] stabilize
///
/// Interval set with right-open intervals [lo, hi).
///

#include "cvx/fallback.h"
#include "cvx/flags.h"

// clang-format off
#ifndef V
#error "cvx/interval_set.h requires V to be defined (the domain element type, e.g. #define V int)"
#endif
#ifndef SNAME
#error "cvx/interval_set.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_iset)"
#endif
#ifndef PFX
#error "cvx/interval_set.h requires PFX to be defined (the function prefix, e.g. #define PFX ms)"
#endif
#ifndef TAG
#error "cvx/interval_set.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
#endif
// clang-format on

// Intervals are right-open: [lo, hi).
// Two intervals [a,b) and [c,d) overlap  iff comp(a,d)<0 && comp(c,b)<0.
// Two intervals [a,b) and [c,d) touch    iff comp(b,c)==0 or comp(d,a)==0.
// Both overlap and touch trigger merging on _add.
//
// vtabv->comp is required; all operations validate it and set CVX_FLAG_VTAB if
// absent.  vtabv->clone / vtabv->drop apply to individual V values (lo and hi
// each independently).  vtabv->prio, if set, is used as an extra adjacency
// predicate: prio(a,b)!=0 means b immediately follows a (e.g. b==a+1 for
// integers).  When prio is not set, only the natural touching condition
// comp(a,b)==0 is used.
//
// _iter_entry returns struct ENTRY (the interval), not V.  To use the generic
// iterator interfaces (IMPL_FORWARD_ITER / IMPL_BIDIRECTIONAL_ITER), declare
// the interface with V = struct CVX_(SNAME,_entry) BEFORE including this
// header.

#include <stdlib.h>
#include <string.h>

#include "cvx/core.h"

#define FUNC(X) CVX_(PFX, X)
#define FUNC_PROXY(X) CVX_(PFX, CVX_(__proxy, X))
#define ENTRY CVX_(SNAME, _entry)
#define ITERATOR CVX_(SNAME, _iter)
#define ITER_TAG (TAG * CVX_ITER_TAG_MULT)
#define VTAB_V CVX_(SNAME, _vtabv)

struct VTAB_V
{
    CVX_VTAB_DEFINITION(V)
};

// An interval [lo, hi) stored in the set's buffer.
struct ENTRY
{
    V lo;
    V hi;
};

struct SNAME
{
    cvx_container super;
    size_t capacity;      // allocated slots in buffer
    size_t count;         // number of intervals currently stored
    struct VTAB_V *vtabv; // vtabv->comp is required for all operations
    struct ENTRY *buffer; // sorted, non-overlapping interval array
};

struct ITERATOR
{
    cvx_container super; // tag = ITER_TAG
    size_t index;        // current interval index (count = past-end)
    struct SNAME *target;
};

// ---- Initializers ----
void FUNC(_init)(struct SNAME *self, struct VTAB_V *vtabv);
void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone);

// ---- Destructor ----
void FUNC(_drop)(struct SNAME *_self_);

// ---- Getters ----
enum cvx_flags FUNC(_flag)(struct SNAME *_self_);
size_t FUNC(_count)(struct SNAME *_self_);
bool FUNC(_empty)(struct SNAME *_self_);

// ---- Operations ----
void FUNC(_add)(struct SNAME *_self_, V _lo_, V _hi_);
void FUNC(_remove)(struct SNAME *_self_, V _lo_, V _hi_);
/// Returns true if val is covered by any interval in the set, i.e. there exists
/// [lo, hi) such that lo <= val < hi.  The hi endpoint is excluded (right-open).
/// Sets flag to CVX_FLAG_OK on success.
bool FUNC(_contains)(struct SNAME *_self_, V _val_);
/// Returns true if the interval [lo, hi) is fully covered by a single stored
/// interval, i.e. there exists [a, b) such that a <= lo and hi <= b.
/// Returns false for an empty query interval (lo >= hi).
/// Sets flag to CVX_FLAG_OK on success.
bool FUNC(_contains_interval)(struct SNAME *_self_, V _lo_, V _hi_);
/// Returns true if [lo, hi) has a non-empty intersection with any interval in
/// the set.  Touching intervals (hi of stored == lo of query, or vice-versa) do
/// NOT count as overlapping under right-open semantics.
/// Returns false for an empty query interval (lo >= hi).
/// Sets flag to CVX_FLAG_OK on success.
bool FUNC(_overlaps)(struct SNAME *_self_, V _lo_, V _hi_);

// ---- Set operations ----
/// Returns a new set containing all intervals from _left_ and _right_ (A ∪ B).
/// Both operands are left unchanged.  Returns NULL on allocation failure.
struct SNAME *FUNC(_union)(struct SNAME *_left_, struct SNAME *_right_);
/// Returns a new set containing only intervals covered by both _left_ and _right_ (A ∩ B).
/// Both operands are left unchanged.  Returns NULL on allocation failure.
struct SNAME *FUNC(_intersect)(struct SNAME *_left_, struct SNAME *_right_);
/// Returns a new set of intervals in _left_ that are not covered by _right_ (A \ B).
/// Both operands are left unchanged.  Returns NULL on allocation failure.
struct SNAME *FUNC(_diff)(struct SNAME *_left_, struct SNAME *_right_);
/// Returns a new set of intervals covered by exactly one of _left_ or _right_ ((A \ B) ∪ (B \ A)).
/// Both operands are left unchanged.  Returns NULL on allocation failure.
struct SNAME *FUNC(_symdiff)(struct SNAME *_left_, struct SNAME *_right_);
/// Returns a new set representing the complement of _self_ within the universe [_lo_, _hi_).
/// _self_ is left unchanged.  Returns NULL on allocation failure.  Returns an empty set
/// if _lo_ >= _hi_.
struct SNAME *FUNC(_compl)(struct SNAME *_self_, V _lo_, V _hi_);

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
V FUNC(_iter_value_lo)(struct ITERATOR *_iter_);
V FUNC(_iter_value_hi)(struct ITERATOR *_iter_);
size_t FUNC(_iter_index)(struct ITERATOR *_iter_);

// ---- Private helpers ----
bool FUNC(__assert_capacity)(struct SNAME *_self_);
size_t FUNC(__lower_bound)(struct SNAME *_self_, V _lo_);
bool FUNC(__append)(struct SNAME *_self_, V _lo_, V _hi_);

///
///
/// IMPLEMENTATIONS
///
///

void FUNC(_init)(struct SNAME *self, struct VTAB_V *vtabv)
{
    *self = (struct SNAME){ 0 };
    if (!vtabv || !vtabv->comp)
    {
        self->super.flag = CVX_FLAG_VTAB;
        return;
    }
    self->super.tag = TAG;
    self->super.flag = CVX_FLAG_OK;
    self->vtabv = vtabv;
}

void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone)
{
    FUNC(_init)(clone, orig->vtabv);
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
        if (orig->vtabv && orig->vtabv->clone)
        {
            _buf_[_i_].lo = orig->vtabv->clone(orig->buffer[_i_].lo);
            _buf_[_i_].hi = orig->vtabv->clone(orig->buffer[_i_].hi);
        }
        else
        {
            _buf_[_i_] = orig->buffer[_i_];
        }
    }

    clone->buffer = _buf_;
    clone->count = orig->count;
    clone->capacity = orig->count;
    orig->super.flag = CVX_FLAG_OK;
}

void FUNC(_drop)(struct SNAME *_self_)
{
    if (!_self_)
        return;
    if (_self_->vtabv && _self_->vtabv->drop)
    {
        for (size_t _i_ = 0; _i_ < _self_->count; _i_++)
        {
            _self_->vtabv->drop(_self_->buffer[_i_].lo);
            _self_->vtabv->drop(_self_->buffer[_i_].hi);
        }
    }
    free(_self_->buffer);
    _self_->buffer = NULL;
    _self_->capacity = 0;
    _self_->count = 0;
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

void FUNC(_add)(struct SNAME *_self_, V _lo_, V _hi_)
{
    if (!_self_->vtabv || !_self_->vtabv->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return;
    }

    if (_self_->vtabv->comp(_lo_, _hi_) >= 0)
    {
        _self_->super.flag = CVX_FLAG_INVALID;
        return;
    }

    if (!FUNC(__assert_capacity)(_self_))
        return;

    // Find the first existing interval whose upper bound reaches lo
    // (i.e., could overlap or touch [lo, hi) from the left).
    size_t _start_ = FUNC(__lower_bound)(_self_, _lo_);

    // Scan forward: collect all intervals that overlap OR touch [lo, hi).
    // For right-open intervals, touching means buffer[i].lo == hi (comp == 0).
    size_t _end_ = _start_;
    while (_end_ < _self_->count && _self_->vtabv->comp(_self_->buffer[_end_].lo, _hi_) <= 0)
    {
        _end_++;
    }
    // buffer[_start_ .. _end_-1] are the intervals to merge with [lo, hi).
    // If _end_ == _start_, no existing intervals overlap/touch: pure insert.

    if (_end_ == _start_)
    {
        // Pure insert at _start_: store caller's values directly (by value).
        memmove(&_self_->buffer[_start_ + 1], &_self_->buffer[_start_],
                (_self_->count - _start_) * sizeof(struct ENTRY));

        _self_->buffer[_start_] = (struct ENTRY){ .lo = _lo_, .hi = _hi_ };
        _self_->count++;
    }
    else
    {
        size_t _last_ = _end_ - 1;

        // Compute merged lo = min(_lo_, buffer[_start_].lo).
        // Values are stored by value (no copy call on _add); drop is called on
        // the discarded stored value when the incoming one wins.
        V _merged_lo_;
        if (_self_->vtabv->comp(_lo_, _self_->buffer[_start_].lo) < 0)
        {
            // Incoming lo is smaller: use it directly, drop the existing one.
            _merged_lo_ = _lo_;
            if (_self_->vtabv->drop)
                _self_->vtabv->drop(_self_->buffer[_start_].lo);
        }
        else
        {
            // Existing lo is smaller or equal: keep it.
            _merged_lo_ = _self_->buffer[_start_].lo;
        }

        // Compute merged hi = max(_hi_, buffer[_last_].hi).
        V _merged_hi_;
        if (_self_->vtabv->comp(_hi_, _self_->buffer[_last_].hi) > 0)
        {
            // Incoming hi is larger: use it directly, drop the existing one.
            _merged_hi_ = _hi_;
            if (_self_->vtabv->drop)
                _self_->vtabv->drop(_self_->buffer[_last_].hi);
        }
        else
        {
            // Existing hi is larger or equal: keep it.
            _merged_hi_ = _self_->buffer[_last_].hi;
        }

        // Drop the "inner" boundaries that are discarded by the merge.
        // When _start_ < _last_: buffer[_start_].hi and buffer[_last_].lo
        // are consumed; all entries in between are fully consumed.
        if (_start_ < _last_ && _self_->vtabv && _self_->vtabv->drop)
        {
            _self_->vtabv->drop(_self_->buffer[_start_].hi);

            for (size_t _i_ = _start_ + 1; _i_ < _last_; _i_++)
            {
                _self_->vtabv->drop(_self_->buffer[_i_].lo);
                _self_->vtabv->drop(_self_->buffer[_i_].hi);
            }

            _self_->vtabv->drop(_self_->buffer[_last_].lo);
        }

        // Write the merged entry at _start_, then compact.
        _self_->buffer[_start_] = (struct ENTRY){ .lo = _merged_lo_, .hi = _merged_hi_ };

        size_t _n_removed_ = _last_ - _start_;
        if (_n_removed_ > 0)
        {
            memmove(&_self_->buffer[_start_ + 1], &_self_->buffer[_last_ + 1],
                    (_self_->count - _last_ - 1) * sizeof(struct ENTRY));
            _self_->count -= _n_removed_;
        }
    }

    _self_->super.flag = CVX_FLAG_OK;
}

void FUNC(_remove)(struct SNAME *_self_, V _lo_, V _hi_)
{
    if (!_self_->vtabv || !_self_->vtabv->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return;
    }

    // Empty removal range is a no-op.
    if (_self_->vtabv->comp(_lo_, _hi_) >= 0)
    {
        _self_->super.flag = CVX_FLAG_OK;
        return;
    }

    // Ensure room for up to 2 residual entries in the worst case where a
    // single interval is split by the removal range (net +1 entry).
    if (!FUNC(__assert_capacity)(_self_))
        return;

    // Find first interval whose upper bound strictly exceeds _lo_
    // (i.e., overlaps [lo, hi): comp(buffer[i].hi, lo) > 0 means hi > lo).
    // Use __lower_bound which finds comp >= 0; then filter strict overlap
    // via the scan condition below.
    size_t _start_ = FUNC(__lower_bound)(_self_, _lo_);

    // Scan forward: collect intervals that strictly overlap [lo, hi).
    // Strict overlap: buffer[i].lo < hi (comp < 0).
    size_t _end_ = _start_;
    while (_end_ < _self_->count && _self_->vtabv->comp(_self_->buffer[_end_].lo, _hi_) < 0)
    {
        _end_++;
    }

    if (_end_ == _start_)
    {
        _self_->super.flag = CVX_FLAG_OK;
        return;
    }

    size_t _last_ = _end_ - 1;

    // Determine residuals.
    bool _has_left_ = _self_->vtabv->comp(_self_->buffer[_start_].lo, _lo_) < 0;
    bool _has_right_ = _self_->vtabv->comp(_hi_, _self_->buffer[_last_].hi) < 0;

    V _left_lo_, _left_hi_, _right_lo_, _right_hi_;

    if (_has_left_)
    {
        _left_lo_ = (_self_->vtabv->clone) ? _self_->vtabv->clone(_self_->buffer[_start_].lo)
                                           : _self_->buffer[_start_].lo;
        // left_hi = _lo_ (caller-owned value, copy it to store independently).
        _left_hi_ = (_self_->vtabv->clone) ? _self_->vtabv->clone(_lo_) : _lo_;
    }

    if (_has_right_)
    {
        // right_lo = _hi_ (caller-owned value, copy it to store independently).
        _right_lo_ = (_self_->vtabv->clone) ? _self_->vtabv->clone(_hi_) : _hi_;
        _right_hi_ = (_self_->vtabv->clone) ? _self_->vtabv->clone(_self_->buffer[_last_].hi)
                                            : _self_->buffer[_last_].hi;
    }

    // Drop all overlapping entries.
    if (_self_->vtabv && _self_->vtabv->drop)
    {
        for (size_t _i_ = _start_; _i_ < _end_; _i_++)
        {
            _self_->vtabv->drop(_self_->buffer[_i_].lo);
            _self_->vtabv->drop(_self_->buffer[_i_].hi);
        }
    }

    size_t _n_remove_ = _end_ - _start_;
    size_t _n_insert_ = (_has_left_ ? 1 : 0) + (_has_right_ ? 1 : 0);

    // Shift remaining entries to make room for residuals.
    memmove(&_self_->buffer[_start_ + _n_insert_], &_self_->buffer[_end_],
            (_self_->count - _end_) * sizeof(struct ENTRY));

    _self_->count = _self_->count - _n_remove_ + _n_insert_;

    size_t _idx_ = _start_;
    if (_has_left_)
        _self_->buffer[_idx_++] = (struct ENTRY){ .lo = _left_lo_, .hi = _left_hi_ };
    if (_has_right_)
        _self_->buffer[_idx_++] = (struct ENTRY){ .lo = _right_lo_, .hi = _right_hi_ };

    _self_->super.flag = CVX_FLAG_OK;
}

bool FUNC(_contains)(struct SNAME *_self_, V _val_)
{
    if (!_self_->vtabv || !_self_->vtabv->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return false;
    }

    _self_->super.flag = CVX_FLAG_OK;

    if (_self_->count == 0)
        return false;

    // Find rightmost interval where lo <= val.
    // upper_bound on lo: first index where buffer[i].lo > val.
    size_t _lo_idx_ = 0, _hi_idx_ = _self_->count;
    while (_lo_idx_ < _hi_idx_)
    {
        size_t _mid_ = _lo_idx_ + (_hi_idx_ - _lo_idx_) / 2;
        if (_self_->vtabv->comp(_self_->buffer[_mid_].lo, _val_) <= 0)
            _lo_idx_ = _mid_ + 1;
        else
            _hi_idx_ = _mid_;
    }
    // _lo_idx_ is now the first where buffer[i].lo > val.
    // _lo_idx_ - 1 (if >= 0) is the last where buffer[i].lo <= val.

    if (_lo_idx_ == 0)
        return false;

    size_t _i_ = _lo_idx_ - 1;
    // val is in [buffer[i].lo, buffer[i].hi) iff val < buffer[i].hi
    return _self_->vtabv->comp(_val_, _self_->buffer[_i_].hi) < 0;
}

bool FUNC(_contains_interval)(struct SNAME *_self_, V _lo_, V _hi_)
{
    if (!_self_->vtabv || !_self_->vtabv->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return false;
    }

    _self_->super.flag = CVX_FLAG_OK;

    if (_self_->vtabv->comp(_lo_, _hi_) >= 0 || _self_->count == 0)
        return false;

    // Find first interval whose hi could reach lo.
    size_t _start_ = FUNC(__lower_bound)(_self_, _lo_);

    if (_start_ >= _self_->count)
        return false;

    // [lo, hi) is fully contained in buffer[_start_] iff:
    //   buffer[_start_].lo <= lo  AND  hi <= buffer[_start_].hi
    return _self_->vtabv->comp(_self_->buffer[_start_].lo, _lo_) <= 0 &&
           _self_->vtabv->comp(_hi_, _self_->buffer[_start_].hi) <= 0;
}

bool FUNC(_overlaps)(struct SNAME *_self_, V _lo_, V _hi_)
{
    if (!_self_->vtabv || !_self_->vtabv->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return false;
    }

    _self_->super.flag = CVX_FLAG_OK;

    if (_self_->vtabv->comp(_lo_, _hi_) >= 0 || _self_->count == 0)
        return false;

    // First interval where hi >= lo.
    size_t _start_ = FUNC(__lower_bound)(_self_, _lo_);

    if (_start_ >= _self_->count)
        return false;

    // Strict overlap: buffer[_start_].hi > lo AND buffer[_start_].lo < hi.
    return _self_->vtabv->comp(_self_->buffer[_start_].hi, _lo_) > 0 &&
           _self_->vtabv->comp(_self_->buffer[_start_].lo, _hi_) < 0;
}

struct SNAME *FUNC(_union)(struct SNAME *_left_, struct SNAME *_right_)
{
    CVX_VTAB_COMP(_comp_, V) = NULL;

    if (_left_->vtabv && _left_->vtabv->comp)
        _comp_ = _left_->vtabv->comp;
    else if (_right_->vtabv && _right_->vtabv->comp)
        _comp_ = _right_->vtabv->comp;

    if (!_comp_)
    {
        _left_->super.flag = CVX_FLAG_VTAB;
        _right_->super.flag = CVX_FLAG_VTAB;
        return NULL;
    }

    struct SNAME *_res_ = malloc(sizeof(struct SNAME));
    if (!_res_)
        return NULL;
    FUNC(_init)(_res_, _left_->vtabv);

    size_t _i_ = 0, _j_ = 0;
    bool _active_ = false;
    V _cur_lo_ = (V){ 0 }, _cur_hi_ = (V){ 0 };

    while (_i_ < _left_->count || _j_ < _right_->count)
    {
        V _nlo_, _nhi_;
        if (_j_ >= _right_->count ||
            (_i_ < _left_->count &&
             _left_->vtabv->comp(_left_->buffer[_i_].lo, _right_->buffer[_j_].lo) <= 0))
        {
            _nlo_ = _left_->buffer[_i_].lo;
            _nhi_ = _left_->buffer[_i_].hi;
            _i_++;
        }
        else
        {
            _nlo_ = _right_->buffer[_j_].lo;
            _nhi_ = _right_->buffer[_j_].hi;
            _j_++;
        }

        if (!_active_)
        {
            _cur_lo_ = _nlo_;
            _cur_hi_ = _nhi_;
            _active_ = true;
        }
        else if (_left_->vtabv->comp(_nlo_, _cur_hi_) <= 0)
        {
            // overlapping or touching: extend if needed
            if (_left_->vtabv->comp(_nhi_, _cur_hi_) > 0)
                _cur_hi_ = _nhi_;
        }
        else
        {
            // disjoint: emit current, start new
            if (!FUNC(__append)(_res_, _cur_lo_, _cur_hi_))
            {
                FUNC(_drop)(_res_);
                free(_res_);
                return NULL;
            }
            _cur_lo_ = _nlo_;
            _cur_hi_ = _nhi_;
        }
    }

    if (_active_)
    {
        if (!FUNC(__append)(_res_, _cur_lo_, _cur_hi_))
        {
            FUNC(_drop)(_res_);
            free(_res_);
            return NULL;
        }
    }

    _res_->super.flag = CVX_FLAG_OK;
    return _res_;
}

struct SNAME *FUNC(_intersect)(struct SNAME *_left_, struct SNAME *_right_)
{
    CVX_VTAB_COMP(_comp_, V) = NULL;

    if (_left_->vtabv && _left_->vtabv->comp)
        _comp_ = _left_->vtabv->comp;
    else if (_right_->vtabv && _right_->vtabv->comp)
        _comp_ = _right_->vtabv->comp;

    if (!_comp_)
    {
        _left_->super.flag = CVX_FLAG_VTAB;
        _right_->super.flag = CVX_FLAG_VTAB;
        return NULL;
    }

    struct SNAME *_res_ = malloc(sizeof(struct SNAME));
    if (!_res_)
        return NULL;
    FUNC(_init)(_res_, _left_->vtabv);

    size_t _i_ = 0, _j_ = 0;

    while (_i_ < _left_->count && _j_ < _right_->count)
    {
        V _alo_ = _left_->buffer[_i_].lo;
        V _ahi_ = _left_->buffer[_i_].hi;
        V _blo_ = _right_->buffer[_j_].lo;
        V _bhi_ = _right_->buffer[_j_].hi;

        // overlap = [max(alo,blo), min(ahi,bhi))
        V _lo_ = _left_->vtabv->comp(_alo_, _blo_) >= 0 ? _alo_ : _blo_;
        V _hi_ = _left_->vtabv->comp(_ahi_, _bhi_) <= 0 ? _ahi_ : _bhi_;

        if (_left_->vtabv->comp(_lo_, _hi_) < 0)
        {
            if (!FUNC(__append)(_res_, _lo_, _hi_))
            {
                FUNC(_drop)(_res_);
                free(_res_);
                return NULL;
            }
        }

        // advance whichever interval ends first
        if (_left_->vtabv->comp(_ahi_, _bhi_) <= 0)
            _i_++;
        else
            _j_++;
    }

    _res_->super.flag = CVX_FLAG_OK;
    return _res_;
}

struct SNAME *FUNC(_diff)(struct SNAME *_left_, struct SNAME *_right_)
{
    CVX_VTAB_COMP(_comp_, V) = NULL;

    if (_left_->vtabv && _left_->vtabv->comp)
        _comp_ = _left_->vtabv->comp;
    else if (_right_->vtabv && _right_->vtabv->comp)
        _comp_ = _right_->vtabv->comp;

    if (!_comp_)
    {
        _left_->super.flag = CVX_FLAG_VTAB;
        _right_->super.flag = CVX_FLAG_VTAB;
        return NULL;
    }

    struct SNAME *_res_ = malloc(sizeof(struct SNAME));
    if (!_res_)
        return NULL;
    FUNC(_init)(_res_, _left_->vtabv);

    size_t _j_ = 0; // global right pointer; only advances forward

    for (size_t _i_ = 0; _i_ < _left_->count; _i_++)
    {
        V _cur_lo_ = _left_->buffer[_i_].lo;
        V _cur_hi_ = _left_->buffer[_i_].hi;

        // skip right intervals that end at or before cur_lo
        while (_j_ < _right_->count && _left_->vtabv->comp(_right_->buffer[_j_].hi, _cur_lo_) <= 0)
            _j_++;

        size_t _jj_ = _j_;
        while (_jj_ < _right_->count && _left_->vtabv->comp(_right_->buffer[_jj_].lo, _cur_hi_) < 0)
        {
            // right[jj] overlaps [cur_lo, cur_hi)
            if (_left_->vtabv->comp(_right_->buffer[_jj_].lo, _cur_lo_) > 0)
            {
                // gap before right[jj]
                if (!FUNC(__append)(_res_, _cur_lo_, _right_->buffer[_jj_].lo))
                {
                    FUNC(_drop)(_res_);
                    free(_res_);
                    return NULL;
                }
            }
            if (_left_->vtabv->comp(_right_->buffer[_jj_].hi, _cur_lo_) > 0)
                _cur_lo_ = _right_->buffer[_jj_].hi;
            _jj_++;
        }

        if (_left_->vtabv->comp(_cur_lo_, _cur_hi_) < 0)
        {
            if (!FUNC(__append)(_res_, _cur_lo_, _cur_hi_))
            {
                FUNC(_drop)(_res_);
                free(_res_);
                return NULL;
            }
        }
    }

    _res_->super.flag = CVX_FLAG_OK;
    return _res_;
}

struct SNAME *FUNC(_symdiff)(struct SNAME *_left_, struct SNAME *_right_)
{
    struct SNAME *_ab_ = FUNC(_diff)(_left_, _right_);
    if (!_ab_)
        return NULL;

    struct SNAME *_ba_ = FUNC(_diff)(_right_, _left_);
    if (!_ba_)
    {
        FUNC(_drop)(_ab_);
        free(_ab_);
        return NULL;
    }

    struct SNAME *_res_ = FUNC(_union)(_ab_, _ba_);
    FUNC(_drop)(_ab_);
    free(_ab_);
    FUNC(_drop)(_ba_);
    free(_ba_);
    return _res_;
}

struct SNAME *FUNC(_compl)(struct SNAME *_self_, V _lo_, V _hi_)
{
    if (!_self_->vtabv || !_self_->vtabv->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return NULL;
    }

    struct SNAME *_res_ = malloc(sizeof(struct SNAME));
    if (!_res_)
        return NULL;
    FUNC(_init)(_res_, _self_->vtabv);

    if (_self_->vtabv->comp(_lo_, _hi_) >= 0)
    {
        _res_->super.flag = CVX_FLAG_OK;
        return _res_;
    }

    V _cur_ = _lo_;

    for (size_t _i_ = 0; _i_ < _self_->count; _i_++)
    {
        // skip intervals entirely before [lo, hi)
        if (_self_->vtabv->comp(_self_->buffer[_i_].hi, _lo_) <= 0)
            continue;
        // stop if interval starts at or after hi
        if (_self_->vtabv->comp(_self_->buffer[_i_].lo, _hi_) >= 0)
            break;

        // gap = [cur, min(buffer[i].lo, hi))
        V _gap_hi_ =
            _self_->vtabv->comp(_self_->buffer[_i_].lo, _hi_) < 0 ? _self_->buffer[_i_].lo : _hi_;

        if (_self_->vtabv->comp(_cur_, _gap_hi_) < 0)
        {
            if (!FUNC(__append)(_res_, _cur_, _gap_hi_))
            {
                FUNC(_drop)(_res_);
                free(_res_);
                return NULL;
            }
        }

        if (_self_->vtabv->comp(_self_->buffer[_i_].hi, _cur_) > 0)
            _cur_ = _self_->buffer[_i_].hi;

        if (_self_->vtabv->comp(_cur_, _hi_) >= 0)
            break;
    }

    if (_self_->vtabv->comp(_cur_, _hi_) < 0)
    {
        if (!FUNC(__append)(_res_, _cur_, _hi_))
        {
            FUNC(_drop)(_res_);
            free(_res_);
            return NULL;
        }
    }

    _res_->super.flag = CVX_FLAG_OK;
    return _res_;
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

V FUNC(_iter_value_lo)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->target->buffer[_iter_->index].lo;
}

V FUNC(_iter_value_hi)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->target->buffer[_iter_->index].hi;
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

bool FUNC(__append)(struct SNAME *_self_, V _lo_, V _hi_)
{
    if (!FUNC(__assert_capacity)(_self_))
        return false;

    _self_->buffer[_self_->count++] = (struct ENTRY){
        .lo = (_self_->vtabv && _self_->vtabv->clone) ? _self_->vtabv->clone(_lo_) : _lo_,
        .hi = (_self_->vtabv && _self_->vtabv->clone) ? _self_->vtabv->clone(_hi_) : _hi_,
    };

    return true;
}

// Returns the index of the first interval whose hi >= _lo_ (i.e., the first
// that could overlap or touch [_lo_, ...) from the left).
// Returns self->count if no such interval exists.
size_t FUNC(__lower_bound)(struct SNAME *_self_, V _lo_)
{
    size_t _lo_idx_ = 0, _hi_idx_ = _self_->count;
    while (_lo_idx_ < _hi_idx_)
    {
        size_t _mid_ = _lo_idx_ + (_hi_idx_ - _lo_idx_) / 2;
        if (_self_->vtabv->comp(_self_->buffer[_mid_].hi, _lo_) < 0)
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
void FUNC_PROXY(_add)(cvx_container *_col_, V _lo_, V _hi_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_add)((struct SNAME *)_col_, _lo_, _hi_); }
void FUNC_PROXY(_remove)(cvx_container *_col_, V _lo_, V _hi_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_remove)((struct SNAME *)_col_, _lo_, _hi_); }
bool FUNC_PROXY(_contains)(cvx_container *_col_, V _val_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_contains)((struct SNAME *)_col_, _val_); }
bool FUNC_PROXY(_contains_interval)(cvx_container *_col_, V _lo_, V _hi_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_contains_interval)((struct SNAME *)_col_, _lo_, _hi_); }
bool FUNC_PROXY(_overlaps)(cvx_container *_col_, V _lo_, V _hi_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_overlaps)((struct SNAME *)_col_, _lo_, _hi_); }
cvx_container *FUNC_PROXY(_union)(cvx_container *_l_, cvx_container *_r_) { CVX_CONTAINER_GUARDS(TAG, _l_, NULL); CVX_CONTAINER_GUARDS(TAG, _r_, NULL); return (cvx_container *)FUNC(_union)((struct SNAME *)_l_, (struct SNAME *)_r_); }
cvx_container *FUNC_PROXY(_intersect)(cvx_container *_l_, cvx_container *_r_) { CVX_CONTAINER_GUARDS(TAG, _l_, NULL); CVX_CONTAINER_GUARDS(TAG, _r_, NULL); return (cvx_container *)FUNC(_intersect)((struct SNAME *)_l_, (struct SNAME *)_r_); }
cvx_container *FUNC_PROXY(_diff)(cvx_container *_l_, cvx_container *_r_) { CVX_CONTAINER_GUARDS(TAG, _l_, NULL); CVX_CONTAINER_GUARDS(TAG, _r_, NULL); return (cvx_container *)FUNC(_diff)((struct SNAME *)_l_, (struct SNAME *)_r_); }
cvx_container *FUNC_PROXY(_symdiff)(cvx_container *_l_, cvx_container *_r_) { CVX_CONTAINER_GUARDS(TAG, _l_, NULL); CVX_CONTAINER_GUARDS(TAG, _r_, NULL); return (cvx_container *)FUNC(_symdiff)((struct SNAME *)_l_, (struct SNAME *)_r_); }
cvx_container *FUNC_PROXY(_compl)(cvx_container *_col_, V _lo_, V _hi_) { CVX_CONTAINER_GUARDS(TAG, _col_, NULL); return (cvx_container *)FUNC(_compl)((struct SNAME *)_col_, _lo_, _hi_); }
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
V FUNC_PROXY(_iter_value_lo)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, (V){ 0 }); return FUNC(_iter_value_lo)((struct ITERATOR *)_col_); }
V FUNC_PROXY(_iter_value_hi)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, (V){ 0 }); return FUNC(_iter_value_hi)((struct ITERATOR *)_col_); }
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
// before this header is included, because _iter_entry returns struct ENTRY.
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
// before this header is included, because _iter_entry returns struct ENTRY.
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
