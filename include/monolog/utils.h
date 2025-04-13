#pragma once

#include <stdio.h>

long file_size(FILE *file);
char *read_file_stream(FILE *file);
char *read_file(const char *filename);
