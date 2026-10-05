#include "hashset.h"
#include <stdlib.h>

// Each collision variant implements this set of helpers; the public API
// below is shared between both.
static enum cvx_flags hs__init_buffer(struct HashSet *self, size_t capacity);
static void hs__drop_buffer(struct HashSet *self);
static enum cvx_flags hs__clone_buffer(struct HashSet *orig, struct HashSet *clone);
static bool hs__key_exists(struct HashSet *self, void *key);
static void hs__insert_unchecked(struct HashSet *self, void *key);
static enum cvx_flags hs__remove(struct HashSet *self, void *key);
static enum cvx_flags hs__resize(struct HashSet *self, size_t new_cap);

// Capacity is always rounded up to one of these primes.
// clang-format off
static const size_t hs__primes[] = {
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
static const size_t hs__primes_count = sizeof(hs__primes) / sizeof(hs__primes[0]);

enum cvx_flags hs_init(struct HashSet *self, struct HashSetVtabK *vtabk, size_t capacity)
{
    *self = (struct HashSet){ 0 };

    if (!vtabk || !vtabk->hash || !vtabk->comp)
        return CVX_FLAG_VTAB;

    self->load = 0.7;
    self->vtabk = vtabk;

    if (capacity == 0)
        return CVX_FLAG_OK;

    return hs__init_buffer(self, capacity);
}

enum cvx_flags hs_clone(struct HashSet *orig, struct HashSet *clone)
{
    enum cvx_flags flag = hs_init(clone, orig->vtabk, 0);
    if (flag != CVX_FLAG_OK)
        return flag;

    clone->load = orig->load;

    if (orig->count == 0)
        return CVX_FLAG_OK;

    return hs__clone_buffer(orig, clone);
}

enum cvx_flags hs_drop(struct HashSet *self)
{
    if (!self)
        return CVX_FLAG_OK;

    hs__drop_buffer(self);
    self->capacity = 0;
    self->count = 0;
    return CVX_FLAG_OK;
}

size_t hs_count(struct HashSet *self)
{
    return self->count;
}

size_t hs_capacity(struct HashSet *self)
{
    return self->capacity;
}

double hs_load(struct HashSet *self)
{
    return self->load;
}

bool hs_empty(struct HashSet *self)
{
    return self->count == 0;
}

enum cvx_flags hs_insert(struct HashSet *self, void *key)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return CVX_FLAG_VTAB;

    if (hs__key_exists(self, key))
        return CVX_FLAG_DUPLICATE;

    if (self->capacity == 0 || (double)self->count >= (double)self->capacity * self->load)
    {
        size_t need = (self->capacity == 0) ? 53 : self->capacity + 1;
        enum cvx_flags rflag = hs__resize(self, need);
        if (rflag != CVX_FLAG_OK)
            return rflag;
    }

    hs__insert_unchecked(self, key);
    self->count++;
    return CVX_FLAG_OK;
}

enum cvx_flags hs_remove(struct HashSet *self, void *key)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return CVX_FLAG_VTAB;

    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    enum cvx_flags flag = hs__remove(self, key);
    if (flag == CVX_FLAG_OK)
        self->count--;
    return flag;
}

bool hs_contains(struct HashSet *self, void *key)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return false;

    return hs__key_exists(self, key);
}

//
//
//
// BEGIN PRIVATE FUNCTIONS
//
//
//

// Returns the smallest prime in the table that is >= required.
// Falls back to required if it exceeds all primes.
static size_t hs__next_prime(size_t required)
{
    for (size_t i = 0; i < hs__primes_count; i++)
    {
        if (hs__primes[i] >= required)
            return hs__primes[i];
    }
    return required;
}

// collision strategy: open addressing

// state field values.
enum
{
    CVX2_HT_ENTRY_EMPTY = 0,
    CVX2_HT_ENTRY_FILLED = 1,
    CVX2_HT_ENTRY_DELETED = -1,
};

static struct HashSetEntry *hs__find_entry(struct HashSet *self, void *key)
{
    if (self->capacity == 0)
        return NULL;

    size_t pos = self->vtabk->hash(key) % self->capacity;
    struct HashSetEntry *e = &self->buffer[pos];

    while (e->state != CVX2_HT_ENTRY_EMPTY)
    {
        if (e->state == CVX2_HT_ENTRY_FILLED && self->vtabk->comp(e->key, key) == 0)
            return e;
        pos++;
        e = &self->buffer[pos % self->capacity];
    }

    return NULL;
}

static bool hs__key_exists(struct HashSet *self, void *key)
{
    return hs__find_entry(self, key) != NULL;
}

static enum cvx_flags hs__init_buffer(struct HashSet *self, size_t capacity)
{
    size_t cap = hs__next_prime(capacity);
    struct HashSetEntry *buf = malloc(sizeof(struct HashSetEntry) * cap);
    if (!buf)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < cap; i++)
        buf[i] = (struct HashSetEntry){ 0 };

    self->buffer = buf;
    self->capacity = cap;
    return CVX_FLAG_OK;
}

static void hs__drop_buffer(struct HashSet *self)
{
    if (!self->buffer)
        return;

    for (size_t i = 0; i < self->capacity; i++)
    {
        if (self->buffer[i].state == CVX2_HT_ENTRY_FILLED)
        {
            if (self->vtabk && self->vtabk->drop)
                self->vtabk->drop(self->buffer[i].key);
        }
    }

    free(self->buffer);
    self->buffer = NULL;
}

static enum cvx_flags hs__clone_buffer(struct HashSet *orig, struct HashSet *clone)
{
    if (!orig->buffer)
        return CVX_FLAG_OK;

    struct HashSetEntry *buf = malloc(sizeof(struct HashSetEntry) * orig->capacity);
    if (!buf)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < orig->capacity; i++)
    {
        buf[i] = orig->buffer[i];
        if (orig->buffer[i].state != CVX2_HT_ENTRY_FILLED)
            continue;

        if (orig->vtabk && orig->vtabk->copy)
        {
            enum cvx_flags flag = orig->vtabk->copy(orig->buffer[i].key, &buf[i].key);
            if (flag != CVX_FLAG_OK)
            {
                free(buf);
                return flag;
            }
        }
    }

    clone->buffer = buf;
    clone->capacity = orig->capacity;
    clone->count = orig->count;
    return CVX_FLAG_OK;
}

// Precondition: capacity has room and `key` is not already present.
static void hs__insert_unchecked(struct HashSet *self, void *key)
{
    void *k = key;
    size_t orig_pos = self->vtabk->hash(k) % self->capacity;
    size_t pos = orig_pos;

    while (true)
    {
        struct HashSetEntry *e = &self->buffer[pos % self->capacity];

        if (e->state != CVX2_HT_ENTRY_FILLED) // empty or tombstone: claim it
        {
            e->key = k;
            e->dist = pos - orig_pos;
            e->state = CVX2_HT_ENTRY_FILLED;
            return;
        }

        if (e->dist < pos - orig_pos) // robin hood: steal from rich
        {
            void *tmp_k = e->key;
            size_t tmp_dist = e->dist;

            e->key = k;
            e->dist = pos - orig_pos;

            k = tmp_k;
            orig_pos = pos - tmp_dist;
        }

        pos++;
    }
}

static enum cvx_flags hs__resize(struct HashSet *self, size_t new_cap)
{
    new_cap = hs__next_prime(new_cap);
    struct HashSetEntry *new_buf = malloc(sizeof(struct HashSetEntry) * new_cap);
    if (!new_buf)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < new_cap; i++)
        new_buf[i] = (struct HashSetEntry){ 0 };

    struct HashSetEntry *old_buf = self->buffer;
    size_t old_cap = self->capacity;
    self->buffer = new_buf;
    self->capacity = new_cap;

    for (size_t i = 0; i < old_cap; i++)
    {
        if (old_buf[i].state == CVX2_HT_ENTRY_FILLED)
            hs__insert_unchecked(self, old_buf[i].key);
    }

    free(old_buf);
    return CVX_FLAG_OK;
}

// Precondition: self->count > 0 and the vtab has been checked.
static enum cvx_flags hs__remove(struct HashSet *self, void *key)
{
    struct HashSetEntry *e = hs__find_entry(self, key);
    if (!e)
        return CVX_FLAG_NOT_FOUND;

    if (self->vtabk && self->vtabk->drop)
        self->vtabk->drop(e->key);

    e->key = (void *){ 0 };
    e->dist = 0;
    e->state = CVX2_HT_ENTRY_DELETED;
    return CVX_FLAG_OK;
}
