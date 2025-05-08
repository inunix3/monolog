/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#define VECTOR_DEFAULT_CAP 64

typedef struct Vector {
    void *data;
    size_t cap;
    size_t len;
    size_t element_size;
} Vector;

bool vec_init(Vector *self, size_t element_size);
void vec_deinit(Vector *self);
void *vec_push(Vector *self, const void *data);
void *vec_emplace(Vector *self);
void vec_pop(Vector *self);
bool vec_reserve(Vector *self, size_t new_cap);
void vec_clear(Vector *self);

/* NOTE: using on empty vector is an undefined behavior */
#define VEC_LAST(_self, _type) (((_type *)(_self)->data)[(_self)->len - 1])
