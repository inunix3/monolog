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
    Vector builtin_fn_args;

    Vector dmsgs; /* Vector<DiagnosticMessage> */
    bool had_error;
    bool fatal_error;
} SemChecker;

void semck_init(SemChecker *self, TypeSystem *types);
void semck_deinit(SemChecker *self);
bool semck_check(
    SemChecker *self, const Ast *ast, HashMap *vars, HashMap *funcs
);
void semck_reset(SemChecker *self);
