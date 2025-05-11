#pragma once

#include "variable.h"
#include "value.h"

typedef enum ExprResultKind {
    EXPR_ERROR,
    EXPR_HALT,
    EXPR_VALUE,
    EXPR_VAR,
    EXPR_REF,
    EXPR_CHAR_REF
} ExprResultKind;

typedef struct ExprResult {
    ExprResultKind kind;

    union {
        Value val;
        Variable *var;
        Value *ref;
        char *char_ref;
    };
} ExprResult;
