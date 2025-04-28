#include "shared.h"

TEST binary_int_string(void) {
    CHECK_FAIL("2 + \"hi\"");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_OP_PLUS, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for +: int and string",
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
        "bad operand combination for +: string and int",
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
    ASSERT_STR_EQ("expected int, found string", dmsg_to_str(&NTH_DMSG(0)));

    ASSERT_EQ(DIAGNOSTIC_MISMATCHED_TYPES, NTH_DMSG(1).kind);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(1).type_mismatch.expected->id);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(1).type_mismatch.found->id);
    ASSERT_STR_EQ("expected string, found int", dmsg_to_str(&NTH_DMSG(1)));

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(2).kind);
    ASSERT_EQ(TOKEN_OP_PLUS, NTH_DMSG(2).binary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(2).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(2).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for +: string and int",
        dmsg_to_str(&NTH_DMSG(2))
    );

    PASS();
}

TEST undeclared_variable(void) {
    CHECK_FAIL("int pythagoras = side * side + 3 * 3; int side = 4;");

    ASSERT_EQ(DIAGNOSTIC_UNDECLARED_VARIABLE, NTH_DMSG(0).kind);
    ASSERT_STR_EQ("side", NTH_DMSG(0).undef_sym.name);
    ASSERT_STR_EQ("undeclared variable side", dmsg_to_str(&NTH_DMSG(0)));

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

TEST param_redecl(void) {
    CHECK_FAIL("int foo(int a, int b, int a);");

    ASSERT_EQ(DIAGNOSTIC_PARAM_REDECLARATION, NTH_DMSG(0).kind);
    ASSERT_STR_EQ("a", NTH_DMSG(0).param_redecl.name);
    ASSERT_STR_EQ("parameter a is already declared", dmsg_to_str(&NTH_DMSG(0)));

    PASS();
}

TEST fn_call_too_many_arguments(void) {
    CHECK_FAIL("int foo(); foo(3, 4); int bar(int a); bar(5, 6)");

    ASSERT_EQ(DIAGNOSTIC_TOO_MANY_ARGS, NTH_DMSG(0).kind);
    ASSERT_EQ(0, NTH_DMSG(0).bad_arg_count.expected);
    ASSERT_EQ(2, NTH_DMSG(0).bad_arg_count.supplied);
    ASSERT_STR_EQ(
        "too many arguments: expected 0, supplied 2", dmsg_to_str(&NTH_DMSG(0))
    );

    ASSERT_EQ(DIAGNOSTIC_TOO_MANY_ARGS, NTH_DMSG(1).kind);
    ASSERT_EQ(1, NTH_DMSG(1).bad_arg_count.expected);
    ASSERT_EQ(2, NTH_DMSG(1).bad_arg_count.supplied);
    ASSERT_STR_EQ(
        "too many arguments: expected 1, supplied 2", dmsg_to_str(&NTH_DMSG(1))
    );

    PASS();
}

TEST fn_call_too_few_arguments(void) {
    CHECK_FAIL("int foo(int a, int b, int c); foo(3, 4);");

    ASSERT_EQ(DIAGNOSTIC_TOO_FEW_ARGS, NTH_DMSG(0).kind);
    ASSERT_EQ(3, NTH_DMSG(0).bad_arg_count.expected);
    ASSERT_EQ(2, NTH_DMSG(0).bad_arg_count.supplied);
    ASSERT_STR_EQ(
        "too few arguments: expected 3, supplied 2", dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST array_sub_not_indexable(void) {
    CHECK_FAIL("int a = 115; a[5]");

    ASSERT_EQ(DIAGNOSTIC_EXPR_NOT_INDEXABLE, NTH_DMSG(0).kind);
    ASSERT_STR_EQ(
        "expression is not indexable: expected array or string",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST array_sub_bad_index_type(void) {
    CHECK_FAIL("[int, 5] array; array[\"115\"]");

    ASSERT_EQ(DIAGNOSTIC_BAD_INDEX_TYPE, NTH_DMSG(0).kind);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).bad_index_type.found->id);
    ASSERT_STR_EQ(
        "index expression must result in int, but found string",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST immutable_expr(void) {
    CHECK_FAIL(
        "int foo();\n"
        "int? opt = nil;\n"
        "[int, 5] array;\n"
        "foo() = 5; 3 + 4 = 8;\n"
        "--3; --\"adada\";\n"
        "++3; ++\"adada\";\n"
        "3--; \"adada\"--;\n"
        "3++; \"adada\"++;\n"
    );

    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(DIAGNOSTIC_EXPR_NOT_MUTABLE, NTH_DMSG(i).kind);
        ASSERT_STR_EQ(
            "expression cannot be mutated", dmsg_to_str(&NTH_DMSG(i))
        );
    }

    PASS();
}

TEST size_op_can_be_applied_only_for_string_and_array(void) {
    CHECK_FAIL("int? a = 5; #a; int b = 0; #b");

    ASSERT_EQ(DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_OP_HASHTAG, NTH_DMSG(0).unary_op_comb.op);
    ASSERT_EQ(TYPE_OPTION, NTH_DMSG(0).unary_op_comb.type->id);
    ASSERT_STR_EQ(
        "bad operand type for unary #: option<int>", dmsg_to_str(&NTH_DMSG(0))
    );

    ASSERT_EQ(DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION, NTH_DMSG(1).kind);
    ASSERT_EQ(TOKEN_OP_HASHTAG, NTH_DMSG(1).unary_op_comb.op);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(1).unary_op_comb.type->id);
    ASSERT_STR_EQ(
        "bad operand type for unary #: int", dmsg_to_str(&NTH_DMSG(1))
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
    RUN_TEST(param_redecl);
    RUN_TEST(fn_call_too_many_arguments);
    RUN_TEST(fn_call_too_few_arguments);
    RUN_TEST(array_sub_not_indexable);
    RUN_TEST(array_sub_bad_index_type);
    RUN_TEST(immutable_expr);
    RUN_TEST(size_op_can_be_applied_only_for_string_and_array);
}
