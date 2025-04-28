#include <monolog/strbuf.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

bool str_init(StrBuf *self, const char *cstr) {
    return str_init_n(self, cstr, strlen(cstr));
}

bool str_init_n(StrBuf *self, const char *cstr, size_t len) {
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

bool str_set(StrBuf *self, const char *cstr) {
    if (!self->data) {
        return str_init(self, cstr);
    }

    size_t cstr_len = strlen(cstr);

    if (cstr_len > self->len) {
        self->data = realloc(self->data, self->len + cstr_len);

        if (self->data) {
            return false;
        }

        self->len += cstr_len;
    } else {
        self->len = cstr_len;
    }

    memcpy(self->data, cstr, cstr_len);

    return true;
}
