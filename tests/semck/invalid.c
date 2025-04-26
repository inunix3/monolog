#include "shared.h"

TEST binary_int_string(void) {
    CHECK_FAIL("2 + \"hi\"");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_OP_PLUS, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for +: integer and string",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST binary_string_int(void) {
    CHECK_FAIL("\"hi\" + 2");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_OP_PLUS, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for +: string and integer",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST unary_string(void) {
    CHECK_FAIL("-\"a\"; +\"a\"; !\"a\"");

    ASSERT_EQ(DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_OP_MINUS, NTH_DMSG(0).unary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).unary_op_comb.type->id);
    ASSERT_STR_EQ(
        "bad operand type for unary -: string", dmsg_to_str(&NTH_DMSG(0))
    );

    ASSERT_EQ(DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION, NTH_DMSG(1).kind);
    ASSERT_EQ(TOKEN_OP_PLUS, NTH_DMSG(1).unary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(1).unary_op_comb.type->id);
    ASSERT_STR_EQ(
        "bad operand type for unary +: string", dmsg_to_str(&NTH_DMSG(1))
    );

    ASSERT_EQ(DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION, NTH_DMSG(2).kind);
    ASSERT_EQ(TOKEN_OP_EXCL, NTH_DMSG(2).unary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(2).unary_op_comb.type->id);
    ASSERT_STR_EQ(
        "bad operand type for unary !: string", dmsg_to_str(&NTH_DMSG(2))
    );

    PASS();
}

TEST var_decl(void) {
    CHECK_FAIL("int a = \"Hi\"; string b = 115 + 94; int c = \"Hi\" + 115");

    ASSERT_EQ(DIAGNOSTIC_MISMATCHED_TYPES, NTH_DMSG(0).kind);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).type_mismatch.expected->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).type_mismatch.found->id);
    ASSERT_STR_EQ("expected integer, found string", dmsg_to_str(&NTH_DMSG(0)));

    ASSERT_EQ(DIAGNOSTIC_MISMATCHED_TYPES, NTH_DMSG(1).kind);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(1).type_mismatch.expected->id);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(1).type_mismatch.found->id);
    ASSERT_STR_EQ("expected string, found integer", dmsg_to_str(&NTH_DMSG(1)));

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(2).kind);
    ASSERT_EQ(TOKEN_OP_PLUS, NTH_DMSG(2).binary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(2).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(2).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for +: string and integer",
        dmsg_to_str(&NTH_DMSG(2))
    );

    PASS();
}

TEST undeclared_variable(void) {
    CHECK_FAIL("int pythagoras = side * side + 3 * 3; int side = 4;");

    ASSERT_EQ(DIAGNOSTIC_UNDECLARED_SYMBOL, NTH_DMSG(0).kind);
    ASSERT_STR_EQ("side", NTH_DMSG(0).undef_sym.name);
    ASSERT_STR_EQ("undeclared symbol side", dmsg_to_str(&NTH_DMSG(0)));

    PASS();
}

TEST undefined_variable(void) {
    CHECK_FAIL("int side; int pythagoras = side * side + 3 * 3;");

    ASSERT_EQ(DIAGNOSTIC_UNDEFINED_VARIABLE, NTH_DMSG(0).kind);
    ASSERT_STR_EQ("side", NTH_DMSG(0).undef_sym.name);
    ASSERT_STR_EQ(
        "variable side exists, but no value is assigned to it",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

SUITE(invalid) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(binary_int_string);
    RUN_TEST(binary_string_int);
    RUN_TEST(unary_string);
    RUN_TEST(var_decl);
    RUN_TEST(undeclared_variable);
    RUN_TEST(undefined_variable);
}
