/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#define SUITE_NAME invalid
#define PARSER_SHOULD_FAIL 1

#include "shared.h"

TEST_STRING_AGAINST_FILE(unknown_token, "@~`", "unknown-token.txt")
TEST_STRING_AGAINST_FILE(incomplete_expr_1, "1 +", "incomplete-expr-1.txt")
TEST_STRING_AGAINST_FILE(incomplete_expr_2, "-", "incomplete-expr-2.txt")

TEST_STRING_AGAINST_FILE(
    incomplete_expr_3, "5 * (3 + )", "incomplete-expr-3.txt"
)

TEST_STRING_AGAINST_FILE(incomplete_expr_4, "2 + (3 *", "incomplete-expr-4.txt")

TEST_STRING_AGAINST_FILE(
    incomplete_expr_5, "2 + (3 * 2", "incomplete-expr-5.txt"
)

TEST_STRING_AGAINST_FILE(incomplete_expr_6, "((1 + 2)", "incomplete-expr-6.txt")
TEST_STRING_AGAINST_FILE(invalid_expr_1, "+ / 3", "invalid-expr-1.txt")
TEST_STRING_AGAINST_FILE(invalid_expr_2, "1 + ;", "invalid-expr-2.txt")
TEST_STRING_AGAINST_FILE(invalid_expr_3, "1 2", "invalid-expr-3.txt")
TEST_STRING_AGAINST_FILE(invalid_expr_4, "()", "invalid-expr-4.txt")

TEST_STRING_AGAINST_FILE(
    fn_call_bad_arg,
    "foo(@);",
    "fn-call-bad-arg.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_call_missing_rparen,
    "foo(;",
    "fn-call-missing-rparen.txt"
)

TEST_STRING_AGAINST_FILE(
    list_subscript_missing_rbracket,
    "list[2 +",
    "list-subscript-missing-rbracket.txt"
)

TEST_STRING_AGAINST_FILE(
    list_subscript_bad_expression_1,
    "list[@]",
    "list-subscript-bad-expression-1.txt"
)

TEST_STRING_AGAINST_FILE(
    list_subscript_bad_expression_2,
    "list[2 + @]",
    "list-subscript-bad-expression-2.txt"
)

TEST_STRING_AGAINST_FILE(
    unterminated_string, "\"Hello, World!", "unterminated-string.txt"
)

TEST_STRING_AGAINST_FILE(
    if_missing_cond,
    "if 42;",
    "if-missing-cond.txt"
)

TEST_STRING_AGAINST_FILE(
    extra_else,
    "else 115;",
    "extra-else.txt"
)

TEST_STRING_AGAINST_FILE(
    while_missing_cond,
    "while 115;",
    "while-missing-cond.txt"
)

TEST_STRING_AGAINST_FILE(
    for_missing_all,
    "for 94;",
    "for-missing-all.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_in_for_init_clause,
    "for (void foo() {}; 1; 2);",
    "fn-in-for-init-clause.txt"
)

TEST_STRING_AGAINST_FILE(
    var_with_bad_name,
    "int 324 = 5; string @~`; int +; int int; string int;",
    "var-with-bad-name.txt"
)

TEST_STRING_AGAINST_FILE(
    var_with_bad_assign,
    "int a + 5; string var var2 \"Stringy var\"; int x string str_var; int y while 94;",
    "var-with-bad-assign.txt"
)

TEST_STRING_AGAINST_FILE(
    var_with_bad_value,
    "int a = @",
    "var-with-bad-value.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_with_bad_name,
    "int 1231() {}",
    "fn-decl-with-bad-name.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_with_bad_body,
    "void foo() @",
    "fn-decl-with-bad-body.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_with_bad_name_block,
    "int 1231() { 115; }",
    "fn-decl-with-bad-name-block.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_with_missing_rparen,
    "int foo(int a, int b",
    "fn-decl-with-missing-rparen.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_with_bad_param_name,
    "void foo(int @, int b) {}",
    "fn-decl-with-bad-param-name.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_with_bad_param_type,
    "void foo(3434 a, int b) {}",
    "fn-decl-with-bad-param-type.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_with_missing_comma,
    "void foo(int a int b) {}",
    "fn-decl-with-missing-comma.txt"
)

SUITE(SUITE_NAME) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(unknown_token);
    RUN_TEST(incomplete_expr_1);
    RUN_TEST(incomplete_expr_2);
    RUN_TEST(incomplete_expr_3);
    RUN_TEST(incomplete_expr_4);
    RUN_TEST(incomplete_expr_5);
    RUN_TEST(incomplete_expr_6);
    RUN_TEST(invalid_expr_1);
    RUN_TEST(invalid_expr_2);
    RUN_TEST(invalid_expr_3);
    RUN_TEST(invalid_expr_4);
    RUN_TEST(fn_call_bad_arg);
    RUN_TEST(fn_call_missing_rparen);
    RUN_TEST(list_subscript_missing_rbracket);
    RUN_TEST(list_subscript_bad_expression_1);
    RUN_TEST(list_subscript_bad_expression_2);
    RUN_TEST(unterminated_string);
    RUN_TEST(if_missing_cond);
    RUN_TEST(extra_else);
    RUN_TEST(while_missing_cond);
    RUN_TEST(for_missing_all);
    RUN_TEST(fn_in_for_init_clause);
    RUN_TEST(var_with_bad_name);
    RUN_TEST(var_with_bad_assign);
    RUN_TEST(var_with_bad_value);
    RUN_TEST(fn_decl_with_bad_name);
    RUN_TEST(fn_decl_with_bad_body);
    RUN_TEST(fn_decl_with_bad_name_block);
    RUN_TEST(fn_decl_with_missing_rparen);
    RUN_TEST(fn_decl_with_bad_param_name);
    RUN_TEST(fn_decl_with_bad_param_type);
    RUN_TEST(fn_decl_with_missing_comma);
}
