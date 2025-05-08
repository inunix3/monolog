#pragma once

#include "variable.h"
#include "value.h"

typedef enum ExprResultKind {
    EXPR_ERROR,
    EXPR_VALUE,
    EXPR_VAR,
    EXPR_REF
} ExprResultKind;

typedef struct ExprResult {
    ExprResultKind kind;

    union {
        Value val;
        Variable *var;
        Value *ref;
    };
} ExprResult;
