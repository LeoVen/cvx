#include "hashmap.h"
#include <stdlib.h>

// Each collision variant implements this set of helpers; the public API
// below is shared between both.
static enum cvx_flags hm__init_buffer(struct HashMap *self, size_t capacity);
static void hm__drop_buffer(struct HashMap *self);
static enum cvx_flags hm__clone_buffer(struct HashMap *orig, struct HashMap *clone);
static bool hm__key_exists(struct HashMap *self, void *key);
static void **hm__get_ref(struct HashMap *self, void *key);
static void hm__insert_unchecked(struct HashMap *self, void *key, void *val);
static enum cvx_flags hm__remove(struct HashMap *self, void *key, void **out);
static enum cvx_flags hm__resize(struct HashMap *self, size_t new_cap);

// Capacity is always rounded up to one of these primes.
// clang-format off
static const size_t hm__primes[] = {
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
static const size_t hm__primes_count = sizeof(hm__primes) / sizeof(hm__primes[0]);

enum cvx_flags hm_init(struct HashMap *self, struct HashMapVtabK *vtabk, struct HashMapVtabV *vtabv,
                       size_t capacity)
{
    *self = (struct HashMap){ 0 };

    if (!vtabk || !vtabk->hash || !vtabk->comp)
        return CVX_FLAG_VTAB;

    self->load = 0.7;
    self->vtabk = vtabk;
    self->vtabv = vtabv;

    if (capacity == 0)
        return CVX_FLAG_OK;

    return hm__init_buffer(self, capacity);
}

enum cvx_flags hm_clone(struct HashMap *orig, struct HashMap *clone)
{
    enum cvx_flags flag = hm_init(clone, orig->vtabk, orig->vtabv, 0);
    if (flag != CVX_FLAG_OK)
        return flag;

    clone->load = orig->load;

    if (orig->count == 0)
        return CVX_FLAG_OK;

    return hm__clone_buffer(orig, clone);
}

enum cvx_flags hm_drop(struct HashMap *self)
{
    if (!self)
        return CVX_FLAG_OK;

    hm__drop_buffer(self);
    self->capacity = 0;
    self->count = 0;
    return CVX_FLAG_OK;
}

size_t hm_count(struct HashMap *self)
{
    return self->count;
}

size_t hm_capacity(struct HashMap *self)
{
    return self->capacity;
}

double hm_load(struct HashMap *self)
{
    return self->load;
}

bool hm_empty(struct HashMap *self)
{
    return self->count == 0;
}

enum cvx_flags hm_insert(struct HashMap *self, void *key, void *val)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return CVX_FLAG_VTAB;

    if (hm__key_exists(self, key))
        return CVX_FLAG_DUPLICATE;

    if (self->capacity == 0 || (double)self->count >= (double)self->capacity * self->load)
    {
        size_t need = (self->capacity == 0) ? 53 : self->capacity + 1;
        enum cvx_flags rflag = hm__resize(self, need);
        if (rflag != CVX_FLAG_OK)
            return rflag;
    }

    hm__insert_unchecked(self, key, val);
    self->count++;
    return CVX_FLAG_OK;
}

enum cvx_flags hm_update(struct HashMap *self, void *key, void *new_val, void **old_out)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return CVX_FLAG_VTAB;

    void **ref = hm__get_ref(self, key);
    if (!ref)
        return CVX_FLAG_NOT_FOUND;

    if (old_out)
        *old_out = *ref;
    else if (self->vtabv && self->vtabv->drop)
        self->vtabv->drop(*ref);

    *ref = new_val;
    return CVX_FLAG_OK;
}

enum cvx_flags hm_remove(struct HashMap *self, void *key, void **out)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return CVX_FLAG_VTAB;

    if (self->count == 0)
        return CVX_FLAG_EMPTY;

    enum cvx_flags flag = hm__remove(self, key, out);
    if (flag == CVX_FLAG_OK)
        self->count--;
    return flag;
}

enum cvx_flags hm_get(struct HashMap *self, void *key, void **out)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return CVX_FLAG_VTAB;

    void **ref = hm__get_ref(self, key);
    if (!ref)
        return CVX_FLAG_NOT_FOUND;

    if (out)
        *out = *ref;
    return CVX_FLAG_OK;
}

void **hm_get_ref(struct HashMap *self, void *key)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return NULL;

    return hm__get_ref(self, key);
}

bool hm_contains(struct HashMap *self, void *key)
{
    if (!self->vtabk || !self->vtabk->hash || !self->vtabk->comp)
        return false;

    return hm__key_exists(self, key);
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
static size_t hm__next_prime(size_t required)
{
    for (size_t i = 0; i < hm__primes_count; i++)
    {
        if (hm__primes[i] >= required)
            return hm__primes[i];
    }
    return required;
}

// collision strategy: separate chaining

static struct HashMapNode *hm__find_node(struct HashMap *self, void *key)
{
    if (self->capacity == 0)
        return NULL;

    size_t idx = self->vtabk->hash(key) % self->capacity;
    for (struct HashMapNode *node = self->buckets[idx]; node; node = node->next)
    {
        if (self->vtabk->comp(node->key, key) == 0)
            return node;
    }

    return NULL;
}

static bool hm__key_exists(struct HashMap *self, void *key)
{
    return hm__find_node(self, key) != NULL;
}

static enum cvx_flags hm__init_buffer(struct HashMap *self, size_t capacity)
{
    size_t cap = hm__next_prime(capacity);
    struct HashMapNode **buckets = malloc(sizeof(struct HashMapNode *) * cap);
    if (!buckets)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < cap; i++)
        buckets[i] = NULL;

    self->buckets = buckets;
    self->capacity = cap;
    return CVX_FLAG_OK;
}

static void hm__drop_buffer(struct HashMap *self)
{
    if (!self->buckets)
        return;

    for (size_t i = 0; i < self->capacity; i++)
    {
        struct HashMapNode *node = self->buckets[i];
        while (node)
        {
            struct HashMapNode *next = node->next;
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

static enum cvx_flags hm__clone_buffer(struct HashMap *orig, struct HashMap *clone)
{
    if (!orig->buckets)
        return CVX_FLAG_OK;

    struct HashMapNode **buckets = malloc(sizeof(struct HashMapNode *) * orig->capacity);
    if (!buckets)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < orig->capacity; i++)
        buckets[i] = NULL;

    clone->buckets = buckets;
    clone->capacity = orig->capacity;

    for (size_t i = 0; i < orig->capacity; i++)
    {
        struct HashMapNode **tail = &buckets[i];
        for (struct HashMapNode *node = orig->buckets[i]; node; node = node->next)
        {
            struct HashMapNode *node_copy = malloc(sizeof(struct HashMapNode));
            if (!node_copy)
                return CVX_FLAG_ALLOC;

            node_copy->key = node->key;
            node_copy->val = node->val;
            node_copy->next = NULL;

            if (orig->vtabk && orig->vtabk->copy)
            {
                enum cvx_flags flag = orig->vtabk->copy(node->key, &node_copy->key);
                if (flag != CVX_FLAG_OK)
                {
                    free(node_copy);
                    return flag;
                }
            }

            if (orig->vtabv && orig->vtabv->copy)
            {
                enum cvx_flags flag = orig->vtabv->copy(node->val, &node_copy->val);
                if (flag != CVX_FLAG_OK)
                {
                    free(node_copy);
                    return flag;
                }
            }

            *tail = node_copy;
            tail = &node_copy->next;
        }
    }

    clone->count = orig->count;
    return CVX_FLAG_OK;
}

static void **hm__get_ref(struct HashMap *self, void *key)
{
    struct HashMapNode *node = hm__find_node(self, key);
    return node ? &node->val : NULL;
}

// Precondition: capacity has room and `key` is not already present.
static void hm__insert_unchecked(struct HashMap *self, void *key, void *val)
{
    size_t idx = self->vtabk->hash(key) % self->capacity;
    struct HashMapNode *node = malloc(sizeof(struct HashMapNode));
    if (!node) // pre-sized by the caller, so this is an unexpected hard OOM
        return;

    node->key = key;
    node->val = val;
    node->next = self->buckets[idx];
    self->buckets[idx] = node;
}

static enum cvx_flags hm__resize(struct HashMap *self, size_t new_cap)
{
    new_cap = hm__next_prime(new_cap);
    struct HashMapNode **new_buckets = malloc(sizeof(struct HashMapNode *) * new_cap);
    if (!new_buckets)
        return CVX_FLAG_ALLOC;

    for (size_t i = 0; i < new_cap; i++)
        new_buckets[i] = NULL;

    struct HashMapNode **old_buckets = self->buckets;
    size_t old_cap = self->capacity;
    self->buckets = new_buckets;
    self->capacity = new_cap;

    for (size_t i = 0; i < old_cap; i++)
    {
        struct HashMapNode *node = old_buckets[i];
        while (node)
        {
            struct HashMapNode *next = node->next;
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
static enum cvx_flags hm__remove(struct HashMap *self, void *key, void **out)
{
    if (self->capacity == 0)
        return CVX_FLAG_NOT_FOUND;

    size_t idx = self->vtabk->hash(key) % self->capacity;
    struct HashMapNode **link = &self->buckets[idx];

    while (*link)
    {
        struct HashMapNode *node = *link;
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
