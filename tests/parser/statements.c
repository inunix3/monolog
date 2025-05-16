/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#define SUITE_NAME statements

#include "shared.h"

TEST_STRING_AGAINST_FILE(semicolon_misuse_1, ";", "empty.txt")
TEST_STRING_AGAINST_FILE(semicolon_misuse_2, "; ; ; ", "empty.txt")
TEST_STRING_AGAINST_FILE(semicolon_misuse_3, "; ; ;;;", "empty.txt")

TEST_STRING_AGAINST_FILE(
    semicolon_misuse_4, ";;;;2+2;;;;;;", "semicolon-misuse-4.txt"
)

TEST_STRING_AGAINST_FILE(semicolon_misuse_5, ";1 * 2", "semicolon-misuse-5.txt")

TEST_STRING_AGAINST_FILE(
    semicolon_misuse_6, "1 / 2;;3", "semicolon-misuse-6.txt"
)

TEST_STRING_AGAINST_FILE(
    multiple_exprs_1, "1 + 2; 3 * 4;", "multiple-exprs-1.txt"
)

TEST_STRING_AGAINST_FILE(
    multiple_exprs_2, "((1)); 2 + 3; 4 * (5 + 6)", "multiple-exprs-2.txt"
)

TEST_STRING_AGAINST_FILE(
    return_keyword,
    "return (2 + 4) * 3",
    "return-keyword.txt"
)

TEST_STRING_AGAINST_FILE(
    break_keyword,
    "break",
    "break-keyword.txt"
)

TEST_STRING_AGAINST_FILE(
    continue_keyword,
    "continue",
    "continue-keyword.txt"
)

TEST_STRING_AGAINST_FILE(empty_block, "{}", "empty-block.txt")

TEST_STRING_AGAINST_FILE(
    block_1_statement,
    "{\n"
    "2 + 2;\n"
    "}",
    "block-1-statement.txt"
)

TEST_STRING_AGAINST_FILE(
    block_multiple_statements,
    "{"
    "  println(\"aaabbbbccccddeeefff\");\n"
    "  2 + 2;\n"
    "  print(\"\");"
    "  ;\n"
    "}",
    "block-multiple-statements.txt"
)

TEST_STRING_AGAINST_FILE(empty_if, "if (0);", "empty-if.txt")

TEST_STRING_AGAINST_FILE(
    if_statement_1,
    "if (1)\n"
    "  println(\"Hello, World!\");",
    "if-statement-1.txt"
)

TEST_STRING_AGAINST_FILE(
    if_statement_2,
    "if ((1 + 2) * (3 - 4) / (9 * -(+10000 % (34 - -99999)))) {\n"
    "  2 + 3 + 4;\n"
    "  println(\"Hello, World!\");\n"
    "}",
    "if-statement-2.txt"
)

TEST_STRING_AGAINST_FILE(
    if_nested_oneline, "if (1) if (2) if (3) 115;", "if-nested-oneline.txt"
)

TEST_STRING_AGAINST_FILE(
    if_nested_multiline,
    "if (1) {\n"
    "    if (2) {\n"
    "      if (3) {\n"
    "        115;\n"
    "      }\n"
    "    }\n"
    "}",
    "if-nested-multiline.txt"
)

TEST_STRING_AGAINST_FILE(
    if_else_nested_oneline, "if (1) if (2) if (3) 115; else 94;",
    "if-else-nested-oneline.txt"
)

TEST_STRING_AGAINST_FILE(
    if_else_nested_multiline,
    "if (1) {\n"
    "    if (2) {\n"
    "        115;\n"
    "    } else {\n"
    "        94;\n"
    "    }\n"
    "}",
    "if-else-nested-multiline.txt"
)

TEST_STRING_AGAINST_FILE(
    dangling_else,
    "if (1)\n"
    "    if (2)\n"
    "        115;\n"
    "    else\n"
    "        94;\n",
    "dangling-else.txt"
)

TEST_STRING_AGAINST_FILE(
    else_if_oneline, "if (1) 115; else if (2) 94; else if (3) 1; else 4;",
    "else-if-oneline.txt"
)

TEST_STRING_AGAINST_FILE(
    else_if_multiline_1,
    "if (1)\n"
    "    115;\n"
    "else if (2)\n"
    "    94;\n"
    "else if (3)\n"
    "    1;\n"
    "else\n"
    "    4;",
    "else-if-multiline-1.txt"
)

TEST_STRING_AGAINST_FILE(
    else_if_multiline_2,
    "if (1) {\n"
    "    115;\n"
    "} else if (2) {\n"
    "    94;\n"
    "} else if (3) {\n"
    "    1;\n"
    "} else {\n"
    "    4;\n"
    "}",
    "else-if-multiline-2.txt"
)

TEST_STRING_AGAINST_FILE(empty_while, "while (0);", "empty-while.txt")

TEST_STRING_AGAINST_FILE(
    while_statement_1,
    "while (1)\n"
    "  println(\"Hello, World!\");",
    "while-statement-1.txt"
)

TEST_STRING_AGAINST_FILE(
    while_statement_2,
    "while ((1 + 2) * (3 - 4) / (9 * -(+10000 % (34 - -99999)))) {\n"
    "  2 + 3 + 4;\n"
    "  println(\"Hello, World!\");\n"
    "}",
    "while-statement-2.txt"
)

TEST_STRING_AGAINST_FILE(
    while_nested_oneline, "while (1) while (2) while (3) 115;",
    "while-nested-oneline.txt"
)

TEST_STRING_AGAINST_FILE(
    while_nested_multiline_1,
    "while (1)\n"
    "    while (2)\n"
    "        while (3)\n"
    "            115;\n",
    "while-nested-multiline-1.txt"
)

TEST_STRING_AGAINST_FILE(
    while_nested_multiline_2,
    "while (1) {\n"
    "    while (2) {\n"
    "        while (3) {\n"
    "            115;\n"
    "        }\n"
    "    }\n"
    "}",
    "while-nested-multiline-2.txt"
)

TEST_STRING_AGAINST_FILE(empty_for, "for (1; 2; 3);", "empty-for.txt")

TEST_STRING_AGAINST_FILE(
    for_statement_1,
    "for (1; 2; 3)\n"
    "    println(\"in for\");",
    "for-statement-1.txt"
)

TEST_STRING_AGAINST_FILE(
    for_statement_2,
    "for (1 + 3 * -(3 - 4); +4 - -115; 94) {\n"
    "    println(\"in for 1x\");"
    "    println(\"in for 2x\");"
    "    println(\"in for 3x\");"
    "}",
    "for-statement-2.txt"
)

TEST_STRING_AGAINST_FILE(
    for_nested_oneline, "for (1; 2; 3) for (4; 5; 6) for (7; 8; 9) 115;",
    "for-nested-oneline.txt"
)

TEST_STRING_AGAINST_FILE(
    for_nested_multiline_1,
    "for (1; 2; 3)\n"
    "    for (4; 5; 6)\n"
    "        for (7; 8; 9)\n"
    "            115;\n",
    "for-nested-multiline-1.txt"
)

TEST_STRING_AGAINST_FILE(
    for_nested_multiline_2,
    "for (1; 2; 3) {\n"
    "    for (4; 5; 6) {\n"
    "        for (7; 8; 9) {\n"
    "            115;\n"
    "        }\n"
    "    }\n"
    "}",
    "for-nested-multiline-2.txt"
)

TEST_STRING_AGAINST_FILE(
    for_without_init, "for (; 2; 3);", "for-without-init.txt"
)

TEST_STRING_AGAINST_FILE(
    for_without_cond, "for (1; ; 3);", "for-without-cond.txt"
)

TEST_STRING_AGAINST_FILE(
    for_without_iter, "for (1; 2;);", "for-without-iter.txt"
)

TEST_STRING_AGAINST_FILE(
    for_without_init_and_iter, "for (; 2;);", "for-without-init-and-iter.txt"
)

TEST_STRING_AGAINST_FILE(infinite_for, "for (;;);", "infinite-for.txt")

TEST_STRING_AGAINST_FILE(
    var_without_value, "int a; string b; void c;", "var-without-value.txt"
)

TEST_STRING_AGAINST_FILE(
    var_with_value,
    "int SomeIntVariable = 115; string string_var = \"Hello, World!\";",
    "var-with-value.txt"
)

TEST_STRING_AGAINST_FILE(
    var_with_expression,
    "int myvar_115 = (1 + 2) * (3 - 4) / (9 * -(+10000 % (34 - -99999))); "
    "string concat = \"Hello \" + \"World\";",
    "var-with-expression.txt"
)


TEST_STRING_AGAINST_FILE(
    option_type,
    "int? result_of_operation = nil;",
    "option-type.txt"
)

TEST_STRING_AGAINST_FILE(
    nested_option_type,
    "string????? a;",
    "nested-option-type.txt"
)

TEST_STRING_AGAINST_FILE(
    empty_list,
    "[int] my_list;",
    "empty-list.txt"
)

TEST_STRING_AGAINST_FILE(
    nested_lists,
    "[[[int]]] foo;",
    "nested-lists.txt"
)

TEST_STRING_AGAINST_FILE(
    list_with_initial_size,
    "[int, 2 + 5 * 9] foo;",
    "list-with-initial-size.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_empty_without_params,
    "int foo() {}",
    "fn-decl-empty-without-params.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_empty_with_1_param,
    "string bar(int x) {}",
    "fn-decl-empty-with-1-param.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_empty_with_params,
    "void __someFunc115(int x, string bar, int y) {}",
    "fn-decl-empty-with-params.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_empty_body,
    "void foo() {}",
    "fn-decl-empty-body.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_with_1_statement,
    "int foo() {\n"
    "  println(\"In foo()\");\n"
    "}",
    "fn-decl-with-1-statement.txt"
)

TEST_STRING_AGAINST_FILE(
    fn_decl_with_more_statements,
    "void foo() {\n"
    "  println(\"In foo()\");\n"
    "  int x = 5;\n"
    "  x = 6 + 8;\n"
    "  println(\"After x = 14\");\n"
    "}",
    "fn-decl-with-more-statements.txt"
)

SUITE(SUITE_NAME) {
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
    RUN_TEST(return_keyword);
    RUN_TEST(break_keyword);
    RUN_TEST(continue_keyword);
    RUN_TEST(empty_block);
    RUN_TEST(block_1_statement);
    RUN_TEST(block_multiple_statements);
    RUN_TEST(empty_if);
    RUN_TEST(if_statement_1);
    RUN_TEST(if_statement_2);
    RUN_TEST(if_nested_oneline);
    RUN_TEST(if_nested_multiline);
    RUN_TEST(if_else_nested_oneline);
    RUN_TEST(if_else_nested_multiline);
    RUN_TEST(dangling_else);
    RUN_TEST(else_if_oneline);
    RUN_TEST(else_if_multiline_1);
    RUN_TEST(else_if_multiline_2);
    RUN_TEST(empty_while);
    RUN_TEST(while_statement_1);
    RUN_TEST(while_statement_2);
    RUN_TEST(while_nested_oneline);
    RUN_TEST(while_nested_multiline_1);
    RUN_TEST(while_nested_multiline_2);
    RUN_TEST(empty_for);
    RUN_TEST(for_statement_1);
    RUN_TEST(for_statement_2);
    RUN_TEST(for_nested_oneline);
    RUN_TEST(for_nested_multiline_1);
    RUN_TEST(for_nested_multiline_2);
    RUN_TEST(for_without_init);
    RUN_TEST(for_without_cond);
    RUN_TEST(for_without_iter);
    RUN_TEST(for_without_init_and_iter);
    RUN_TEST(infinite_for);
    RUN_TEST(var_without_value);
    RUN_TEST(var_with_value);
    RUN_TEST(var_with_expression);
    RUN_TEST(option_type);
    RUN_TEST(nested_option_type);
    RUN_TEST(empty_list);
    RUN_TEST(nested_lists);
    RUN_TEST(list_with_initial_size);
    RUN_TEST(fn_decl_empty_without_params);
    RUN_TEST(fn_decl_empty_with_1_param);
    RUN_TEST(fn_decl_empty_with_params);
    RUN_TEST(fn_decl_empty_body);
    RUN_TEST(fn_decl_with_1_statement);
    RUN_TEST(fn_decl_with_more_statements);
}
