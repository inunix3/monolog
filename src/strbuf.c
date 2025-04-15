#include <monolog/strbuf.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

bool str_init(StrBuf *self, const char *cstr) {
    return str_initn(self, cstr, strlen(cstr));
}

bool str_initn(StrBuf *self, const char *cstr, size_t len) {
    self->data = malloc(len + 1);

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
