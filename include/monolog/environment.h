#pragma once

#include "ast.h"
#include "hashmap.h"
#include "type.h"
#include "value.h"
#include "vector.h"

#include <stdbool.h>

typedef struct Variable {
    Type *type;
    char *name;
    Value val;
    bool defined;
    bool is_param;
} Variable;

typedef struct Function {
    Type *type;
    const char *name;
    Vector params; /* Vector<Variable> */
    AstNode *body;
} Function;

typedef struct Scope {
    HashMap vars; /* HashMap<char *, Variable *> */
} Scope;

void scope_init(Scope *self);
void scope_deinit(Scope *self);
void scope_clear(Scope *self);

typedef struct Envrionment {
    Vector scopes; /* Vector<Scope> */
    Scope *global_scope;
    Scope *curr_scope;
    HashMap funcs; /* HashMap<char *, Function *> */
    Function *curr_fn;
} Environment;

void env_init(Environment *self);
void env_deinit(Environment *self);
Variable *env_find_var(const Environment *self, const char *name);
void env_reset(Environment *self);
Scope *env_enter_scope(Environment *self);
void env_leave_scope(Environment *self);
