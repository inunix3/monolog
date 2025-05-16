/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/utils.h>
#include <monolog/vector.h>

#include <stdlib.h>
#include <string.h>

bool vec_init(Vector *self, size_t element_size) {
    self->data = mem_alloc(element_size * VECTOR_DEFAULT_CAP);

    if (!self->data) {
        return false;
    }

    self->cap = VECTOR_DEFAULT_CAP;
    self->len = 0;
    self->element_size = element_size;

    return true;
}

void vec_deinit(Vector *self) {
    if (!self) {
        return;
    }

    free(self->data);
    self->data = NULL;
    self->len = 0;
    self->cap = 0;
    self->element_size = 0;
}

static bool grow(Vector *self) {
    size_t new_cap = self->cap * 2;

    self->data = mem_realloc(self->data, new_cap * self->element_size);

    if (!self->data) {
        return false;
    }

    memset(
        self->data + self->len * self->element_size, 0,
        (new_cap - self->cap) * self->element_size
    );

    self->cap = new_cap;

    return true;
}

void *vec_push(Vector *self, const void *data) {
    if (self->len >= self->cap && !grow(self)) {
        return NULL;
    }

    void *block = self->data + self->len++ * self->element_size;
    memcpy(block, data, self->element_size);

    return block;
}

void *vec_emplace(Vector *self) {
    if (self->len >= self->cap && !grow(self)) {
        return NULL;
    }

    return self->data + self->len++ * self->element_size;
}

void vec_pop(Vector *self) {
    if (self->len == 0) {
        return;
    }

    memset(
        self->data + (self->len - 1) * self->element_size, 0, self->element_size
    );

    --self->len;
}

void vec_clear(Vector *self) {
    memset(self->data, 0, self->cap * self->element_size);
    self->len = 0;
}
