/// Open-addressing, linear-probing hashtable with robin hood insertion.
///
/// Keys are hashed with vtabk->hash and compared with vtabk->comp.
/// Both are required; vtabk->clone / vtabk->drop and vtabv->clone / vtabv->drop
/// are optional.
///
/// Capacity is always a prime number.  A resize is triggered whenever
/// count >= capacity * load (default load = 0.7).  Deletions use tombstones;
/// tombstones are purged on resize.

#include "cvx/fallback.h"

// clang-format off
#ifndef K
#error "cvx/hashtable.h requires K to be defined (the key type, e.g. #define K int)"
#endif
#ifndef V
#error "cvx/hashtable.h requires V to be defined (the value type, e.g. #define V int)"
#endif
#ifndef SNAME
#error "cvx/hashtable.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_ht)"
#endif
#ifndef PFX
#error "cvx/hashtable.h requires PFX to be defined (the function prefix, e.g. #define PFX ht)"
#endif
#ifndef TAG
#error "cvx/hashtable.h requires TAG to be defined (a unique integer tag, e.g. #define TAG 1)"
#endif
// clang-format on

#include <stdbool.h>
#include <stdlib.h>

#include "cvx/core.h"

#define FUNC(X) CVX_(PFX, X)
#define FUNC_PROXY(X) CVX_(PFX, CVX_(__proxy, X))
#define ENTRY CVX_(SNAME, _entry)
#define ENTRY_STATE CVX_(SNAME, _entry_state)
#define ITERATOR CVX_(SNAME, _iter)
#define ITER_TAG (TAG * CVX_ITER_TAG_MULT)
#define VTAB_K CVX_(SNAME, _vtabk)
#define VTAB_V CVX_(SNAME, _vtabv)

// Prime numbers shared across all hashtable instantiations in this TU.
#ifndef CVX_HASHTABLE_PRIMES_DEFINED
#define CVX_HASHTABLE_PRIMES_DEFINED
// clang-format off
static const size_t cvx_hashtable_primes[] = {
    /* < 1e3  */ 53, 97, 191, 383, 769,
    /* < 1e4  */ 1531, 3067, 6143,
    /* < 1e5  */ 12289, 24571, 49157, 98299,
    /* < 1e6  */ 196613, 393209, 786431,
    /* < 1e7  */ 1572869, 3145721, 6291449,
    /* < 1e8  */ 12582917, 25165813, 50331653,
    /* < 1e9  */ 100663291, 201326611, 402653189, 805306357,
    /* < 1e10 */ 1610612741, 3221225473, 6442450939,
    /* < 1e11 */ 12884901893, 25769803799, 51539607551,
    /* < 1e12 */ 103079215111, 206158430209, 412316860441, 824633720831,
    /* < 1e13 */ 1649267441651, 3298534883309, 6597069766657,
    /* < 1e14 */ 13194139533299, 26388279066623, 52776558133303,
    /* < 1e15 */ 105553116266489, 211106232532969, 422212465066001, 844424930131963,
    /* < 1e16 */ 1688849860263953, 3377699720527861, 6755399441055731,
    /* < 1e17 */ 13510798882111483, 27021597764222939, 54043195528445957,
    /* < 1e18 */ 108086391056891903, 216172782113783773, 432345564227567621, 864691128455135207,
    /* < 1e19 */ 1729382256910270481, 3458764513820540933, 6917529027641081903,
    /* < 1e20 */ 13835058055282163729llu,
};
static const size_t cvx_hashtable_primes_count =
    sizeof(cvx_hashtable_primes) / sizeof(cvx_hashtable_primes[0]);
// clang-format on
#endif

// Entry state constants (used as the state field value).
// 0 = EMPTY (never filled), 1 = FILLED, -1 = DELETED (tombstone)
enum ENTRY_STATE
{
    CVX_(SNAME, _es_empty) = 0,
    CVX_(SNAME, _es_filled) = 1,
    CVX_(SNAME, _es_deleted) = -1,
};

struct VTAB_K
{
    CVX_VTAB_DEFINITION(K)
};

struct VTAB_V
{
    CVX_VTAB_DEFINITION(V)
};

struct ENTRY
{
    K key;
    V val;
    size_t dist;            // displacement from home slot (robin hood)
    enum ENTRY_STATE state; // empty / filled / deleted
};

struct SNAME
{
    cvx_container super;
    size_t capacity;      // total buffer slots
    size_t count;         // number of filled entries
    double load;          // resize threshold (count >= capacity * load)
    struct VTAB_K *vtabk; // hash and comp are required for operations
    struct VTAB_V *vtabv;
    struct ENTRY *buffer;
};

struct ITERATOR
{
    cvx_container super; // tag = ITER_TAG
    size_t cursor;       // buffer index of current entry; capacity = past-end
    size_t index;        // logical position [0, count]
    size_t first;        // buffer index of first filled entry at construction
    size_t last;         // buffer index of last filled entry at construction
    struct SNAME *target;
};

// ---- Initializers ----
void FUNC(_init)(struct SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv, size_t capacity);
void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone);

// ---- Destructor ----
void FUNC(_drop)(struct SNAME *self);

// ---- Getters ----
enum cvx_flags FUNC(_flag)(struct SNAME *_self_);
size_t FUNC(_count)(struct SNAME *_self_);
size_t FUNC(_capacity)(struct SNAME *_self_);
double FUNC(_load)(struct SNAME *_self_);
bool FUNC(_empty)(struct SNAME *_self_);

// ---- Operations ----
bool FUNC(_insert)(struct SNAME *_self_, K _key_, V _val_);
bool FUNC(_update)(struct SNAME *_self_, K _key_, V _new_, V *_old_);
bool FUNC(_remove)(struct SNAME *_self_, K _key_, V *_out_);
V FUNC(_get)(struct SNAME *_self_, K _key_);
V *FUNC(_get_ref)(struct SNAME *_self_, K _key_);
bool FUNC(_contains)(struct SNAME *_self_, K _key_);

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
K FUNC(_iter_key)(struct ITERATOR *_iter_);
V FUNC(_iter_value)(struct ITERATOR *_iter_);
size_t FUNC(_iter_index)(struct ITERATOR *_iter_);

// ---- Private helpers ----
struct ENTRY *FUNC(__get_entry)(struct SNAME *_self_, K _key_);
size_t FUNC(__next_prime)(size_t _required_);
bool FUNC(__resize)(struct SNAME *_self_, size_t _new_cap_);
void FUNC(__iter_scan_bounds)(struct SNAME *_target_, size_t *_first_, size_t *_last_);

///
///
/// IMPLEMENTATIONS
///
///

void FUNC(_init)(struct SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv, size_t capacity)
{
    *self = (struct SNAME){ 0 };

    if (!vtabk || !vtabk->hash || !vtabk->comp)
    {
        self->super.flag = CVX_FLAG_VTAB;
        return;
    }

    self->super.tag = TAG;
    self->super.flag = CVX_FLAG_OK;
    self->load = 0.7;
    self->vtabk = vtabk;
    self->vtabv = vtabv;

    if (capacity == 0)
        return;

    size_t _cap_ = FUNC(__next_prime)(capacity);
    struct ENTRY *_buf_ = malloc(sizeof(struct ENTRY) * _cap_);
    if (!_buf_)
    {
        self->super.flag = CVX_FLAG_ALLOC;
        return;
    }

    for (size_t _i_ = 0; _i_ < _cap_; _i_++)
        _buf_[_i_] = (struct ENTRY){ 0 };

    self->buffer = _buf_;
    self->capacity = _cap_;
}

void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone)
{
    FUNC(_init)(clone, orig->vtabk, orig->vtabv, 0);
    if (clone->super.flag != CVX_FLAG_OK)
        return;

    clone->load = orig->load;

    if (orig->count == 0)
        return;

    struct ENTRY *_buf_ = malloc(sizeof(struct ENTRY) * orig->capacity);
    if (!_buf_)
    {
        clone->super.flag = CVX_FLAG_ALLOC;
        orig->super.flag = CVX_FLAG_ALLOC;
        return;
    }

    for (size_t _i_ = 0; _i_ < orig->capacity; _i_++)
    {
        _buf_[_i_] = orig->buffer[_i_];
        if (orig->buffer[_i_].state == CVX_(SNAME, _es_filled))
        {
            _buf_[_i_].key = (orig->vtabk && orig->vtabk->clone)
                                 ? orig->vtabk->clone(orig->buffer[_i_].key)
                                 : orig->buffer[_i_].key;
            _buf_[_i_].val = (orig->vtabv && orig->vtabv->clone)
                                 ? orig->vtabv->clone(orig->buffer[_i_].val)
                                 : orig->buffer[_i_].val;
        }
    }

    clone->buffer = _buf_;
    clone->count = orig->count;
    clone->capacity = orig->capacity;
}

void FUNC(_drop)(struct SNAME *self)
{
    if (!self)
        return;

    for (size_t _i_ = 0; _i_ < self->capacity; _i_++)
    {
        if (self->buffer[_i_].state == CVX_(SNAME, _es_filled))
        {
            if (self->vtabk && self->vtabk->drop)
                self->vtabk->drop(self->buffer[_i_].key);
            if (self->vtabv && self->vtabv->drop)
                self->vtabv->drop(self->buffer[_i_].val);
        }
    }

    free(self->buffer);
    self->buffer = NULL;
    self->capacity = 0;
    self->count = 0;
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

double FUNC(_load)(struct SNAME *_self_)
{
    return _self_->load;
}

bool FUNC(_empty)(struct SNAME *_self_)
{
    return _self_->count == 0;
}

bool FUNC(_insert)(struct SNAME *_self_, K _key_, V _val_)
{
    if (!_self_->vtabk || !_self_->vtabk->hash || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return false;
    }

    if (FUNC(__get_entry)(_self_, _key_) != NULL)
    {
        _self_->super.flag = CVX_FLAG_DUPLICATE;
        return false;
    }

    if (_self_->capacity == 0 || (double)_self_->count >= (double)_self_->capacity * _self_->load)
    {
        size_t _need_ = (_self_->capacity == 0) ? 53 : _self_->capacity + 1;
        if (!FUNC(__resize)(_self_, _need_))
            return false;
    }

    K _k_ = _key_;
    V _v_ = _val_;
    size_t _orig_pos_ = _self_->vtabk->hash(_k_) % _self_->capacity;
    size_t _pos_ = _orig_pos_;

    while (true)
    {
        struct ENTRY *_e_ = &_self_->buffer[_pos_ % _self_->capacity];

        if (_e_->state != CVX_(SNAME, _es_filled)) // empty or deleted: claim it
        {
            _e_->key = _k_;
            _e_->val = _v_;
            _e_->dist = _pos_ - _orig_pos_;
            _e_->state = CVX_(SNAME, _es_filled);
            break;
        }

        if (_e_->dist < _pos_ - _orig_pos_) // robin hood: steal from rich
        {
            K _tmp_k_ = _e_->key;
            V _tmp_v_ = _e_->val;
            size_t _tmp_dist_ = _e_->dist;

            _e_->key = _k_;
            _e_->val = _v_;
            _e_->dist = _pos_ - _orig_pos_;

            _k_ = _tmp_k_;
            _v_ = _tmp_v_;
            _orig_pos_ = _pos_ - _tmp_dist_;
        }

        _pos_++;
    }

    _self_->count++;
    _self_->super.flag = CVX_FLAG_OK;
    return true;
}

bool FUNC(_update)(struct SNAME *_self_, K _key_, V _new_, V *_old_)
{
    if (!_self_->vtabk || !_self_->vtabk->hash || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return false;
    }

    struct ENTRY *_e_ = FUNC(__get_entry)(_self_, _key_);
    if (!_e_)
    {
        _self_->super.flag = CVX_FLAG_NOT_FOUND;
        return false;
    }

    if (_old_)
        *_old_ = _e_->val;
    else if (_self_->vtabv && _self_->vtabv->drop)
        _self_->vtabv->drop(_e_->val);

    _e_->val = _new_;
    _self_->super.flag = CVX_FLAG_OK;
    return true;
}

bool FUNC(_remove)(struct SNAME *_self_, K _key_, V *_out_)
{
    if (!_self_->vtabk || !_self_->vtabk->hash || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return false;
    }

    if (_self_->count == 0)
    {
        _self_->super.flag = CVX_FLAG_EMPTY;
        return false;
    }

    struct ENTRY *_e_ = FUNC(__get_entry)(_self_, _key_);
    if (!_e_)
    {
        _self_->super.flag = CVX_FLAG_NOT_FOUND;
        return false;
    }

    if (_out_)
        *_out_ = _e_->val;
    else if (_self_->vtabv && _self_->vtabv->drop)
        _self_->vtabv->drop(_e_->val);

    if (_self_->vtabk && _self_->vtabk->drop)
        _self_->vtabk->drop(_e_->key);

    _e_->key = (K){ 0 };
    _e_->val = (V){ 0 };
    _e_->dist = 0;
    _e_->state = CVX_(SNAME, _es_deleted);

    _self_->count--;
    _self_->super.flag = CVX_FLAG_OK;
    return true;
}

V FUNC(_get)(struct SNAME *_self_, K _key_)
{
    if (!_self_->vtabk || !_self_->vtabk->hash || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return (V){ 0 };
    }

    struct ENTRY *_e_ = FUNC(__get_entry)(_self_, _key_);
    if (!_e_)
    {
        _self_->super.flag = CVX_FLAG_NOT_FOUND;
        return (V){ 0 };
    }

    _self_->super.flag = CVX_FLAG_OK;
    return _e_->val;
}

V *FUNC(_get_ref)(struct SNAME *_self_, K _key_)
{
    if (!_self_->vtabk || !_self_->vtabk->hash || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return NULL;
    }

    struct ENTRY *_e_ = FUNC(__get_entry)(_self_, _key_);
    if (!_e_)
    {
        _self_->super.flag = CVX_FLAG_NOT_FOUND;
        return NULL;
    }

    _self_->super.flag = CVX_FLAG_OK;
    return &_e_->val;
}

bool FUNC(_contains)(struct SNAME *_self_, K _key_)
{
    if (!_self_->vtabk || !_self_->vtabk->hash || !_self_->vtabk->comp)
    {
        _self_->super.flag = CVX_FLAG_VTAB;
        return false;
    }

    _self_->super.flag = CVX_FLAG_OK;
    return FUNC(__get_entry)(_self_, _key_) != NULL;
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
    _res_.target = _target_;
    _res_.index = 0;

    FUNC(__iter_scan_bounds)(_target_, &_res_.first, &_res_.last);
    _res_.cursor = _res_.first; // capacity if empty
    return _res_;
}

struct ITERATOR FUNC(_iter_init_end)(struct SNAME *_target_)
{
    struct ITERATOR _res_ = { 0 };
    _res_.super.tag = ITER_TAG;
    _res_.super.flag = CVX_FLAG_OK;
    _res_.target = _target_;
    _res_.index = _target_->count;
    _res_.cursor = _target_->capacity; // past-end

    FUNC(__iter_scan_bounds)(_target_, &_res_.first, &_res_.last);
    return _res_;
}

struct ITERATOR *FUNC(_iter_start)(struct SNAME *_target_)
{
    struct ITERATOR *_res_ = malloc(sizeof(struct ITERATOR));
    if (!_res_)
        return NULL;

    *_res_ = FUNC(_iter_init_start)(_target_);
    return _res_;
}

struct ITERATOR *FUNC(_iter_end)(struct SNAME *_target_)
{
    struct ITERATOR *_res_ = malloc(sizeof(struct ITERATOR));
    if (!_res_)
        return NULL;

    *_res_ = FUNC(_iter_init_end)(_target_);
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
    _iter_->cursor = _iter_->first;
    _iter_->index = 0;
    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_to_end)(struct ITERATOR *_iter_)
{
    _iter_->cursor = _iter_->target->capacity;
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

    _iter_->cursor++;
    while (_iter_->cursor < _iter_->target->capacity &&
           _iter_->target->buffer[_iter_->cursor].state != CVX_(SNAME, _es_filled))
        _iter_->cursor++;

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

    // When at the past-end position, jump directly to the last filled entry.
    if (_iter_->index == _iter_->target->count)
    {
        _iter_->cursor = _iter_->last;
        _iter_->index--;
        _iter_->super.flag = CVX_FLAG_OK;
        return;
    }

    // Otherwise cursor is at a filled entry; scan backward to find the previous one.
    // cursor > 0 is guaranteed since index > 0 implies a filled entry exists before cursor.
    size_t _scan_ = _iter_->cursor;
    while (_scan_ > 0)
    {
        _scan_--;
        if (_iter_->target->buffer[_scan_].state == CVX_(SNAME, _es_filled))
            break;
    }

    _iter_->cursor = _scan_;
    _iter_->index--;
    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_forward)(struct ITERATOR *_iter_, size_t _steps_)
{
    size_t _remaining_ = _iter_->target->count - _iter_->index;
    size_t _actual_ = _steps_ < _remaining_ ? _steps_ : _remaining_;

    for (size_t _i_ = 0; _i_ < _actual_; _i_++)
        FUNC(_iter_next)(_iter_);

    _iter_->super.flag = CVX_FLAG_OK;
}

void FUNC(_iter_backward)(struct ITERATOR *_iter_, size_t _steps_)
{
    size_t _actual_ = _steps_ < _iter_->index ? _steps_ : _iter_->index;

    for (size_t _i_ = 0; _i_ < _actual_; _i_++)
        FUNC(_iter_prev)(_iter_);

    _iter_->super.flag = CVX_FLAG_OK;
}

K FUNC(_iter_key)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return (K){ 0 };
    }

    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->target->buffer[_iter_->cursor].key;
}

V FUNC(_iter_value)(struct ITERATOR *_iter_)
{
    if (_iter_->index >= _iter_->target->count)
    {
        _iter_->super.flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    _iter_->super.flag = CVX_FLAG_OK;
    return _iter_->target->buffer[_iter_->cursor].val;
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

// Linear probe search. Returns NULL when the key is absent or capacity == 0.
struct ENTRY *FUNC(__get_entry)(struct SNAME *_self_, K _key_)
{
    if (_self_->capacity == 0)
        return NULL;

    size_t _pos_ = _self_->vtabk->hash(_key_) % _self_->capacity;
    struct ENTRY *_e_ = &_self_->buffer[_pos_];

    while (_e_->state != CVX_(SNAME, _es_empty))
    {
        if (_e_->state == CVX_(SNAME, _es_filled) && _self_->vtabk->comp(_e_->key, _key_) == 0)
            return _e_;

        _pos_++;
        _e_ = &_self_->buffer[_pos_ % _self_->capacity];
    }

    return NULL;
}

// Returns the smallest prime in the table that is >= required.
// Falls back to required if it exceeds all primes.
size_t FUNC(__next_prime)(size_t _required_)
{
    for (size_t _i_ = 0; _i_ < cvx_hashtable_primes_count; _i_++)
    {
        if (cvx_hashtable_primes[_i_] >= _required_)
            return cvx_hashtable_primes[_i_];
    }

    return _required_;
}

// Reallocates to the next prime >= new_cap and re-inserts all filled entries.
// Sets CVX_FLAG_ALLOC and returns false on allocation failure.
bool FUNC(__resize)(struct SNAME *_self_, size_t _new_cap_)
{
    _new_cap_ = FUNC(__next_prime)(_new_cap_);

    struct ENTRY *_new_buf_ = malloc(sizeof(struct ENTRY) * _new_cap_);
    if (!_new_buf_)
    {
        _self_->super.flag = CVX_FLAG_ALLOC;
        return false;
    }

    for (size_t _i_ = 0; _i_ < _new_cap_; _i_++)
        _new_buf_[_i_] = (struct ENTRY){ 0 };

    // Re-insert every filled entry using robin hood into the new buffer.
    for (size_t _i_ = 0; _i_ < _self_->capacity; _i_++)
    {
        if (_self_->buffer[_i_].state != CVX_(SNAME, _es_filled))
            continue;

        K _k_ = _self_->buffer[_i_].key;
        V _v_ = _self_->buffer[_i_].val;
        size_t _orig_pos_ = _self_->vtabk->hash(_k_) % _new_cap_;
        size_t _pos_ = _orig_pos_;

        while (true)
        {
            struct ENTRY *_e_ = &_new_buf_[_pos_ % _new_cap_];

            if (_e_->state != CVX_(SNAME, _es_filled))
            {
                _e_->key = _k_;
                _e_->val = _v_;
                _e_->dist = _pos_ - _orig_pos_;
                _e_->state = CVX_(SNAME, _es_filled);
                break;
            }

            if (_e_->dist < _pos_ - _orig_pos_)
            {
                K _tmp_k_ = _e_->key;
                V _tmp_v_ = _e_->val;
                size_t _tmp_dist_ = _e_->dist;

                _e_->key = _k_;
                _e_->val = _v_;
                _e_->dist = _pos_ - _orig_pos_;

                _k_ = _tmp_k_;
                _v_ = _tmp_v_;
                _orig_pos_ = _pos_ - _tmp_dist_;
            }

            _pos_++;
        }
    }

    free(_self_->buffer);
    _self_->buffer = _new_buf_;
    _self_->capacity = _new_cap_;
    return true;
}

// Scans the buffer to find the first and last filled entry indices.
// Sets *first = capacity and *last = 0 when the container is empty.
void FUNC(__iter_scan_bounds)(struct SNAME *_target_, size_t *_first_, size_t *_last_)
{
    *_first_ = _target_->capacity;
    *_last_ = 0;

    if (_target_->capacity == 0 || _target_->count == 0)
        return;

    for (size_t _i_ = 0; _i_ < _target_->capacity; _i_++)
    {
        if (_target_->buffer[_i_].state == CVX_(SNAME, _es_filled))
        {
            if (*_first_ == _target_->capacity)
                *_first_ = _i_;
            *_last_ = _i_;
        }
    }
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
size_t FUNC_PROXY(_capacity)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, 0); return FUNC(_capacity)((struct SNAME *)_col_); }
bool FUNC_PROXY(_empty)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_empty)((struct SNAME *)_col_); }
bool FUNC_PROXY(_insert)(cvx_container *_col_, K _key_, V _val_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_insert)((struct SNAME *)_col_, _key_, _val_); }
bool FUNC_PROXY(_update)(cvx_container *_col_, K _key_, V _new_, V *_old_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_update)((struct SNAME *)_col_, _key_, _new_, _old_); }
bool FUNC_PROXY(_remove)(cvx_container *_col_, K _key_, V *_out_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_remove)((struct SNAME *)_col_, _key_, _out_); }
V FUNC_PROXY(_get)(cvx_container *_col_, K _key_) { CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 }); return FUNC(_get)((struct SNAME *)_col_, _key_); }
bool FUNC_PROXY(_contains)(cvx_container *_col_, K _key_) { CVX_CONTAINER_GUARDS(TAG, _col_, false); return FUNC(_contains)((struct SNAME *)_col_, _key_); }
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
K FUNC_PROXY(_iter_key)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, (K){ 0 }); return FUNC(_iter_key)((struct ITERATOR *)_col_); }
V FUNC_PROXY(_iter_value)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, (V){ 0 }); return FUNC(_iter_value)((struct ITERATOR *)_col_); }
size_t FUNC_PROXY(_iter_index)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(ITER_TAG, _col_, 0); return FUNC(_iter_index)((struct ITERATOR *)_col_); }
// clang-format on

///
///
/// INTERFACE CASTS
///
///

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
