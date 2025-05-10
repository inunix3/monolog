#include <monolog/utils.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

long file_size(FILE *file) {
    if (fseek(file, 0, SEEK_END) != 0) {
        return -1;
    }

    long pos = ftell(file);

    if (pos < 0) {
        return -1;
    }

    rewind(file);

    return pos;
}

char *read_file_stream(FILE *file) {
    long size = file_size(file);

    if (size < 0) {
        fprintf(stderr, "cannot get file size: %s\n", strerror(errno));
        fclose(file);

        return NULL;
    }

    char *buf = mem_alloc((size_t)size + 1);

    if (!buf) {
        fprintf(stderr, "cannot allocate buffer: %s\n", strerror(errno));
        fclose(file);

        return NULL;
    }

    fread(buf, sizeof(*buf), (size_t)size, file);
    buf[size] = 0;

    return buf;
}

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");

    if (!file) {
        fprintf(stderr, "cannot open '%s': %s\n", filename, strerror(errno));

        return NULL;
    }

    char *buf = read_file_stream(file);

    fclose(file);

    return buf;
}

char *cstr_dup_n(const char *str, size_t len) {
    char *new_str = mem_alloc(len + 1);
    memcpy(new_str, str, len);

    return new_str;
}

char *cstr_dup(const char *str) { return cstr_dup_n(str, strlen(str)); }

void *mem_alloc(size_t size) {
    void *block = malloc(size);

    if (!block) {
        fprintf(
            stderr,
            "fatal error: cannot allocate %zu bytes: out of memory. "
            "Terminating due to lack of memory\n",
            size
        );

        exit(EXIT_FAILURE);
    }

    memset(block, 0, size);

    return block;
}

void *mem_realloc(void *block, size_t size) {
    block = realloc(block, size);

    if (!block) {
        fprintf(
            stderr,
            "fatal error: cannot allocate %zu bytes: out of memory. "
            "Terminating due to lack of memory\n",
            size
        );

        exit(EXIT_FAILURE);
    }

    return block;
}
