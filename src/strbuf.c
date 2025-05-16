/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/strbuf.h>
#include <monolog/utils.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

bool str_init(StrBuf *self) { return str_init_n(self, 0); }

bool str_init_n(StrBuf *self, size_t len) {
    self->data = mem_alloc(len + 1);

    if (!self->data) {
        return false;
    }

    memset(self->data, 0, len + 1);

    self->len = len;

    return true;
}

bool str_dup(StrBuf *self, const char *cstr) {
    return str_dup_n(self, cstr, strlen(cstr));
}

bool str_dup_n(StrBuf *self, const char *cstr, size_t len) {
    self->data = mem_alloc(len + 1);

    if (!self->data) {
        return false;
    }

    memcpy(self->data, cstr, len);
    self->data[len] = 0;

    self->len = len;

    return self;
}

void str_deinit(StrBuf *self) {
    free(self->data);
    self->data = NULL;
    self->len = 0;
}

bool str_cat(StrBuf *self, const StrBuf *src) {
    size_t new_len = self->len + src->len;
    self->data = mem_realloc(self->data, new_len + 1);

    if (!self->data) {
        return false;
    }

    memcpy(self->data + self->len, src->data, src->len);
    self->data[new_len] = '\0';

    self->len = new_len;

    return true;
}

bool str_set_cstr(StrBuf *self, const char *cstr) {
    if (!self->data) {
        return str_dup(self, cstr);
    }

    size_t cstr_len = strlen(cstr);

    if (cstr_len > self->len) {
        size_t diff = cstr_len - self->len;

        self->data = mem_realloc(self->data, self->len + diff + 1);

        if (!self->data) {
            return false;
        }

        self->len += diff;
    } else {
        self->len = cstr_len;
    }

    memcpy(self->data, cstr, self->len + 1);

    return true;
}

bool str_equal(const StrBuf *self, const StrBuf *str) {
    if (self->len != str->len) {
        return false;
    }

    return strncmp(self->data, str->data, self->len) == 0;
}

bool str_resize(StrBuf *self, size_t n) {
    /* this function does not support shrink for now, because it was not needed.
     */
    if (n <= self->len) {
        return true;
    }

    self->data = mem_realloc(self->data, n + 1);

    if (!self->data) {
        return false;
    }

    memset(self->data + self->len, 0, n - self->len + 1);
    self->len = n;

    return true;
}
