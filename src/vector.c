/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/vector.h>

#include <stdlib.h>
#include <string.h>

bool vec_init(Vector *self, size_t element_size) {
    self->data = calloc(element_size, VECTOR_DEFAULT_CAP);

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
    memset(self, 0, sizeof(*self));
}

bool vec_push(Vector *self, const void *data) {
    if (self->len >= self->cap) {
        size_t new_cap = self->cap * 2;

        self->data = realloc(self->data, new_cap * self->element_size);

        if (!self->data) {
            return false;
        }

        memset(
            self->data + self->len * self->element_size, 0,
            (new_cap - self->cap) * self->element_size
        );

        self->cap = new_cap;
    }

    memcpy(
        self->data + self->len++ * self->element_size, data, self->element_size
    );

    return true;
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

bool vec_reserve(Vector *self, size_t new_cap) {
    if (new_cap <= self->cap) {
        return false;
    }

    self->data = realloc(self->data, new_cap * self->element_size);

    self->cap = new_cap;

    return true;
}

void vec_clear(Vector *self) {
    memset(self->data, 0, self->cap * self->element_size);
    self->len = 0;
}
