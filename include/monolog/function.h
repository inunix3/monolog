#pragma once

#include "ast.h"
#include "type.h"
#include "vector.h"

typedef struct FnParam {
    Type *type;
    char *name;
} FnParam;

typedef struct Function {
    Type *type;
    char *name;
    Vector params; /* Vector<FnParam> */
    AstNode *body;
    bool free_body;
} Function;

void fn_deinit(Function *self);
