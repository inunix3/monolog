/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE(_a) (sizeof(_a) / (sizeof((_a)[0])))
#define UNUSED(_x) (void)(_x)

long file_size(FILE *file);
char *read_file_stream(FILE *file);
char *read_file(const char *filename);

char *cstr_dup_n(const char *str, size_t len);
char *cstr_dup(const char *str);

bool str_to_i64(const char *str, int64_t *out);

/*
 * Allocate a memory block filled with zeros of the specified size in bytes.
 * In case of failure terminate the program.
 */
void *mem_alloc(size_t size);
void *mem_realloc(void *block, size_t size);
