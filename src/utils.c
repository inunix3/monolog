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

    char *buf = malloc((size_t)size + 1);

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

void *cstr_dup_n(const char *str, size_t len) {
    char *new_str = malloc(len + 1);

    if (!new_str) {
        return NULL;
    }

    memcpy(new_str, str, len);
    new_str[len] = '\0';

    return new_str;
}

void *cstr_dup(const char *str) { return cstr_dup_n(str, strlen(str)); }

bool is_ident_builtin(const char *name) {
    static const char *builtins[] = {
        "print",
        "println",
        "exit",
        "push",
        "pop"
    };

    for (size_t i = 0; i < ARRAY_SIZE(builtins); ++i) {
        if (strcmp(name, builtins[i]) == 0) {
            return true;
        }
    }

    return false;
}
