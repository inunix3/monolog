#include <monolog/vector.h>

#include <stdlib.h>
#include <string.h>

bool vec_init(Vector *self, size_t element_size) {
    self->data = malloc(VECTOR_DEFAULT_CAP * element_size);
    self->cap = VECTOR_DEFAULT_CAP;
    self->len = 0;
    self->element_size = element_size;

    return true;
}

void vec_deinit(Vector *self) {
    if (!self) {
        return;
    }

    free(self->data);

    memset(self, 0, sizeof(*self));
}

void vec_push(Vector *self, void *data) {
    if (self->len >= self->cap) {
        self->data = realloc(self->data, self->cap * self->element_size + VECTOR_DEFAULT_CAP * self->element_size);
        self->cap += VECTOR_DEFAULT_CAP;
    }

    memcpy(self->data + self->len++ * self->element_size, data, self->element_size);
}
