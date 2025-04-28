#include <monolog/hashmap.h>

#include <stdlib.h>
#include <string.h>

#define TOMBSTONE ((void *)0x1)
#define MAX_LOAD 0.7

/*
 * Details about the hash function and its parameters can be found here:
 * http://www.isthe.com/chongo/tech/comp/fnv/
 */

#define FNV1A_OFFSET_BASIS 2166136261
#define FNV1A_PRIME 16777619

static HashType calc_hash(const char *key, size_t len) {
    HashType hash = FNV1A_OFFSET_BASIS;

    for (size_t i = 0; i < len; ++i) {
        hash ^= (uint8_t)key[i];
        hash *= FNV1A_PRIME;
    }

    return hash;
}

static Bucket *find_bucket(Bucket *buckets, size_t cap, const char *key) {
    Bucket *bucket = NULL;
    Bucket *tombstone = NULL;

    /* HashType idx = calc_hash(key, strlen(key)) % cap;
     *
     * OPTIMIZATION: since the capacity is always a power of 2, we can use
     * bitwise AND and capacity minus one to get the remainder of division.
     * Bitwise instructions are very cheap, especially when compared to heavy
     * operations like modulo.
     */
    HashType idx = calc_hash(key, strlen(key)) & (cap - 1);

    /* Linear probing */
    for (;;) {
        bucket = &buckets[idx];

        if (!bucket->key) {
            if (!bucket->value) {
                return tombstone ? tombstone : bucket;
            } else if (bucket->value == TOMBSTONE && !tombstone) {
                tombstone = bucket;
            }
        } else if (strcmp(bucket->key, key) == 0) {
            break;
        }

        idx = (idx + 1) & (cap - 1);
    }

    return bucket;
}

static bool should_resize(const HashMap *self) {
    return self->size >= self->cap * MAX_LOAD;
}

static bool grow(HashMap *self) {
    size_t new_cap = self->cap * 2;
    Bucket *new_buckets = calloc(new_cap, sizeof(Bucket));

    if (!new_buckets) {
        return false;
    }

    /* We're ignoring tombstones during copying buckets, so we need to
     * recalculate the number of buckets */
    self->size = 0;

    for (size_t i = 0; i < self->cap; ++i) {
        Bucket *bucket = &self->buckets[i];

        if (!bucket->key) {
            continue;
        }

        Bucket *dest_bucket = find_bucket(new_buckets, new_cap, bucket->key);
        dest_bucket->key = bucket->key;
        dest_bucket->value = bucket->value;

        ++self->size;
    }

    self->buckets = new_buckets;
    self->cap = new_cap;

    return true;
}

bool hashmap_init(HashMap *self) {
    self->buckets = calloc(HASHMAP_DEFAULT_CAP, sizeof(Bucket));

    if (!self->buckets) {
        return false;
    }

    self->size = 0;
    self->cap = HASHMAP_DEFAULT_CAP;

    return true;
}

void hashmap_deinit(HashMap *self) {
    free(self->buckets);
    self->buckets = NULL;

    self->size = 0;
    self->cap = 0;
}

bool hashmap_add(HashMap *self, const char *key, void *value) {
    if (should_resize(self) && !grow(self)) {
        return false;
    }

    Bucket *bucket = find_bucket(self->buckets, self->cap, key);
    bool new_bucket = !bucket->key && bucket->value != TOMBSTONE;

    bucket->key = key;
    bucket->value = value;

    if (new_bucket) {
        ++self->size;
    }

    return true;
}

void hashmap_remove(HashMap *self, const char *key) {
    if (self->size == 0) {
        return;
    }

    Bucket *bucket = find_bucket(self->buckets, self->cap, key);

    if (bucket->key) {
        bucket->key = NULL;
        bucket->value = TOMBSTONE;

        --self->size;
    }
}

void *hashmap_get(const HashMap *self, const char *key) {
    if (self->size == 0) {
        return NULL;
    }

    Bucket *bucket = find_bucket(self->buckets, self->cap, key);

    if (bucket->key && strcmp(bucket->key, key) == 0) {
        return bucket->value;
    }

    return NULL;
}

void hashmap_clear(HashMap *self) {
    memset(self->buckets, 0, self->cap * sizeof(Bucket));
    self->size = 0;
}

HashMapIter hashmap_iter(HashMap *self) {
    HashMapIter it = {self};

    it.idx = 0;
    hashmap_iter_next(&it);

    return it;
}

Bucket *hashmap_iter_next(HashMapIter *self) {
    while (self->idx < self->map->cap) {
        self->bucket = &self->map->buckets[self->idx++];

        if (self->bucket->key) {
            return self->bucket;
        }
    }

    self->bucket = NULL;

    return NULL;
}
