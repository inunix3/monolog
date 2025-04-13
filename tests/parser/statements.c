/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include "shared.h"

TEST_STRING_AGAINST_FILE(semicolon_misuse_1, ";", "empty.txt")
TEST_STRING_AGAINST_FILE(semicolon_misuse_2, "; ; ; ", "empty.txt")
TEST_STRING_AGAINST_FILE(semicolon_misuse_3, "; ; ;;;", "empty.txt")
TEST_STRING_AGAINST_FILE(semicolon_misuse_4, ";;;;2+2;;;;;;", "semicolon-misuse-4.txt")
TEST_STRING_AGAINST_FILE(semicolon_misuse_5, ";1 * 2", "semicolon-misuse-5.txt")
TEST_STRING_AGAINST_FILE(semicolon_misuse_6, "1 / 2;;3", "semicolon-misuse-6.txt")
TEST_STRING_AGAINST_FILE(multiple_exprs_1, "1 + 2; 3 * 4;", "multiple-exprs-1.txt")
TEST_STRING_AGAINST_FILE(multiple_exprs_2, "((1)); 2 + 3; 4 * (5 + 6)", "multiple-exprs-2.txt")

SUITE(statements) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(semicolon_misuse_1);
    RUN_TEST(semicolon_misuse_2);
    RUN_TEST(semicolon_misuse_3);
    RUN_TEST(semicolon_misuse_4);
    RUN_TEST(semicolon_misuse_5);
    RUN_TEST(semicolon_misuse_6);
    RUN_TEST(multiple_exprs_1);
    RUN_TEST(multiple_exprs_2);
}
