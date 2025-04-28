#pragma once

/*
 * Simple implementation of a hash table:
 *   - does not copy anything - keys and values are pointers
 *   - open addressing for bucket storage
 *   - linear probing for finding an available bucket
 *   - FNV-1a as a hash function
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* OPTIMIZATION: it's better to use powers of 2 */
#define HASHMAP_DEFAULT_CAP 256

typedef uint32_t HashType;

typedef struct Bucket {
    const char *key;
    void *value;
} Bucket;

typedef struct HashMap {
    Bucket *buckets;
    size_t size;
    size_t cap;
} HashMap;

bool hashmap_init(HashMap *self);
void hashmap_deinit(HashMap *self);

bool hashmap_add(HashMap *self, const char *key, void *value);
void hashmap_remove(HashMap *self, const char *key);
void *hashmap_get(const HashMap *self, const char *key);
void hashmap_clear(HashMap *self);

typedef struct HashMapIter {
    HashMap *map;
    Bucket *bucket;
    size_t idx;
} HashMapIter;

HashMapIter hashmap_iter(HashMap *self);
Bucket *hashmap_iter_next(HashMapIter *self);
