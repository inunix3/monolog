#pragma once

#include "function.h"
#include "hashmap.h"
#include "scope.h"
#include "variable.h"
#include "vector.h"

#include <stdbool.h>

typedef struct Envrionment {
    Vector scopes; /* Vector<Scope> */
    Scope *global_scope;
    HashMap funcs; /* HashMap<char *, Function *> */

    Scope *caller_scope;
    Scope *curr_scope;
    Scope *old_scope;
    Function *curr_fn;
    Function *old_fn;
} Environment;

void env_init(Environment *self);
void env_deinit(Environment *self);
Variable *env_find_var(const Environment *self, const char *name);
Function *env_find_fn(const Environment *self, const char *name);
void env_reset(Environment *self);
Scope *env_enter_scope(Environment *self);
void env_leave_scope(Environment *self);
Scope *env_enter_fn(Environment *self, Function *fn);
void env_leave_fn(Environment *self);
void env_save_caller(Environment *self, Scope *caller_scope);
void env_restore_caller(Environment *self);
void env_add_fn(Environment *self, Function *fn);
void env_add_local_var(Environment *self, Variable *var);
void env_add_global_var(Environment *self, Variable *var);
