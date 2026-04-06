#include <stdlib.h>
#include <string.h>

#include "cvx/core.h"

#define FUNC(X) CVX_(PFX, X)
#define ITERATOR CVX_(SNAME, _iter)
#define ITER_TAG (TAG * CVX_ITER_TAG_MULT)

struct SNAME
{
    cvx_container super;
    V *buffer;
    size_t capacity;
    size_t count;
};

struct ITERATOR
{
    cvx_container super;
    size_t index;
    struct SNAME *target;
};

// Non-allocating initializers
struct SNAME FUNC(_init)(void);
struct SNAME FUNC(_init_with)(size_t capacity);
struct SNAME FUNC(_copy)(struct SNAME *_self_);

// Allocating initializers
cvx_container *FUNC(_new)(void);
cvx_container *FUNC(_new_with)(size_t capacity);
cvx_container *FUNC(_clone)(cvx_container *_col_);

// Destructors
void FUNC(_drop)(cvx_container *_col_);
void FUNC(_clear)(cvx_container *_col_);

// Getters
size_t FUNC(_count)(cvx_container *_col_);
size_t FUNC(_capacity)(cvx_container *_col_);
bool FUNC(_full)(cvx_container *_col_);
bool FUNC(_empty)(cvx_container *_col_);
V FUNC(_front)(cvx_container *_col_);
V FUNC(_back)(cvx_container *_col_);
V FUNC(_get)(cvx_container *_col_, size_t index);

// Implementation
void FUNC(_push_front)(cvx_container *_col_, V item);
void FUNC(_push_at)(cvx_container *_col_, V item, size_t index);
void FUNC(_push_back)(cvx_container *_col_, V item);
V FUNC(_pop_front)(cvx_container *_col_);
V FUNC(_pop_at)(cvx_container *_col_, size_t index);
V FUNC(_pop_back)(cvx_container *_col_);
V FUNC(_replace_front)(cvx_container *_col_, V new);
V FUNC(_replace_back)(cvx_container *_col_, V new);
// TODO:
// void FUNC(_seq_push_front)(cvx_container *_col_, V *values, size_t size);
// void FUNC(_seq_push_back)(cvx_container *_col_, V *values, size_t size);
// cvx_contaier *FUNC(_sublist)(cvx_container *_col_, size_t from, size_t to);
// bool FUNC(_contains)(cvx_container *_col_, V item);
// void FUNC(_index_of)(cvx_container *_col_, V item, size_t *out);

// Private functions
bool FUNC(__assert_capacity)(cvx_container *_col_);

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
void FUNC(_iter_forward)(cvx_container *_iter_, size_t steps);
void FUNC(_iter_backward)(cvx_container *_iter_, size_t steps);
void FUNC(_iter_go_to)(cvx_container *_iter_, size_t index);
// Iterator access
V FUNC(_iter_value)(cvx_container *_iter_);
size_t FUNC(_iter_index)(cvx_container *_iter_);

struct SNAME FUNC(_init)(void)
{
    struct SNAME _res_ = (struct SNAME){ 0 };
    _res_.super.tag = TAG;

    return _res_;
}

struct SNAME FUNC(_init_with)(size_t capacity)
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

    return _res_;
}

struct SNAME FUNC(_copy)(struct SNAME *_self_)
{
    struct SNAME _res_ = FUNC(_init)();
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

#ifdef V_COPY
    for (size_t i = 0; i < _self_->count; i++)
        _res_.buffer[i] = V_COPY(_self_->buffer[i]);
#else
    memcpy(_res_.buffer, _self_->buffer, _self_->count * sizeof(V));
#endif

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
    _res_->buffer = NULL;

    return (cvx_container *)_res_;
}

cvx_container *FUNC(_new_with)(size_t capacity)
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

    if (_orig_->count == 0)
    {
        _res_->flag = CVX_FLAG_OK;
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

#ifdef V_COPY
    for (size_t i = 0; i < _orig_->count; i++)
        _copy_->buffer[i] = V_COPY(_orig_->buffer[i]);
#else
    memcpy(_copy_->buffer, _orig_->buffer, _orig_->count * sizeof(V));
#endif

    _res_->flag = CVX_FLAG_OK;
    return _res_;
}

void FUNC(_drop)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

#ifdef V_DROP
    for (size_t i = 0; i < _self_->count; i++)
        V_DROP(_self_->buffer[i]);
#endif

    free(_self_->buffer);
    free(_self_);
}

void FUNC(_clear)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

#ifdef V_DROP
    for (size_t i = 0; i < _self_->count; i++)
    {
        V_DROP(_self_->buffer[i]);
    }
#endif

    free(_self_->buffer);
    _self_->buffer = NULL;
    _self_->capacity = 0;
    _self_->count = 0;
    _col_->flag = CVX_FLAG_OK;
}

void FUNC(_push_front)(cvx_container *_col_, V item)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    if (!FUNC(__assert_capacity)(_col_))
        return;

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_self_->count > 0)
    {
        memmove(_self_->buffer + 1, _self_->buffer, _self_->count * sizeof(V));
    }

    _self_->buffer[0] = item;

    _self_->count++;
    _col_->flag = CVX_FLAG_OK;
}

void FUNC(_push_at)(cvx_container *_col_, V item, size_t index)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (index > _self_->count)
    {
        _col_->flag = CVX_FLAG_RANGE;
        return;
    }

    if (!FUNC(__assert_capacity)(_col_))
        return;

    memmove(_self_->buffer + index + 1, _self_->buffer + index,
            (_self_->count - index) * sizeof(V));

    _self_->buffer[index] = item;
    _self_->count++;
    _col_->flag = CVX_FLAG_OK;
}

void FUNC(_push_back)(cvx_container *_col_, V item)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, );

    if (!FUNC(__assert_capacity)(_col_))
        return;

    struct SNAME *_self_ = (struct SNAME *)_col_;

    _self_->buffer[_self_->count++] = item;
    _col_->flag = CVX_FLAG_OK;
}

V FUNC(_pop_front)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_self_->count == 0)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _val_ = _self_->buffer[0];

    memmove(_self_->buffer, _self_->buffer + 1, (_self_->count - 1) * sizeof(V));

    _self_->buffer[_self_->count - 1] = (V){ 0 };
    _self_->count--;
    _col_->flag = CVX_FLAG_OK;
    return _val_;
}

V FUNC(_pop_at)(cvx_container *_col_, size_t index)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_self_->count == 0)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    if (index >= _self_->count)
    {
        _col_->flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    V _val_ = _self_->buffer[index];

    memmove(_self_->buffer + index, _self_->buffer + index + 1,
            (_self_->count - index - 1) * sizeof(V));

    _self_->buffer[_self_->count - 1] = (V){ 0 };
    _self_->count--;
    _col_->flag = CVX_FLAG_OK;
    return _val_;
}

V FUNC(_pop_back)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_self_->count == 0)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _val_ = _self_->buffer[_self_->count - 1];

    _self_->buffer[_self_->count - 1] = (V){ 0 };
    _self_->count--;
    _col_->flag = CVX_FLAG_OK;
    return _val_;
}

V FUNC(_replace_front)(cvx_container *_col_, V new)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_self_->count == 0)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = _self_->buffer[0];
    _self_->buffer[0] = new;
    _col_->flag = CVX_FLAG_OK;
    return _old_;
}

V FUNC(_replace_back)(cvx_container *_col_, V new)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_self_->count == 0)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    V _old_ = _self_->buffer[_self_->count - 1];
    _self_->buffer[_self_->count - 1] = new;
    _col_->flag = CVX_FLAG_OK;
    return _old_;
}

size_t FUNC(_count)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, 0);

    return ((struct SNAME *)_col_)->count;
}

size_t FUNC(_capacity)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, 0);

    return ((struct SNAME *)_col_)->capacity;
}

bool FUNC(_full)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, false);

    struct SNAME *_self_ = (struct SNAME *)_col_;

    return _self_->count >= _self_->capacity;
}

bool FUNC(_empty)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, false);

    struct SNAME *_self_ = (struct SNAME *)_col_;

    return _self_->count == 0;
}

V FUNC(_front)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_self_->count == 0 || _self_->buffer == NULL)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    return _self_->buffer[0];
}

V FUNC(_back)(cvx_container *_col_)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (_self_->count == 0 || _self_->buffer == NULL)
    {
        _col_->flag = CVX_FLAG_EMPTY;
        return (V){ 0 };
    }

    return _self_->buffer[_self_->count - 1];
}

V FUNC(_get)(cvx_container *_col_, size_t index)
{
    CVX_CONTAINER_GUARDS(TAG, _col_, (V){ 0 });

    struct SNAME *_self_ = (struct SNAME *)_col_;

    if (index >= _self_->count)
    {
        _col_->flag = CVX_FLAG_RANGE;
        return (V){ 0 };
    }

    return _self_->buffer[index];
}

///
///
/// ITERATOR
///
///

struct ITERATOR FUNC(_iter_init_start)(cvx_container *_target_)
{
    struct ITERATOR _res_ = { 0 };

    if (_target_->tag != TAG)
    {
        _res_.super.flag = CVX_FLAG_WRONG_TAG;
        return _res_;
    }

    _res_.super.tag = ITER_TAG;
    _res_.target = (struct SNAME *)_target_;
    _res_.index = 0;
    _res_.super.flag = CVX_FLAG_OK;

    return _res_;
}

struct ITERATOR FUNC(_iter_init_end)(cvx_container *_target_)
{
    struct ITERATOR _res_ = { 0 };

    if (_target_->tag != TAG)
    {
        _res_.super.flag = CVX_FLAG_WRONG_TAG;
        return _res_;
    }

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

    _res_->super.tag = ITER_TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->index = 0;
    _res_->target = (struct SNAME *)_target_;

    return (cvx_container *)_res_;
}

cvx_container *FUNC(_iter_end)(cvx_container *_target_)
{
    CVX_CONTAINER_GUARDS(TAG, _target_, NULL);

    struct ITERATOR *_res_ = malloc(sizeof(struct ITERATOR));

    if (!_res_)
        return NULL;

    _res_->super.tag = ITER_TAG;
    _res_->super.flag = CVX_FLAG_OK;
    _res_->target = (struct SNAME *)_target_;
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

    return ((struct ITERATOR *)_iter_)->index == 0;
}

bool FUNC(_iter_at_end)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, false);

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    return _self_->index == _self_->target->count;
}

size_t FUNC(_iter_count)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, 0);

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    return _self_->target->count;
}

void FUNC(_iter_to_start)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    ((struct ITERATOR *)_iter_)->index = 0;

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

void FUNC(_iter_forward)(cvx_container *_iter_, size_t steps)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    size_t remaining = _self_->target->count - _self_->index;
    _self_->index += (steps < remaining) ? steps : remaining;
    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_backward)(cvx_container *_iter_, size_t steps)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    _self_->index -= (steps < _self_->index) ? steps : _self_->index;
    _iter_->flag = CVX_FLAG_OK;
}

void FUNC(_iter_go_to)(cvx_container *_iter_, size_t index)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, );

    struct ITERATOR *_self_ = (struct ITERATOR *)_iter_;

    if (index > _self_->target->count)
    {
        _iter_->flag = CVX_FLAG_RANGE;
        return;
    }

    _self_->index = index;
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

    return _self_->target->buffer[_self_->index];
}

size_t FUNC(_iter_index)(cvx_container *_iter_)
{
    CVX_CONTAINER_GUARDS(ITER_TAG, _iter_, 0);

    return ((struct ITERATOR *)_iter_)->index;
}

///
///
/// PRIVATE FUNCTIONS
///
///

bool FUNC(__assert_capacity)(cvx_container *_col_)
{
    struct SNAME *_self_ = (struct SNAME *)_col_;

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
            _col_->flag = CVX_FLAG_ALLOC;
            return false;
        }
    }
    else
    {
        // Reallocation
        _self_->capacity *= 2;
        V *new_buffer = realloc(_self_->buffer, sizeof(V) * _self_->capacity);

        if (!new_buffer)
        {
            _col_->flag = CVX_FLAG_ALLOC;
            return false;
        }

        _self_->buffer = new_buffer;
    }

    return true;
}

#ifdef IMPL_STACK
#define IMPL_NEW FUNC(_new)
#define IMPL_DROP FUNC(_drop)
#define IMPL_CLONE FUNC(_clone)
#define IMPL_PUSH FUNC(_push_back)
#define IMPL_POP FUNC(_pop_back)
#define IMPL_COUNT FUNC(_count)
#define IMPL_PEEK FUNC(_back)
#define IMPL_REPLACE FUNC(_replace_back)
#include "cvx/istack_cast.h"
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
