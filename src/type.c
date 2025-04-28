#include <monolog/type.h>
#include <monolog/utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

static char g_buf[BUF_SIZE];

void format_type_name(const Type *type, size_t *pos) {
    switch (type->id) {
    case TYPE_INT:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - *pos, "int");

        break;
    case TYPE_STRING:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - *pos, "string");

        break;
    case TYPE_VOID:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - *pos, "void");

        break;
    case TYPE_OPTION:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - *pos, "option<");
        format_type_name(type->opt_type.type, pos);
        *pos += snprintf(g_buf + *pos, BUF_SIZE - *pos, ">");

        break;
    case TYPE_ARRAY:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - *pos, "array<");
        format_type_name(type->array_type.type, pos);
        *pos += snprintf(g_buf + *pos, BUF_SIZE - *pos, ">");

        break;
    case TYPE_ERROR:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - *pos, "<error>");

        break;
    case TYPE_NIL:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - *pos, "nil");

        break;
    }
}

const char *type_name(const Type *type) {
    size_t pos = 0;

    memset(g_buf, 0, sizeof(g_buf));
    format_type_name(type, &pos);

    return g_buf;
}

bool type_can_implicitly_convert(const Type *self, const Type *type) {
    if (type_equal(self, type)) {
        return true;
    }

    if (self->id == TYPE_NIL) {
        return type->id == TYPE_OPTION;
    }

    if (type->id == TYPE_OPTION) {
        return type_equal(self, type->opt_type.type);
    }

    return false;
}

bool type_system_init(TypeSystem *self) {
    if (!hashmap_init(&self->types)) {
        return false;
    }

    if (!vec_init(&self->type_names, sizeof(char *))) {
        return false;
    }

    return true;
}

void type_system_deinit(TypeSystem *self) {
    char **names = self->type_names.data;

    for (size_t i = 0; i < self->type_names.len; ++i) {
        free(names[i]);
    }

    vec_deinit(&self->type_names);

    for (HashMapIter it = hashmap_iter(&self->types); it.bucket != NULL;
         hashmap_iter_next(&it)) {
        Type *type = it.bucket->value;

        free(type->name);
        type->name = NULL;

        free(type);
    }

    hashmap_deinit(&self->types);
}

char *type_system_name(TypeSystem *self, const Type *type) {
    size_t pos = 0;

    format_type_name(type, &pos);

    char *name = cstr_dup(g_buf);

    if (!hashmap_get(&self->types, name)) {
        vec_push(&self->type_names, name);
    }

    return name;
}

Type *type_system_register(TypeSystem *self, const Type *type) {
    size_t pos = 0;

    format_type_name(type, &pos);

    Type *existing_type = hashmap_get(&self->types, g_buf);

    if (existing_type) {
        return existing_type;
    } else {
        Type *new_type = malloc(sizeof(*new_type));
        char *name = cstr_dup(g_buf);

        memcpy(new_type, type, sizeof(*new_type));
        new_type->name = name;

        hashmap_add(&self->types, name, new_type);

        return new_type;
    }
}
