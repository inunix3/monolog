#pragma once

#include "ast.h"
#include "strbuf.h"
#include "type.h"

#include <stdint.h>

typedef int64_t Int;

typedef struct Value {
    TypeId type;

    union {
        Int i;
        StrBuf s;
    };
} Value;

typedef struct Interpreter {
    const Ast *ast;
    int exit_code;
    bool had_error;
    bool halt;
} Interpreter;

void interp_init(Interpreter *self, const Ast *ast);
void interp_deinit(Interpreter *self);
int interp_run(Interpreter *self);
