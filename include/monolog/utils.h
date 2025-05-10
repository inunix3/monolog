#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE(_a) (sizeof(_a) / (sizeof((_a)[0])))
#define UNREACHABLE()                                                          \
    fprintf(                                                                   \
        stderr,                                                                \
        "reached unreachable code (must be a bug in interpreter's code):\n"    \
        "  %s:%d\n",                                                           \
        __FILE__, __LINE__                                                     \
    );                                                                         \
    abort()

long file_size(FILE *file);
char *read_file_stream(FILE *file);
char *read_file(const char *filename);

char *cstr_dup_n(const char *str, size_t len);
char *cstr_dup(const char *str);

bool is_ident_builtin(const char *name);

/*
 * Allocate a memory block filled with zeros of the specified size in bytes.
 * In case of failure, terminate the program.
 *
 * I'd call it just xmalloc, but unfortunately, this function can be defined by
 * some systems/compilers.
 */
void *mem_alloc(size_t size);
void *mem_realloc(void *block, size_t size);
