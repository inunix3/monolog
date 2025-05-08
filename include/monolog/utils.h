#pragma once

#include <stdbool.h>
#include <stdio.h>

#define ARRAY_SIZE(_a) (sizeof(_a) / (sizeof((_a)[0])))

long file_size(FILE *file);
char *read_file_stream(FILE *file);
char *read_file(const char *filename);

void *cstr_dup_n(const char *str, size_t len);
void *cstr_dup(const char *str);

bool is_ident_builtin(const char *name);
