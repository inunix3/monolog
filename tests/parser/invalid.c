/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

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
TEST_STRING_AGAINST_FILE(invalid_expr_1, "+ * 3", "invalid-expr-1.txt")
TEST_STRING_AGAINST_FILE(invalid_expr_2, "1 + ;", "invalid-expr-2.txt")
TEST_STRING_AGAINST_FILE(invalid_expr_3, "1 2", "invalid-expr-3.txt")
TEST_STRING_AGAINST_FILE(invalid_expr_4, "()", "invalid-expr-4.txt")

SUITE(invalid) {
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
}
