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
            type_name(dmsg->binary_op_comb.t1),
            type_name(dmsg->binary_op_comb.t2)
        );

        return g_buf;
    case DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION:
        snprintf(
            g_buf, BUFFER_SIZE, "bad operand type for unary %s: %s",
            token_kind_to_str(dmsg->unary_op_comb.op),
            type_name(dmsg->unary_op_comb.type)
        );

        return g_buf;
    }
}
