#pragma once

#include "ast.h"
#include "strbuf.h"
#include "type.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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
    bool halt;
    bool had_error;
    bool log_errors;
} Interpreter;

void interp_init(Interpreter *self, const Ast *ast);
void interp_deinit(Interpreter *self);
int interp_walk(Interpreter *self);
Value interp_eval(Interpreter *self);
