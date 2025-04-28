#pragma once

#include "ast.h"
#include "diagnostic.h"
#include "hashmap.h"
#include "type.h"
#include "vector.h"

#include <stdbool.h>

typedef struct SemVariable {
    Type *type;
    const char *name;
    bool defined;
    bool is_param;
} SemVariable;

typedef struct SemFunction {
    Type *type;
    const char *name;
    Vector params;
    bool defined;
} SemFunction;

typedef struct SemScope {
    HashMap vars; /* HashMap<char *, Type *> */
} SemScope;

void semscope_init(SemScope *self);
void semscope_deinit(SemScope *self);

typedef struct SemChecker {
    TypeSystem types;
    Vector scopes; /* Vector<SemScope> */
    SemScope *curr_scope;
    HashMap funcs; /* HashMap<char *, Type *> */

    Type *builtin_int;
    Type *builtin_string;
    Type *builtin_void;
    Type *error_type;
    Type *nil_type;

    SemFunction *curr_fn;
    int loop_depth;

    Vector dmsgs; /* Vector<DiagnosticMessage> */
    bool error_state;
} SemChecker;

void semck_init(SemChecker *self);
void semck_deinit(SemChecker *self);
bool semck_check(SemChecker *self, const Ast *ast);
