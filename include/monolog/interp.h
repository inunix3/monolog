/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

#include "ast.h"
#include "environment.h"
#include "type.h"
#include "value.h"

#include <stdbool.h>

typedef struct Interpreter {
    Environment env;
    TypeSystem *types;

    /* Temporary storage for function's arguments */
    Vector builtin_fn_args;

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
