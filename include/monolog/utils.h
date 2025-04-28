#pragma once

#include <stdio.h>

long file_size(FILE *file);
char *read_file_stream(FILE *file);
char *read_file(const char *filename);

void *cstr_dup_n(const char *str, size_t len);
void *cstr_dup(const char *str);
