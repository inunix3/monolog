/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct StrBuf {
    char *data;
    size_t len;
} StrBuf;

bool str_init(StrBuf *self);
bool str_init_n(StrBuf *self, size_t len);
bool str_dup(StrBuf *self, const char *cstr);
bool str_dup_n(StrBuf *self, const char *cstr, size_t len);
void str_deinit(StrBuf *self);
bool str_cat(StrBuf *self, const StrBuf *src);
bool str_set_cstr(StrBuf *self, const char *cstr);
bool str_equal(const StrBuf *self, const StrBuf *str);
bool str_resize(StrBuf *self, size_t n);
