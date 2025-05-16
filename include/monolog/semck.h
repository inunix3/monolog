/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

#include "ast.h"
#include "diagnostic.h"
#include "environment.h"
#include "hashmap.h"
#include "type.h"
#include "vector.h"

#include <stdbool.h>

typedef struct SemChecker {
    Environment env;
    TypeSystem *types;
    int loop_depth;

    Vector dmsgs; /* Vector<DiagnosticMessage> */
    bool had_error;
} SemChecker;

void semck_init(SemChecker *self, TypeSystem *types);
void semck_deinit(SemChecker *self);
bool semck_check(
    SemChecker *self, const Ast *ast, HashMap *vars, HashMap *funcs
);
void semck_reset(SemChecker *self);
