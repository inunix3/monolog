/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/diagnostic.h>
#include <monolog/type.h>

#include <stdio.h>

#define BUFFER_SIZE 4096

static char g_buf[BUFFER_SIZE];

const char *dmsg_to_str(const DiagnosticMessage *dmsg) {
    switch (dmsg->kind) {
    case DIAGNOSTIC_INTERNAL_ERROR:
        return "an internal error occurred during semantic checking";
    case DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION:
        snprintf(
            g_buf, BUFFER_SIZE, "bad operand combination for %s: %s and %s",
            token_kind_to_str(dmsg->binary_op_comb.op),
            dmsg->binary_op_comb.t1->name, dmsg->binary_op_comb.t2->name
        );

        break;
    case DIAGNOSTIC_BAD_UNARY_OPERAND:
        snprintf(
            g_buf, BUFFER_SIZE, "bad operand type for unary %s: %s",
            token_kind_to_str(dmsg->unary_op_comb.op),
            dmsg->unary_op_comb.type->name
        );

        break;
    case DIAGNOSTIC_BAD_SUFFIX_OPERAND_COMBINATION:
        snprintf(
            g_buf, BUFFER_SIZE, "bad operand type for suffix %s: %s",
            token_kind_to_str(dmsg->suffix_op_comb.op),
            dmsg->suffix_op_comb.type->name
        );

        break;
    case DIAGNOSTIC_MISMATCHED_TYPES: {
        const Type *expected = dmsg->bad_arg_type.expected;
        const Type *found = dmsg->bad_arg_type.found;

        if (expected->id == TYPE_OPTION) {
            snprintf(
                g_buf, BUFFER_SIZE, "expected %s, %s or nil, but found %s",
                expected->name, expected->opt_type.type->name, found->name
            );
        } else {
            snprintf(
                g_buf, BUFFER_SIZE, "expected %s, found %s", expected->name,
                found->name
            );
        }

        break;
    }
    case DIAGNOSTIC_EXPECTED_LIST: {
        const Type *found = dmsg->bad_arg_type.found;

        snprintf(
            g_buf, BUFFER_SIZE, "expected list, but found %s", found->name
        );

        break;
    }
    case DIAGNOSTIC_UNDECLARED_VARIABLE:
        snprintf(
            g_buf, BUFFER_SIZE, "undeclared variable %s", dmsg->undef_sym.name
        );

        break;
    case DIAGNOSTIC_UNDECLARED_FUNCTION:
        snprintf(
            g_buf, BUFFER_SIZE, "undeclared function %s", dmsg->undef_sym.name
        );

        break;
    case DIAGNOSTIC_PARAM_REDECLARATION:
        snprintf(
            g_buf, BUFFER_SIZE, "parameter %s is already declared",
            dmsg->param_redecl.name
        );

        break;
    case DIAGNOSTIC_FN_REDEFINITION:
        snprintf(
            g_buf, BUFFER_SIZE, "function %s is already defined",
            dmsg->fn_redef.name
        );

        break;
    case DIAGNOSTIC_FN_BAD_PLACE:
        snprintf(
            g_buf, BUFFER_SIZE, "function can't be declared inside other function"
        );

        break;
    case DIAGNOSTIC_TOO_FEW_ARGS:
        snprintf(
            g_buf, BUFFER_SIZE, "too few arguments: expected %zu, supplied %zu",
            dmsg->bad_arg_count.expected, dmsg->bad_arg_count.supplied
        );

        break;
    case DIAGNOSTIC_TOO_MANY_ARGS:
        snprintf(
            g_buf, BUFFER_SIZE,
            "too many arguments: expected %zu, supplied %zu",
            dmsg->bad_arg_count.expected, dmsg->bad_arg_count.supplied
        );

        break;
    case DIAGNOSTIC_BAD_ARG_TYPE: {
        const Type *expected = dmsg->bad_arg_type.expected;
        const Type *found = dmsg->bad_arg_type.found;

        if (expected->id == TYPE_OPTION) {
            snprintf(
                g_buf, BUFFER_SIZE,
                "bad argument type: expected %s, %s or nil, but found %s",
                expected->name, expected->opt_type.type->name, found->name
            );
        } else {
            snprintf(
                g_buf, BUFFER_SIZE, "bad argument type: expected %s, found %s",
                expected->name, found->name
            );
        }

        break;
    }
    case DIAGNOSTIC_BAD_INDEX_TYPE:
        snprintf(
            g_buf, BUFFER_SIZE,
            "index expression must result in int, but found %s",
            type_name(dmsg->bad_index_type.found)
        );

        break;
    case DIAGNOSTIC_EXPR_NOT_INDEXABLE:
        snprintf(
            g_buf, BUFFER_SIZE,
            "expression is not indexable: expected list or string"
        );

        break;
    case DIAGNOSTIC_EXPR_NOT_MUTABLE:
        snprintf(g_buf, BUFFER_SIZE, "expression cannot be mutated");

        break;
    case DIAGNOSTIC_BREAK_OUTSIDE_LOOP:
        snprintf(g_buf, BUFFER_SIZE, "break must be inside loop");

        break;
    case DIAGNOSTIC_CONTINUE_OUTSIDE_LOOP:
        snprintf(g_buf, BUFFER_SIZE, "continue must be inside loop");

        break;
    case DIAGNOSTIC_RETURN_OUTSIDE_FUNCTION:
        snprintf(g_buf, BUFFER_SIZE, "return must be inside function");

        break;
    case DIAGNOSTIC_VOID_RETURN:
        snprintf(g_buf, BUFFER_SIZE, "void function cannot return value");

        break;
    case DIAGNOSTIC_VOID_VAR:
        snprintf(g_buf, BUFFER_SIZE, "variable cannot be void");

        break;
    }

    return g_buf;
}
