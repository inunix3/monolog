#pragma once

#include "hashmap.h"
#include "variable.h"

typedef struct Scope {
    HashMap vars; /* HashMap<char *, Variable *> */
    Vector allocated_values; /* Vector<Value *> */
    Vector values; /* Vector<Value *> */
    Vector strings; /* Vector<StrBuf *> */
    Vector lists; /* Vector<Vector<Value> *> */
} Scope;

void scope_init(Scope *self);
void scope_deinit(Scope *self);
void scope_clear(Scope *self);
void scope_add_var(Scope *self, Variable *var);
Value *scope_new_value(Scope *self, Type *type);
StrBuf *scope_new_string(Scope *self);
Vector *scope_new_list(Scope *self);
