#pragma once

#include "value.h"

typedef enum StmtResultKind {
    STMT_ERROR,
    STMT_VOID,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_RETURN
} StmtResultKind;

typedef struct StmtResult {
    StmtResultKind kind;
    Value ret_val;
} StmtResult;
