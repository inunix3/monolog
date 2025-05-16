/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

#include "ast.h"
#include "value.h"

typedef enum StmtResultKind {
    STMT_ERROR,
    STMT_HALT,
    STMT_VOID,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_RETURN
} StmtResultKind;

typedef struct StmtResult {
    StmtResultKind kind;
    const AstNode *node;
    Value ret_val;
} StmtResult;
