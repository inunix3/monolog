#include "shared.h"

TEST binary_int_plus_string(void) {
    CHECK_FAIL("2 + \"hi\"");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_PLUS, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for +: int and string",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST binary_int_minus_string(void) {
    CHECK_FAIL("2 - \"hi\"");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_MINUS, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for -: int and string",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST binary_int_mul_string(void) {
    CHECK_FAIL("2 * \"hi\"");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_MUL, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for *: int and string",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST binary_int_div_string(void) {
    CHECK_FAIL("2 / \"hi\"");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_DIV, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for /: int and string",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST binary_int_mod_string(void) {
    CHECK_FAIL("2 % \"hi\"");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_MOD, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for %: int and string",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST binary_string_plus_int(void) {
    CHECK_FAIL("\"hi\" + 2");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_PLUS, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for +: string and int",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST binary_string_minus_int(void) {
    CHECK_FAIL("\"hi\" - 2");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_MINUS, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for -: string and int",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST binary_string_mul_int(void) {
    CHECK_FAIL("\"hi\" * 2");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_MUL, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for *: string and int",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST binary_string_div_int(void) {
    CHECK_FAIL("\"hi\" / 2");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_DIV, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for /: string and int",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST binary_string_mod_int(void) {
    CHECK_FAIL("\"hi\" % 2");

    ASSERT_EQ(DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_MOD, NTH_DMSG(0).binary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).binary_op_comb.t1->id);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).binary_op_comb.t2->id);
    ASSERT_STR_EQ(
        "bad operand combination for %: string and int",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST unary_string(void) {
    CHECK_FAIL("-\"a\"; +\"a\"; !\"a\"");

    ASSERT_EQ(DIAGNOSTIC_BAD_UNARY_OPERAND, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_MINUS, NTH_DMSG(0).unary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).unary_op_comb.type->id);
    ASSERT_STR_EQ(
        "bad operand type for unary -: string", dmsg_to_str(&NTH_DMSG(0))
    );

    ASSERT_EQ(DIAGNOSTIC_BAD_UNARY_OPERAND, NTH_DMSG(1).kind);
    ASSERT_EQ(TOKEN_PLUS, NTH_DMSG(1).unary_op_comb.op);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(1).unary_op_comb.type->id);
    ASSERT_STR_EQ(
        "bad operand type for unary +: string", dmsg_to_str(&NTH_DMSG(1))
    );

    ASSERT_EQ(DIAGNOSTIC_BAD_UNARY_OPERAND, NTH_DMSG(2).kind);
    ASSERT_EQ(TOKEN_EXCL, NTH_DMSG(2).unary_op_comb.op);
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
    ASSERT_EQ(TOKEN_PLUS, NTH_DMSG(2).binary_op_comb.op);
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

TEST param_redecl(void) {
    CHECK_FAIL("int foo(int a, int b, int a) { return a + b + c; }");

    ASSERT_EQ(DIAGNOSTIC_PARAM_REDECLARATION, NTH_DMSG(0).kind);
    ASSERT_STR_EQ("a", NTH_DMSG(0).param_redecl.name);
    ASSERT_STR_EQ("parameter a is already declared", dmsg_to_str(&NTH_DMSG(0)));

    PASS();
}

TEST fn_call_too_many_arguments(void) {
    CHECK_FAIL("int foo() { return 5; }"
               "foo(3, 4);"
               "int bar(int a) { return a; }"
               "bar(5, 6)");

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
    CHECK_FAIL("int foo(int a, int b, int c) { return a + b + c; } foo(3, 4);");

    ASSERT_EQ(DIAGNOSTIC_TOO_FEW_ARGS, NTH_DMSG(0).kind);
    ASSERT_EQ(3, NTH_DMSG(0).bad_arg_count.expected);
    ASSERT_EQ(2, NTH_DMSG(0).bad_arg_count.supplied);
    ASSERT_STR_EQ(
        "too few arguments: expected 3, supplied 2", dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST list_sub_not_indexable(void) {
    CHECK_FAIL("int a = 115; a[5]");

    ASSERT_EQ(DIAGNOSTIC_EXPR_NOT_INDEXABLE, NTH_DMSG(0).kind);
    ASSERT_STR_EQ(
        "expression is not indexable: expected list or string",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST list_sub_bad_index_type(void) {
    CHECK_FAIL("[int] list; list[\"115\"]");

    ASSERT_EQ(DIAGNOSTIC_BAD_INDEX_TYPE, NTH_DMSG(0).kind);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).bad_index_type.found->id);
    ASSERT_STR_EQ(
        "index expression must result in int, but found string",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST list_sub_bad_type(void) {
    CHECK_FAIL("[int] list; list[0] = \"Hello, World\"");

    ASSERT_EQ(DIAGNOSTIC_MISMATCHED_TYPES, NTH_DMSG(0).kind);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).type_mismatch.expected->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).type_mismatch.found->id);
    ASSERT_STR_EQ("expected int, found string", dmsg_to_str(&NTH_DMSG(0)));

    PASS();
}

TEST immutable_expr(void) {
    CHECK_FAIL("int foo() { return 115; }\n"
               "foo() = 5; 3 + 4 = 8;\n"
               "--3; --\"adada\";\n"
               "++3; ++\"adada\";\n"
               "3--; \"adada\"--;\n"
               "3++; \"adada\"++;\n");

    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(DIAGNOSTIC_EXPR_NOT_MUTABLE, NTH_DMSG(i).kind);
        ASSERT_STR_EQ(
            "expression cannot be mutated", dmsg_to_str(&NTH_DMSG(i))
        );
    }

    PASS();
}

TEST size_op_can_be_applied_only_for_string_and_list(void) {
    CHECK_FAIL("int? a = 5; #a; int b = 0; #b");

    ASSERT_EQ(DIAGNOSTIC_BAD_UNARY_OPERAND, NTH_DMSG(0).kind);
    ASSERT_EQ(TOKEN_HASHTAG, NTH_DMSG(0).unary_op_comb.op);
    ASSERT_EQ(TYPE_OPTION, NTH_DMSG(0).unary_op_comb.type->id);
    ASSERT_STR_EQ(
        "bad operand type for unary #: option<int>", dmsg_to_str(&NTH_DMSG(0))
    );

    ASSERT_EQ(DIAGNOSTIC_BAD_UNARY_OPERAND, NTH_DMSG(1).kind);
    ASSERT_EQ(TOKEN_HASHTAG, NTH_DMSG(1).unary_op_comb.op);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(1).unary_op_comb.type->id);
    ASSERT_STR_EQ(
        "bad operand type for unary #: int", dmsg_to_str(&NTH_DMSG(1))
    );

    PASS();
}

TEST push_op_can_be_applied_only_for_lists(void) {
    CHECK_FAIL("int a; a += 115;");

    ASSERT_EQ(DIAGNOSTIC_EXPECTED_LIST, NTH_DMSG(0).kind);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).type_mismatch.found->id);
    ASSERT_STR_EQ("expected list, but found int", dmsg_to_str(&NTH_DMSG(0)));

    PASS();
}

TEST if_condition_is_int(void) {
    CHECK_FAIL("if (\"a\");");

    ASSERT_EQ(DIAGNOSTIC_MISMATCHED_TYPES, NTH_DMSG(0).kind);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).type_mismatch.expected->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).type_mismatch.found->id);
    ASSERT_STR_EQ("expected int, found string", dmsg_to_str(&NTH_DMSG(0)));

    PASS();
}

TEST while_condition_is_int(void) {
    CHECK_FAIL("while (\"a\");");

    ASSERT_EQ(DIAGNOSTIC_MISMATCHED_TYPES, NTH_DMSG(0).kind);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).type_mismatch.expected->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).type_mismatch.found->id);
    ASSERT_STR_EQ("expected int, found string", dmsg_to_str(&NTH_DMSG(0)));

    PASS();
}

TEST for_condition_is_int(void) {
    CHECK_FAIL("for (;\"a\";);");

    ASSERT_EQ(DIAGNOSTIC_MISMATCHED_TYPES, NTH_DMSG(0).kind);
    ASSERT_EQ(TYPE_INT, NTH_DMSG(0).type_mismatch.expected->id);
    ASSERT_EQ(TYPE_STRING, NTH_DMSG(0).type_mismatch.found->id);
    ASSERT_STR_EQ("expected int, found string", dmsg_to_str(&NTH_DMSG(0)));

    PASS();
}

TEST fn_cant_be_declared_inside_fn(void) {
    CHECK_FAIL("void foo() { int bar() { return 5; } }");

    ASSERT_EQ(DIAGNOSTIC_FN_BAD_PLACE, NTH_DMSG(0).kind);
    ASSERT_STR_EQ(
        "function can't be declared inside other function",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST return_must_be_inside_fn(void) {
    CHECK_FAIL("return 5 + 5;");

    ASSERT_EQ(DIAGNOSTIC_RETURN_OUTSIDE_FUNCTION, NTH_DMSG(0).kind);
    ASSERT_STR_EQ(
        "return must be inside function",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST break_must_be_inside_loop(void) {
    CHECK_FAIL("break;");

    ASSERT_EQ(DIAGNOSTIC_BREAK_OUTSIDE_LOOP, NTH_DMSG(0).kind);
    ASSERT_STR_EQ(
        "break must be inside loop",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

TEST continue_must_be_inside_loop(void) {
    CHECK_FAIL("continue;");

    ASSERT_EQ(DIAGNOSTIC_CONTINUE_OUTSIDE_LOOP, NTH_DMSG(0).kind);
    ASSERT_STR_EQ(
        "continue must be inside loop",
        dmsg_to_str(&NTH_DMSG(0))
    );

    PASS();
}

SUITE(invalid) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(binary_int_plus_string);
    RUN_TEST(binary_int_minus_string);
    RUN_TEST(binary_int_mul_string);
    RUN_TEST(binary_int_div_string);
    RUN_TEST(binary_int_mod_string);
    RUN_TEST(binary_string_plus_int);
    RUN_TEST(binary_string_minus_int);
    RUN_TEST(binary_string_mul_int);
    RUN_TEST(binary_string_div_int);
    RUN_TEST(binary_string_mod_int);
    RUN_TEST(unary_string);
    RUN_TEST(var_decl);
    RUN_TEST(undeclared_variable);
    RUN_TEST(param_redecl);
    RUN_TEST(fn_call_too_many_arguments);
    RUN_TEST(fn_call_too_few_arguments);
    RUN_TEST(list_sub_not_indexable);
    RUN_TEST(list_sub_bad_index_type);
    RUN_TEST(list_sub_bad_type);
    RUN_TEST(immutable_expr);
    RUN_TEST(size_op_can_be_applied_only_for_string_and_list);
    RUN_TEST(push_op_can_be_applied_only_for_lists);
    RUN_TEST(if_condition_is_int);
    RUN_TEST(while_condition_is_int);
    RUN_TEST(for_condition_is_int);
    RUN_TEST(fn_cant_be_declared_inside_fn);
    RUN_TEST(return_must_be_inside_fn);
    RUN_TEST(break_must_be_inside_loop);
    RUN_TEST(continue_must_be_inside_loop);
}
