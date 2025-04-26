#pragma once

#include "lexer.h"
#include "type.h"

typedef enum DiagnosticKind {
    DIAGNOSTIC_INTERNAL_ERROR,
    DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION,
    DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION,
    DIAGNOSTIC_MISMATCHED_TYPES,
    DIAGNOSTIC_UNDECLARED_SYMBOL,
    DIAGNOSTIC_UNDEFINED_VARIABLE
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

        struct {
            Type *expected;
            Type *found;
        } type_mismatch;

        struct {
            const char *name;
        } undef_sym;
    };
} DiagnosticMessage;

const char *dmsg_to_str(const DiagnosticMessage *dmsg);
