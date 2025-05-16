/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

#include "ast.h"
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
    const AstNode *node;

    union {
        Value val;
        Variable *var;
        Value *ref;
        char *char_ref;
    };
} ExprResult;
