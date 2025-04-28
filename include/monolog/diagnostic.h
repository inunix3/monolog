#pragma once

#include "lexer.h"
#include "type.h"

typedef enum DiagnosticKind {
    DIAGNOSTIC_INTERNAL_ERROR,
    DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION,
    DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION,
    DIAGNOSTIC_BAD_SUFFIX_OPERAND_COMBINATION,
    DIAGNOSTIC_MISMATCHED_TYPES,
    DIAGNOSTIC_UNDECLARED_VARIABLE,
    DIAGNOSTIC_UNDECLARED_FUNCTION,
    DIAGNOSTIC_UNDEFINED_VARIABLE,
    DIAGNOSTIC_PARAM_REDECLARATION,
    DIAGNOSTIC_FN_REDEFINITION,
    DIAGNOSTIC_TOO_FEW_ARGS,
    DIAGNOSTIC_TOO_MANY_ARGS,
    DIAGNOSTIC_BAD_ARG_TYPE,
    DIAGNOSTIC_BAD_INDEX_TYPE,
    DIAGNOSTIC_EXPR_NOT_INDEXABLE,
    DIAGNOSTIC_EXPR_NOT_MUTABLE
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
            TokenKind op;
            Type *type;
        } suffix_op_comb;

        struct {
            Type *expected;
            Type *found;
        } type_mismatch;

        struct {
            char *name;
        } undef_sym;

        struct {
            char *name;
        } param_redecl;

        struct {
            char *name;
        } fn_redef;

        struct {
            size_t expected;
            size_t supplied;
        } bad_arg_count;

        struct {
            Type *expected;
            Type *found;
        } bad_arg_type;

        struct {
            Type *found;
        } bad_index_type;
    };
} DiagnosticMessage;

void dmsg_deinit(DiagnosticMessage *dmsg);
const char *dmsg_to_str(const DiagnosticMessage *dmsg);
