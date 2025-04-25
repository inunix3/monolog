#pragma once

#include "lexer.h"
#include "type.h"

typedef enum DiagnosticKind {
    DIAGNOSTIC_INTERNAL_ERROR,
    DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION,
    DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION
} DiagnosticKind;

typedef struct DiagnosticMessage {
    enum DiagnosticKind kind;

    union {
        struct {
            TokenKind op;
            Type *t1;
            Type *t2;
        } binary_op_comb;

        struct {
            TokenKind op;
            Type *type;
        } unary_op_comb;
    };
} DiagnosticMessage;

const char *dmsg_to_str(const DiagnosticMessage *dmsg);
