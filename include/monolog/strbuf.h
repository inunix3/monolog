#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct StrBuf {
    char *data;
    size_t len;
} StrBuf;

bool str_init(StrBuf *self, const char *cstr);
bool str_init_n(StrBuf *self, const char *cstr, size_t len);
void str_deinit(StrBuf *self);
bool str_set(StrBuf *self, const char *cstr);
