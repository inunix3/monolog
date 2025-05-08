#include <monolog/environment.h>

#include <stddef.h>
#include <stdlib.h>

void env_init(Environment *self) {
    vec_init(&self->scopes, sizeof(Scope));

    Scope global_scope;
    scope_init(&global_scope);
    vec_push(&self->scopes, &global_scope);

    self->global_scope = &VEC_LAST(&self->scopes, Scope);
    self->curr_scope = &VEC_LAST(&self->scopes, Scope);
    self->caller_scope = NULL;
    self->old_scope = NULL;
    self->curr_fn = NULL;
    self->old_fn = NULL;

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

        fn_deinit(fn);
        free(fn);
    }

    vec_deinit(&self->scopes);
    hashmap_deinit(&self->funcs);
}

Variable *env_find_var(const Environment *self, const char *name) {
    Variable *var = NULL;

    for (size_t i = self->scopes.len - 1; i >= 1; --i) {
        const Scope *scopes = self->scopes.data;
        const Scope *scope = &scopes[i];

        if (scope == self->caller_scope &&
            self->caller_scope != self->global_scope) {
            break;
        }

        var = hashmap_get(&scope->vars, name);

        if (var) {
            break;
        }
    }

    if (!var) {
        var = hashmap_get(&self->global_scope->vars, name);
    }

    return var;
}

Function *env_find_fn(const Environment *self, const char *name) {
    return hashmap_get(&self->funcs, name);
}

void env_reset(Environment *self) {
    Scope *scopes = self->scopes.data;

    for (size_t i = 1; i < self->scopes.len; ++i) {
        scope_deinit(&scopes[i]);
    }

    scope_clear(&scopes[0]);

    self->global_scope = &VEC_LAST(&self->scopes, Scope);
    self->curr_scope = &VEC_LAST(&self->scopes, Scope);
    self->caller_scope = NULL;
    self->old_scope = NULL;
    self->curr_fn = NULL;
    self->old_fn = NULL;

    for (size_t i = 1; i < self->scopes.len; ++i) {
        vec_pop(&self->scopes);
    }

    for (HashMapIter it = hashmap_iter(&self->funcs); it.bucket != NULL;
         hashmap_iter_next(&it)) {
        Function *fn = it.bucket->value;

        hashmap_remove(&self->funcs, fn->name);
        fn_deinit(fn);

        free(fn);
    }
}

Scope *env_enter_scope(Environment *self) {
    Scope *new_scope = vec_emplace(&self->scopes);
    scope_init(new_scope);

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

Scope *env_enter_fn(Environment *self, Function *fn) {
    self->old_scope = self->curr_scope;

    Scope *fn_scope = vec_emplace(&self->scopes);
    scope_init(fn_scope);

    self->old_fn = self->curr_fn;
    self->curr_scope = fn_scope;
    self->curr_fn = fn;

    return fn_scope;
}

void env_leave_fn(Environment *self) {
    scope_deinit(self->curr_scope);
    vec_pop(&self->scopes);

    self->curr_fn = self->old_fn;
}

void env_add_fn(Environment *self, Function *fn) {
    hashmap_add(&self->funcs, fn->name, fn);
}

void env_add_local_var(Environment *self, Variable *var) {
    scope_add_var(self->curr_scope, var);
}

void env_add_global_var(Environment *self, Variable *var) {
    scope_add_var(self->global_scope, var);
}
