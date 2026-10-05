#include "hashtable.h"

#include <stdlib.h>

// ---- Private per-variant interface ----
// Both collision variants implement exactly this set of `static` helpers
// with matching signatures; everything else in this file (the public API
// below) is axis-independent and written once.
static enum cvx_flags FUNC(__init_buffer)(struct CVX_SNAME *self, size_t capacity);
static void FUNC(__drop_buffer)(struct CVX_SNAME *self);
static enum cvx_flags FUNC(__clone_buffer)(struct CVX_SNAME *orig, struct CVX_SNAME *clone);
static CVX_VAL *FUNC(__get_ref)(struct CVX_SNAME *self, CVX_KEY key);
static void FUNC(__insert_unchecked)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL val);
static enum cvx_flags FUNC(__resize)(struct CVX_SNAME *self, size_t new_cap);
static enum cvx_flags FUNC(__remove)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out);

// ---- Shared prime table (both variants round capacity up to a prime) ----
// clang-format off
static const size_t CVX_(CVX_PFX, __primes)[] = {
    /* < 1e3  */ 53, 97, 191, 383, 769,
    /* < 1e4  */ 1531, 3067, 6143,
    /* < 1e5  */ 12289, 24571, 49157, 98299,
    /* < 1e6  */ 196613, 393209, 786431,
    /* < 1e7  */ 1572869, 3145721, 6291449,
    /* < 1e8  */ 12582917, 25165813, 50331653,
    /* < 1e9  */ 100663291, 201326611, 402653189, 805306357,
    /* < 1e10 */ 1610612741, 3221225473, 6442450939,
    /* < 1e11 */ 12884901893, 25769803799, 51539607551,
};
// clang-format on
static const size_t CVX_(CVX_PFX, __primes_count) =
    sizeof(CVX_(CVX_PFX, __primes)) / sizeof(CVX_(CVX_PFX, __primes)[0]);

// Returns the smallest prime in the table that is >= required.
// Falls back to required if it exceeds all primes.
static size_t FUNC(__next_prime)(size_t required)
{
    for (size_t i = 0; i < CVX_(CVX_PFX, __primes_count); i++)
    {
        if (CVX_(CVX_PFX, __primes)[i] >= required)
            return CVX_(CVX_PFX, __primes)[i];
    }
    return required;
}

///
///
/// COLLISION VARIANTS
///
///

// @cvx2:variant axis="collision" name="open_addressing"
#ifdef CVX2_COLLISION_OPEN_ADDRESSING

// Entry state constants (used as the `state` field value). File-scope and
// unqualified is fine here: each generated instantiation is its own
// translation unit, so these can't collide across instantiations the way a
// header-level definition could.
enum
{
    CVX2_HT_ENTRY_EMPTY = 0,
    CVX2_HT_ENTRY_FILLED = 1,
    CVX2_HT_ENTRY_DELETED = -1,
};

static struct ENTRY *FUNC(__find_entry)(struct CVX_SNAME *self, CVX_KEY key)
{
    if (self->capacity == 0)
        return NULL;

    size_t pos = self->vtabk->hash(key) % self->capacity;
    struct ENTRY *e = &self->buffer[pos];

    while (e->state != CVX2_HT_ENTRY_EMPTY)
    {
        if (e->state == CVX2_HT_ENTRY_FILLED && self->vtabk->comp(e->key, key) == 0)
            return e;
        pos++;
        e = &self->buffer[pos % self->capacity];
    }

    return NULL;
}

static enum cvx_flags FUNC(__init_buffer)(struct CVX_SNAME *self, size_t capacity)
{
    size_t cap = FUNC(__next_prime)(capacity);
    struct ENTRY *buf = malloc(sizeof(struct ENTRY) * cap);
    if (!buf)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < cap; i++)
        buf[i] = (struct ENTRY){ 0 };

    self->buffer = buf;
    self->capacity = cap;
    return CVX_FLAG_OK;
}

static void FUNC(__drop_buffer)(struct CVX_SNAME *self)
{
    if (!self->buffer)
        return;

    for (size_t i = 0; i < self->capacity; i++)
    {
        if (self->buffer[i].state == CVX2_HT_ENTRY_FILLED)
        {
            if (self->vtabk && self->vtabk->drop)
                self->vtabk->drop(self->buffer[i].key);
            if (self->vtabv && self->vtabv->drop)
                self->vtabv->drop(self->buffer[i].val);
        }
    }

    free(self->buffer);
    self->buffer = NULL;
}

static enum cvx_flags FUNC(__clone_buffer)(struct CVX_SNAME *orig, struct CVX_SNAME *clone)
{
    if (!orig->buffer)
        return CVX_FLAG_OK;

    struct ENTRY *buf = malloc(sizeof(struct ENTRY) * orig->capacity);
    if (!buf)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < orig->capacity; i++)
    {
        buf[i] = orig->buffer[i];
        if (orig->buffer[i].state == CVX2_HT_ENTRY_FILLED)
        {
            buf[i].key = (orig->vtabk && orig->vtabk->clone) ? orig->vtabk->clone(orig->buffer[i].key)
                                                               : orig->buffer[i].key;
            buf[i].val = (orig->vtabv && orig->vtabv->clone) ? orig->vtabv->clone(orig->buffer[i].val)
                                                               : orig->buffer[i].val;
        }
    }

    clone->buffer = buf;
    clone->capacity = orig->capacity;
    clone->count = orig->count;
    return CVX_FLAG_OK;
}

static CVX_VAL *FUNC(__get_ref)(struct CVX_SNAME *self, CVX_KEY key)
{
    struct ENTRY *e = FUNC(__find_entry)(self, key);
    return e ? &e->val : NULL;
}

// Precondition: capacity has room and `key` is not already present.
static void FUNC(__insert_unchecked)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL val)
{
    CVX_KEY k = key;
    CVX_VAL v = val;
    size_t orig_pos = self->vtabk->hash(k) % self->capacity;
    size_t pos = orig_pos;

    while (true)
    {
        struct ENTRY *e = &self->buffer[pos % self->capacity];

        if (e->state != CVX2_HT_ENTRY_FILLED) // empty or tombstone: claim it
        {
            e->key = k;
            e->val = v;
            e->dist = pos - orig_pos;
            e->state = CVX2_HT_ENTRY_FILLED;
            return;
        }

        if (e->dist < pos - orig_pos) // robin hood: steal from rich
        {
            CVX_KEY tmp_k = e->key;
            CVX_VAL tmp_v = e->val;
            size_t tmp_dist = e->dist;

            e->key = k;
            e->val = v;
            e->dist = pos - orig_pos;

            k = tmp_k;
            v = tmp_v;
            orig_pos = pos - tmp_dist;
        }

        pos++;
    }
}

static enum cvx_flags FUNC(__resize)(struct CVX_SNAME *self, size_t new_cap)
{
    new_cap = FUNC(__next_prime)(new_cap);
    struct ENTRY *new_buf = malloc(sizeof(struct ENTRY) * new_cap);
    if (!new_buf)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < new_cap; i++)
        new_buf[i] = (struct ENTRY){ 0 };

    struct ENTRY *old_buf = self->buffer;
    size_t old_cap = self->capacity;
    self->buffer = new_buf;
    self->capacity = new_cap;

    for (size_t i = 0; i < old_cap; i++)
    {
        if (old_buf[i].state == CVX2_HT_ENTRY_FILLED)
            FUNC(__insert_unchecked)(self, old_buf[i].key, old_buf[i].val);
    }

    free(old_buf);
    return CVX_FLAG_OK;
}

// Precondition: self->count > 0 and the vtab has been checked.
static enum cvx_flags FUNC(__remove)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out)
{
    struct ENTRY *e = FUNC(__find_entry)(self, key);
    if (!e)
        return CVX_FLAG_NOT_FOUND;

    if (out)
        *out = e->val;
    else if (self->vtabv && self->vtabv->drop)
        self->vtabv->drop(e->val);

    if (self->vtabk && self->vtabk->drop)
        self->vtabk->drop(e->key);

    e->key = (CVX_KEY){ 0 };
    e->val = (CVX_VAL){ 0 };
    e->dist = 0;
    e->state = CVX2_HT_ENTRY_DELETED;
    return CVX_FLAG_OK;
}

#endif
// @cvx2:endvariant

// @cvx2:variant axis="collision" name="separate_chaining"
#ifdef CVX2_COLLISION_SEPARATE_CHAINING

static struct NODE *FUNC(__find_node)(struct CVX_SNAME *self, CVX_KEY key)
{
    if (self->capacity == 0)
        return NULL;

    size_t idx = self->vtabk->hash(key) % self->capacity;
    for (struct NODE *node = self->buckets[idx]; node; node = node->next)
    {
        if (self->vtabk->comp(node->key, key) == 0)
            return node;
    }

    return NULL;
}

static enum cvx_flags FUNC(__init_buffer)(struct CVX_SNAME *self, size_t capacity)
{
    size_t cap = FUNC(__next_prime)(capacity);
    struct NODE **buckets = malloc(sizeof(struct NODE *) * cap);
    if (!buckets)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < cap; i++)
        buckets[i] = NULL;

    self->buckets = buckets;
    self->capacity = cap;
    return CVX_FLAG_OK;
}

static void FUNC(__drop_buffer)(struct CVX_SNAME *self)
{
    if (!self->buckets)
        return;

    for (size_t i = 0; i < self->capacity; i++)
    {
        struct NODE *node = self->buckets[i];
        while (node)
        {
            struct NODE *next = node->next;
            if (self->vtabk && self->vtabk->drop)
                self->vtabk->drop(node->key);
            if (self->vtabv && self->vtabv->drop)
                self->vtabv->drop(node->val);
            free(node);
            node = next;
        }
    }

    free(self->buckets);
    self->buckets = NULL;
}

static enum cvx_flags FUNC(__clone_buffer)(struct CVX_SNAME *orig, struct CVX_SNAME *clone)
{
    if (!orig->buckets)
        return CVX_FLAG_OK;

    struct NODE **buckets = malloc(sizeof(struct NODE *) * orig->capacity);
    if (!buckets)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < orig->capacity; i++)
        buckets[i] = NULL;

    clone->buckets = buckets;
    clone->capacity = orig->capacity;

    for (size_t i = 0; i < orig->capacity; i++)
    {
        struct NODE **tail = &buckets[i];
        for (struct NODE *node = orig->buckets[i]; node; node = node->next)
        {
            struct NODE *copy = malloc(sizeof(struct NODE));
            if (!copy)
                return CVX_FLAG_ALLOC;

            copy->key = (orig->vtabk && orig->vtabk->clone) ? orig->vtabk->clone(node->key) : node->key;
            copy->val = (orig->vtabv && orig->vtabv->clone) ? orig->vtabv->clone(node->val) : node->val;
            copy->next = NULL;

            *tail = copy;
            tail = &copy->next;
        }
    }

    clone->count = orig->count;
    return CVX_FLAG_OK;
}

static CVX_VAL *FUNC(__get_ref)(struct CVX_SNAME *self, CVX_KEY key)
{
    struct NODE *node = FUNC(__find_node)(self, key);
    return node ? &node->val : NULL;
}

// Precondition: capacity has room and `key` is not already present.
static void FUNC(__insert_unchecked)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL val)
{
    size_t idx = self->vtabk->hash(key) % self->capacity;
    struct NODE *node = malloc(sizeof(struct NODE));
    if (!node) // pre-sized by the caller, so this is an unexpected hard OOM
        return;

    node->key = key;
    node->val = val;
    node->next = self->buckets[idx];
    self->buckets[idx] = node;
}

static enum cvx_flags FUNC(__resize)(struct CVX_SNAME *self, size_t new_cap)
{
    new_cap = FUNC(__next_prime)(new_cap);
    struct NODE **new_buckets = malloc(sizeof(struct NODE *) * new_cap);
    if (!new_buckets)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < new_cap; i++)
        new_buckets[i] = NULL;

    struct NODE **old_buckets = self->buckets;
    size_t old_cap = self->capacity;
    self->buckets = new_buckets;
    self->capacity = new_cap;

    for (size_t i = 0; i < old_cap; i++)
    {
        struct NODE *node = old_buckets[i];
        while (node)
        {
            struct NODE *next = node->next;
            size_t idx = self->vtabk->hash(node->key) % self->capacity;
            node->next = self->buckets[idx];
            self->buckets[idx] = node;
            node = next;
        }
    }

    free(old_buckets);
    return CVX_FLAG_OK;
}

// Precondition: self->count > 0 and the vtab has been checked.
static enum cvx_flags FUNC(__remove)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out)
{
    if (self->capacity == 0)
        return CVX_FLAG_NOT_FOUND;

    size_t idx = self->vtabk->hash(key) % self->capacity;
    struct NODE **link = &self->buckets[idx];

    while (*link)
    {
        struct NODE *node = *link;
        if (self->vtabk->comp(node->key, key) == 0)
        {
            if (out)
                *out = node->val;
            else if (self->vtabv && self->vtabv->drop)
                self->vtabv->drop(node->val);

            if (self->vtabk && self->vtabk->drop)
                self->vtabk->drop(node->key);

            *link = node->next;
            free(node);
            return CVX_FLAG_OK;
        }
        link = &node->next;
    }

    return CVX_FLAG_NOT_FOUND;
}

#endif
// @cvx2:endvariant

///
///
/// PUBLIC API (axis-independent)
///
///

enum cvx_flags FUNC(_init)(struct CVX_SNAME *self, struct VTAB_K *vtabk, struct VTAB_V *vtabv, size_t capacity)
{
    *self = (struct CVX_SNAME){ 0 };

    if (!vtabk || !vtabk->hash || !vtabk->comp)
        return CVX_FLAG_VTAB;

    self->load = 0.7;
    self->vtabk = vtabk;
    self->vtabv = vtabv;

    if (capacity == 0)
        return CVX_FLAG_OK;

    return FUNC(__init_buffer)(self, capacity);
}

enum cvx_flags FUNC(_clone)(struct CVX_SNAME *orig, struct CVX_SNAME *clone)
{
    enum cvx_flags flag = FUNC(_init)(clone, orig->vtabk, orig->vtabv, 0);
    if (flag != CVX_FLAG_OK)
        return flag;

    clone->load = orig->load;

    if (orig->count == 0)
        return CVX_FLAG_OK;

    return FUNC(__clone_buffer)(orig, clone);
}

enum cvx_flags FUNC(_drop)(struct CVX_SNAME *self)
{
    if (!self)
        return CVX_FLAG_OK;

    FUNC(__drop_buffer)(self);
    self->capacity = 0;
    self->count = 0;
    return CVX_FLAG_OK;
}

size_t FUNC(_count)(struct CVX_SNAME *self)
{
    return self->count;
}

size_t FUNC(_capacity)(struct CVX_SNAME *self)
{
    return self->capacity;
}

double FUNC(_load)(struct CVX_SNAME *self)
{
    return self->load;
}

bool FUNC(_empty)(struct CVX_SNAME *self)
{
    return self->count == 0;
}

enum cvx_flags FUNC(_insert)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL val)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return CVX_FLAG_VTAB;

    if (FUNC(__get_ref)(self, key) != NULL)
        return CVX_FLAG_DUPLICATE;

    if (self->capacity == 0 || (double)self->count >= (double)self->capacity * self->load)
    {
        size_t need = (self->capacity == 0) ? 53 : self->capacity + 1;
        enum cvx_flags rflag = FUNC(__resize)(self, need);
        if (rflag != CVX_FLAG_OK)
            return rflag;
    }

    FUNC(__insert_unchecked)(self, key, val);
    self->count++;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_update)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL new_val, CVX_VAL *old_out)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return CVX_FLAG_VTAB;

    CVX_VAL *ref = FUNC(__get_ref)(self, key);
    if (!ref)
        return CVX_FLAG_NOT_FOUND;

    if (old_out)
        *old_out = *ref;
    else if (self->vtabv && self->vtabv->drop)
        self->vtabv->drop(*ref);

    *ref = new_val;
    return CVX_FLAG_OK;
}

enum cvx_flags FUNC(_remove)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return CVX_FLAG_VTAB;

    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    enum cvx_flags flag = FUNC(__remove)(self, key, out);
    if (flag == CVX_FLAG_OK)
        self->count--;
    return flag;
}

enum cvx_flags FUNC(_get)(struct CVX_SNAME *self, CVX_KEY key, CVX_VAL *out)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return CVX_FLAG_VTAB;

    CVX_VAL *ref = FUNC(__get_ref)(self, key);
    if (!ref)
        return CVX_FLAG_NOT_FOUND;

    if (out)
        *out = *ref;
    return CVX_FLAG_OK;
}

CVX_VAL *FUNC(_get_ref)(struct CVX_SNAME *self, CVX_KEY key)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return NULL;

    return FUNC(__get_ref)(self, key);
}

bool FUNC(_contains)(struct CVX_SNAME *self, CVX_KEY key)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return false;

    return FUNC(__get_ref)(self, key) != NULL;
}
