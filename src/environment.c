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

void env_init(Environment *self) {
    vec_init(&self->scopes, sizeof(Scope));

    Scope global_scope;
    scope_init(&global_scope);
    vec_push(&self->scopes, &global_scope);
    self->curr_scope = &VEC_LAST(&self->scopes, Scope);
    self->global_scope = &VEC_LAST(&self->scopes, Scope);
    self->caller_scope = NULL;

    self->old_scope = NULL;
    self->old_fn = NULL;
    self->old_caller_scope = NULL;

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
        FnParam *params = fn->params.data;

        for (size_t i = 0; i < fn->params.len; ++i) {
            FnParam *param = &params[i];

            param->type = NULL;

            free(param->name);
            param->name = NULL;
        }

        vec_deinit(&fn->params);

        if (fn->free_body) {
            astnode_destroy(fn->body);
            fn->body = NULL;
        }

        free(fn->name);
        fn->name = NULL;

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

    self->curr_scope = &VEC_LAST(&self->scopes, Scope);
    self->global_scope = &VEC_LAST(&self->scopes, Scope);
    self->caller_scope = NULL;
    self->old_scope = NULL;
    self->old_fn = NULL;
    self->old_caller_scope = NULL;
    self->curr_fn = NULL;

    for (size_t i = 1; i < self->scopes.len; ++i) {
        vec_pop(&self->scopes);
    }

    for (HashMapIter it = hashmap_iter(&self->funcs); it.bucket != NULL;
         hashmap_iter_next(&it)) {
        Function *fn = it.bucket->value;

        vec_deinit(&fn->params);
        hashmap_remove(&self->funcs, fn->name);

        vec_deinit(&fn->params);

        if (fn->free_body) {
            astnode_destroy(fn->body);
            fn->body = NULL;
        }

        free(fn->name);
        fn->name = NULL;

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

void env_save_caller(Environment *self, Scope *caller_scope) {
    self->old_caller_scope = self->caller_scope;
    self->caller_scope = caller_scope;
}

void env_restore_caller(Environment *self) {
    self->curr_scope = self->old_scope;
    self->caller_scope = self->old_caller_scope;
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
