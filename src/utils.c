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

    char *buf = malloc(size + 1);

    if (!buf) {
        fprintf(stderr, "cannot allocate buffer: %s\n", strerror(errno));
        fclose(file);

        return NULL;
    }

    fread(buf, sizeof(*buf), size, file);
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
