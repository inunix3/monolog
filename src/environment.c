#include <monolog/builtin_funcs.h>
#include <monolog/environment.h>
#include <monolog/interp.h>
#include <monolog/utils.h>

#include <stddef.h>
#include <stdlib.h>

static bool
add_fn(HashMap *funcs, Type *type, const char *name, FnBuiltin builtin) {
    Function *fn = mem_alloc(sizeof(*fn));

    if (!fn) {
        return false;
    }

    fn->type = type;
    fn->name = cstr_dup(name);
    fn->builtin = builtin;
    fn->free_body = false;
    fn->is_builtin = true;

    if (!fn->name || !vec_init(&fn->params, sizeof(FnParam))) {
        free(fn);

        return false;
    }

    if (!hashmap_add(funcs, name, fn)) {
        vec_deinit(&fn->params);
        free(fn->name);
        free(fn);

        return false;
    }

    return true;
}

static bool add_param(Function *fn, Type *type) {
    FnParam *param = vec_emplace(&fn->params);

    if (!param) {
        return false;
    }

    param->type = type;
    param->name = NULL;

    return true;
}

static bool add_builtin_funcs(HashMap *funcs, TypeSystem *types) {
    Type opt_int_descr = {TYPE_OPTION, NULL, {0}};
    opt_int_descr.opt_type.type = types->builtin_int;
    Type *opt_int = type_system_register(types, &opt_int_descr);

    Type opt_string_descr = {TYPE_OPTION, NULL, {0}};
    opt_string_descr.opt_type.type = types->builtin_string;
    Type *opt_string = type_system_register(types, &opt_string_descr);

    Type *fn_ret_types[] = {
        types->builtin_void, /* print */
        types->builtin_void, /* println */
        types->builtin_void, /* exit */
        opt_int,             /* input_int */
        opt_string,          /* input_string */
    };

    static const char *fn_names[] = {
        "print", "println", "exit", "input_int", "input_string"
    };

    static FnBuiltin builtins[] = {
        builtin_print, builtin_println, builtin_exit, builtin_input_int,
        builtin_input_string
    };

    for (size_t i = 0; i < ARRAY_SIZE(fn_names); ++i) {
        if (hashmap_get(funcs, fn_names[i])) {
            continue;
        }

        if (!add_fn(funcs, fn_ret_types[i], fn_names[i], builtins[i])) {
            return false;
        }
    }

    Function *print_fn = hashmap_get(funcs, "print");
    Function *println_fn = hashmap_get(funcs, "println");
    Function *exit_fn = hashmap_get(funcs, "exit");

    return add_param(print_fn, types->builtin_string) &&
           add_param(println_fn, types->builtin_string) &&
           add_param(exit_fn, types->builtin_int);
}

bool env_init(Environment *self, TypeSystem *types) {
    if (!vec_init(&self->scopes, sizeof(Scope))) {
        return false;
    }

    Scope global_scope;

    if (!scope_init(&global_scope)) {
        return false;
    }

    if (!vec_push(&self->scopes, &global_scope)) {
        return false;
    }

    self->global_scope = &VEC_LAST(&self->scopes, Scope);
    self->curr_scope = &VEC_LAST(&self->scopes, Scope);
    self->caller_scope = NULL;
    self->old_scope = NULL;
    self->curr_fn = NULL;
    self->old_fn = NULL;

    return hashmap_init(&self->funcs) && add_builtin_funcs(&self->funcs, types);
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

    if (!new_scope || !scope_init(new_scope)) {
        return NULL;
    }

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

bool env_add_fn(Environment *self, Function *fn) {
    return hashmap_add(&self->funcs, fn->name, fn);
}

bool env_add_local_var(Environment *self, Variable *var) {
    return scope_add_var(self->curr_scope, var);
}

bool env_add_global_var(Environment *self, Variable *var) {
    return scope_add_var(self->global_scope, var);
}
