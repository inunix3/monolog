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

typedef struct SemScope {
    HashMap variables; /* HashMap<char *, Type *> */
} SemScope;

void semscope_init(SemScope *self);
void semscope_deinit(SemScope *self);

typedef struct SemChecker {
    Vector scopes; /* Vector<SemScope> */
    SemScope *curr_scope;

    Vector dmsgs; /* Vector<DiagnosticMessage> */
    bool error_state;
} SemChecker;

void semck_init(SemChecker *self);
void semck_deinit(SemChecker *self);
bool semck_check(SemChecker *self, const Ast *ast);

