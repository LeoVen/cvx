/// binary_heap.h
///
/// Status: refine
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

// ---- Initializers and destructors ----
void FUNC(_init)(struct SNAME *self, struct VTAB_V *vtabv, enum cvx_heap_order HO, size_t capacity);
void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone);
void FUNC(_drop)(struct SNAME *_self_);

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
bool FUNC(__assert_buffer)(struct SNAME *self, size_t capacity);

///
///
/// IMPLEMENTATIONS
///
///

void FUNC(_init)(struct SNAME *self, struct VTAB_V *vtabv, enum cvx_heap_order HO, size_t capacity)
{
    *self = (struct SNAME){ 0 };
    if (!vtabv || !vtabv->comp)
    {
        self->super.flag = CVX_FLAG_VTAB;
        return;
    }
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
    self->vtabv = vtabv;
    self->HO = HO;
}

void FUNC(_clone)(struct SNAME *orig, struct SNAME *clone)
{
    FUNC(_init)(clone, orig->vtabv, orig->HO, orig->capacity);
    if (clone->super.flag != CVX_FLAG_OK)
    {
        orig->super.flag = clone->super.flag;
        return;
    }
    if (orig->count == 0 || orig->capacity == 0)
        return;
    for (size_t i = 0; i < orig->count; i++)
    {
        clone->buffer[i] = (orig->vtabv && orig->vtabv->clone) ? orig->vtabv->clone(orig->buffer[i])
                                                               : orig->buffer[i];
    }
    clone->count = orig->count;
    clone->capacity = orig->count;
    orig->super.flag = CVX_FLAG_OK;
}

void FUNC(_drop)(struct SNAME *self)
{
    if (!self || !self->buffer)
        return;
    if (self->vtabv && self->vtabv->drop)
    {
        for (size_t _i_ = 0; _i_ < self->count; _i_++)
            self->vtabv->drop(self->buffer[_i_]);
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
void FUNC_PROXY(_clone)(cvx_container *_orig_, cvx_container *_clone_) { CVX_CONTAINER_GUARDS(TAG, _orig_, ); FUNC(_clone)((struct SNAME *)_orig_, (struct SNAME *)_clone_); }
void FUNC_PROXY(_drop)(cvx_container *_col_) { CVX_CONTAINER_GUARDS(TAG, _col_, ); FUNC(_drop)((struct SNAME *)_col_); }
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
