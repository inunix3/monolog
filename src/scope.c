#include <monolog/scope.h>

#include <stdlib.h>

void scope_init(Scope *self) { hashmap_init(&self->vars); }

void scope_deinit(Scope *self) {
    scope_clear(self);
    hashmap_deinit(&self->vars);
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
}

void scope_add_var(Scope *scope, Variable *var) {
    Variable *old_var = hashmap_get(&scope->vars, var->name);

    if (old_var) {
        hashmap_remove(&scope->vars, old_var->name);

        free(old_var->name);
        old_var->name = NULL;

        free(old_var);
    }

    hashmap_add(&scope->vars, var->name, var);
}
