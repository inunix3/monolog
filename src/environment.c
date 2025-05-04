#include <monolog/environment.h>

#include <stddef.h>
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

void env_init(Environment *self) {
    vec_init(&self->scopes, sizeof(Scope));

    Scope global_scope;
    scope_init(&global_scope);
    vec_push(&self->scopes, &global_scope);
    self->curr_scope = &VEC_LAST(&self->scopes, Scope);
    self->global_scope = &VEC_LAST(&self->scopes, Scope);

    self->curr_fn = NULL;
    hashmap_init(&self->funcs);
}

void env_deinit(Environment *self) {
    Scope *scopes = self->scopes.data;

    for (size_t i = 0; i < self->scopes.len; ++i) {
        scope_deinit(&scopes[i]);
    }

    for (HashMapIter it = hashmap_iter(&self->funcs); it.bucket != NULL;
         hashmap_iter_next(&it)) {
        Function *fn = it.bucket->value;

        vec_deinit(&fn->params);
        free(fn);
    }

    vec_deinit(&self->scopes);
    hashmap_deinit(&self->funcs);
}

Variable *env_find_var(const Environment *self, const char *name) {
    Variable *var = NULL;

    for (size_t i = self->scopes.len - 1; i >= 0; --i) {
        const Scope *scopes = self->scopes.data;
        const Scope *scope = &scopes[i];

        var = hashmap_get(&scope->vars, name);

        if (var || i == 0) {
            break;
        }
    }

    if (!var && self->curr_fn) {
        Variable *vars = self->curr_fn->params.data;

        for (size_t i = 0; i < self->curr_fn->params.len; ++i) {
            var = &vars[i];

            if (var->name == name) {
                break;
            }
        }
    }

    return var;
}

void env_reset(Environment *self) {
    Scope *scopes = self->scopes.data;

    for (size_t i = 1; i < self->scopes.len; ++i) {
        scope_deinit(&scopes[i]);
    }

    scope_clear(&scopes[0]);

    self->curr_scope = &VEC_LAST(&self->scopes, Scope);
    self->global_scope = &VEC_LAST(&self->scopes, Scope);

    for (size_t i = 1; i < self->scopes.len; ++i) {
        vec_pop(&self->scopes);
    }

    for (HashMapIter it = hashmap_iter(&self->funcs); it.bucket != NULL;
         hashmap_iter_next(&it)) {
        Function *fn = it.bucket->value;

        vec_deinit(&fn->params);
        hashmap_remove(&self->funcs, fn->name);
        free(fn);
    }
}

Scope *env_enter_scope(Environment *self) {
    Scope new_scope;
    scope_init(&new_scope);
    vec_push(&self->scopes, &new_scope);

    self->curr_scope = &VEC_LAST(&self->scopes, Scope);

    return self->curr_scope;
}

void env_leave_scope(Environment *self) {
    if (self->curr_scope == self->global_scope) {
        return;
    }

    scope_deinit(self->curr_scope);
    vec_pop(&self->scopes);
    self->curr_scope = &VEC_LAST(&self->scopes, Scope);
}
