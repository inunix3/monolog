/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include "shared.h"

TEST empty_string_returns_empty_ast(void) {
    parse("");

    ASSERT_EQ(0, g_ast.nodes.len);

    PASS();
}

TEST whitespaces_return_empty_ast(void) {
    parse("    \t\t\t\t\n\r  \n   \r\t\n  ");

    ASSERT_EQ(0, g_ast.nodes.len);

    PASS();
}

TEST_STRING_AGAINST_FILE(int_42, "42", "int-42.txt")
TEST_STRING_AGAINST_FILE(int_0, "0", "int-0.txt")
TEST_STRING_AGAINST_FILE(int_1234567893, "1234567893", "int-1234567893.txt")

TEST_STRING_AGAINST_FILE(
    int_INT64_MAX, "9223372036854775807", "int-int64-max.txt"
)

TEST_STRING_AGAINST_FILE(unary_minus_int, "-115", "unary-minus-int.txt")
TEST_STRING_AGAINST_FILE(unary_plus_int, "+12939532", "unary-plus-int.txt")

TEST_STRING_AGAINST_FILE(
    unary_minus_separated_int, "-(100)", "unary-minus-separated-int.txt"
)

TEST_STRING_AGAINST_FILE(
    unary_nested_int, "-(-(+(-5)))", "unary-nested-int.txt"
)

TEST_STRING_AGAINST_FILE(
    binary_int_plus_int, "115 + 94", "binary-int-plus-int.txt"
)

TEST_STRING_AGAINST_FILE(
    binary_int_minus_int, "115 - 94", "binary-int-minus-int.txt"
)

TEST_STRING_AGAINST_FILE(
    binary_int_mul_int, "115 * 94", "binary-int-mul-int.txt"
)

TEST_STRING_AGAINST_FILE(
    binary_int_div_int, "115 / 94", "binary-int-div-int.txt"
)

TEST_STRING_AGAINST_FILE(
    binary_int_mod_int, "115 % 94", "binary-int-mod-int.txt"
)

TEST_STRING_AGAINST_FILE(precedence, "1 + 2 * 3 - 4 / 10", "precedence.txt")

TEST_STRING_AGAINST_FILE(
    left_associativity, "115 + 94 + 3 - 1 - 10", "left-associativity.txt"
)

TEST_STRING_AGAINST_FILE(grouping, "(115 + 94) / 34", "grouping.txt")

TEST_STRING_AGAINST_FILE(
    nested_grouping, "((((94 + 115) * 2) % 2 ) - 1) / 0", "nested-grouping.txt"
)

TEST_STRING_AGAINST_FILE(
    combination, "(1 + 2) * (3 - 4) / (9 * -(+10000 % (34 - -99999)))",
    "combination.txt"
)

TEST_STRING_AGAINST_FILE(
    string, "\"Hello, World! This is a one-line string\"", "string.txt"
)

TEST_STRING_AGAINST_FILE(
    multiline_string, "\"Hello\n        World!\"", "multiline-string.txt"
)

TEST_STRING_AGAINST_FILE(
    assignment_1,
    "5 = 6",
    "assignment-1.txt"
)

TEST_STRING_AGAINST_FILE(
    assignment_2,
    "foo = 5 + 6",
    "assignment-2.txt"
)

SUITE(expressions) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(empty_string_returns_empty_ast);
    RUN_TEST(whitespaces_return_empty_ast);
    RUN_TEST(int_42);
    RUN_TEST(int_0);
    RUN_TEST(int_1234567893);
    RUN_TEST(int_INT64_MAX);
    RUN_TEST(unary_minus_int);
    RUN_TEST(unary_plus_int);
    RUN_TEST(unary_minus_separated_int);
    RUN_TEST(unary_nested_int);
    RUN_TEST(binary_int_plus_int);
    RUN_TEST(binary_int_minus_int);
    RUN_TEST(binary_int_mul_int);
    RUN_TEST(binary_int_div_int);
    RUN_TEST(binary_int_mod_int);
    RUN_TEST(precedence);
    RUN_TEST(left_associativity);
    RUN_TEST(grouping);
    RUN_TEST(nested_grouping);
    RUN_TEST(combination);
    RUN_TEST(string);
    RUN_TEST(multiline_string);
    RUN_TEST(assignment_1);
    RUN_TEST(assignment_2);
}
