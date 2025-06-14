/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/type.h>
#include <monolog/utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

static char g_buf[BUF_SIZE];

void format_type_name(const Type *type, int *pos) {
    switch (type->id) {
    case TYPE_INT:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - (size_t)*pos, "int");

        break;
    case TYPE_STRING:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - (size_t)*pos, "string");

        break;
    case TYPE_VOID:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - (size_t)*pos, "void");

        break;
    case TYPE_OPTION:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - (size_t)*pos, "option<");

        if (type->opt_type.type) {
            format_type_name(type->opt_type.type, pos);
        }

        *pos += snprintf(g_buf + *pos, BUF_SIZE - (size_t)*pos, ">");

        break;
    case TYPE_LIST:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - (size_t)*pos, "list<");
        format_type_name(type->list_type.type, pos);
        *pos += snprintf(g_buf + *pos, BUF_SIZE - (size_t)*pos, ">");

        break;
    case TYPE_ERROR:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - (size_t)*pos, "<error>");

        break;
    case TYPE_NIL:
        *pos += snprintf(g_buf + *pos, BUF_SIZE - (size_t)*pos, "nil");

        break;
    }
}

const char *type_name(const Type *type) {
    int pos = 0;

    memset(g_buf, 0, sizeof(g_buf));
    format_type_name(type, &pos);

    return g_buf;
}

bool type_convertable(const Type *self, const Type *type) {
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

void type_system_init(TypeSystem *self) {
    hashmap_init(&self->types);
    vec_init(&self->type_names, sizeof(char *));

    Type builtin_int = {TYPE_INT, NULL, {0}};
    self->builtin_int = type_system_register(self, &builtin_int);

    Type builtin_string = {TYPE_STRING, NULL, {0}};
    self->builtin_string = type_system_register(self, &builtin_string);

    Type builtin_void = {TYPE_VOID, NULL, {0}};
    self->builtin_void = type_system_register(self, &builtin_void);

    Type opt_type = {TYPE_OPTION, NULL, {0}};
    self->opt_type = type_system_register(self, &opt_type);

    Type error_type = {TYPE_ERROR, NULL, {0}};
    self->error_type = type_system_register(self, &error_type);

    Type nil_type = {TYPE_NIL, NULL, {0}};
    self->nil_type = type_system_register(self, &nil_type);
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
    int pos = 0;
    format_type_name(type, &pos);
    char *name = cstr_dup(g_buf);

    if (!hashmap_get(&self->types, name)) {
        vec_push(&self->type_names, name);
    }

    return name;
}

Type *type_system_register(TypeSystem *self, const Type *type) {
    int pos = 0;
    format_type_name(type, &pos);
    Type *existing_type = hashmap_get(&self->types, g_buf);

    if (existing_type) {
        return existing_type;
    } else {
        Type *new_type = mem_alloc(sizeof(*new_type));
        char *name = cstr_dup(g_buf);

        memcpy(new_type, type, sizeof(*new_type));
        new_type->name = name;

        hashmap_add(&self->types, name, new_type);

        return new_type;
    }
}

Type *type_system_get(TypeSystem *self, const char *name) {
    return hashmap_get(&self->types, name);
}
