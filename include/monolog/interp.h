#pragma once

#include "ast.h"
#include "environment.h"
#include "type.h"
#include "value.h"

#include <stdbool.h>

typedef enum ExprResultKind {
    EXPR_ERROR,
    EXPR_VALUE,
    EXPR_VAR
} ExprResultKind;

typedef struct ExprResult {
    ExprResultKind kind;

    union {
        Value val;
        Variable *var;
    };
} ExprResult;

typedef enum StmtResultKind {
    STMT_ERROR,
    STMT_VOID,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_RETURN
} StmtResultKind;

typedef struct StmtResult {
    StmtResultKind kind;
    Value ret_value;
} StmtResult;

typedef struct Interpreter {
    Environment env;
    TypeSystem *types;
    Vector strings; /* Vector<StrBuf *> */

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
