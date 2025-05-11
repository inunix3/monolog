#include <monolog/scope.h>

#include <stdlib.h>

void scope_init(Scope *self) {
    vec_init(&self->values, sizeof(Value));
    vec_init(&self->strings, sizeof(StrBuf *));
    vec_init(&self->lists, sizeof(Vector *));

    hashmap_init(&self->vars);
}

void scope_deinit(Scope *self) {
    scope_clear(self);
    hashmap_deinit(&self->vars);
    vec_deinit(&self->lists);
    vec_deinit(&self->strings);
    vec_deinit(&self->values);
}

void scope_clear(Scope *self) {
    for (HashMapIter it = hashmap_iter(&self->vars); it.bucket != NULL;
         hashmap_iter_next(&it)) {
        Variable *var = it.bucket->value;

        hashmap_remove(&self->vars, var->name);

        free(var->name);
        var->name = NULL;

        free(var);
    }

    vec_clear(&self->values);

    StrBuf *strings = self->strings.data;

    for (size_t i = 0; i < self->strings.len; ++i) {
        str_deinit(&strings[i]);
    }

    vec_clear(&self->strings);

    Vector *lists = self->lists.data;

    for (size_t i = 0; i < lists->len; ++i) {
        vec_deinit(&lists[i]);
    }

    vec_clear(&self->lists);
}

void scope_add_var(Scope *self, Variable *var) {
    Variable *old_var = hashmap_get(&self->vars, var->name);

    if (old_var) {
        hashmap_remove(&self->vars, old_var->name);

        free(old_var->name);
        old_var->name = NULL;

        free(old_var);
    }

    hashmap_add(&self->vars, var->name, var);
}

Value *scope_new_value(Scope *self, Type *type) {
    Value *val = vec_emplace(&self->values);
    val->type = type;
    val->scope = self;

    return val;
}

StrBuf *scope_new_string(Scope *self) {
    StrBuf *str = vec_emplace(&self->strings);

    vec_push(&self->strings, str);

    return str;
}

Vector *scope_new_list(Scope *self) {
    Vector *list = vec_emplace(&self->lists);
    vec_init(list, sizeof(Value));

    vec_push(&self->lists, list);

    return list;
}
