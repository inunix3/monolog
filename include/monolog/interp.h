#pragma once

#include "ast.h"
#include "environment.h"
#include "type.h"
#include "value.h"

#include <stdbool.h>

typedef struct Interpreter {
    Environment env;
    TypeSystem *types;
    Vector strings; /* Vector<StrBuf> */
    Vector lists; /* Vector<Vector<Value>> */
    Vector opts; /* Vector<Value> */

    Ast *ast;
    int exit_code;
    bool halt;
    bool had_error;
    bool log_errors;
} Interpreter;

void interp_init(Interpreter *self, Ast *ast, TypeSystem *types);
void interp_deinit(Interpreter *self);
int interp_walk(Interpreter *self);
Value interp_eval(Interpreter *self);
