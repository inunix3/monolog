#pragma once

#include "ast.h"
#include "diagnostic.h"
#include "vector.h"

#include <stdbool.h>

typedef struct SemChecker {
    Vector dmsgs; /* Vector<DiagnosticMessage> */
    bool error_state;
} SemChecker;

void semck_init(SemChecker *self);
void semck_deinit(SemChecker *self);
bool semck_check(SemChecker *self, const Ast *ast);

