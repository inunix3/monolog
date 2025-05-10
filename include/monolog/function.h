#pragma once

#include "ast.h"
#include "expr_result.h"
#include "type.h"
#include "vector.h"

typedef struct FnParam {
    Type *type;
    char *name;
} FnParam;

typedef struct Interpreter Interpreter;
typedef ExprResult (*FnBuiltin)(Interpreter *self, Value *args);

typedef struct Function {
    Type *type;
    char *name;
    Vector params; /* Vector<FnParam> */
    bool free_body;
    bool is_builtin;

    union {
        FnBuiltin builtin;
        AstNode *body;
    };
} Function;

void fn_deinit(Function *self);
