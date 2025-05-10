#pragma once

#include "hashmap.h"
#include "variable.h"

typedef struct Scope {
    HashMap vars; /* HashMap<char *, Variable *> */
} Scope;

bool scope_init(Scope *self);
void scope_deinit(Scope *self);
void scope_clear(Scope *self);
bool scope_add_var(Scope *scope, Variable *var);
