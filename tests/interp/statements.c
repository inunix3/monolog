#include "shared.h"

TEST print(void) {
    run("print(\"Hello, World!\")");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST println(void) {
    run("println(\"Hello, World!\")");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST print_line_buffering(void) {
    run("print(\"Hello, \"); println(\"World!\")");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST exit_stmt(void) {
    run("exit(-115)");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(-115, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST if_true(void) {
    run("if (1) { exit(1); }");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(1, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST if_false(void) {
    run("if (0) { exit(1); }");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST stmt_else_if(void) {
    run("if (5 < 3) {\n"
        "    exit(1);\n"
        "} else if (5 == 5) {\n"
        "    exit(2);\n"
        "} else {\n"
        "    exit(3);\n"
        "}");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(2, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST stmt_else(void) {
    run("if (5 < 3) {\n"
        "    exit(1);\n"
        "} else if (4 == 5) {\n"
        "    exit(2);\n"
        "} else {\n"
        "    exit(3);\n"
        "}");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(3, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST stmt_while(void) {
    run(
        "int i = 0;\n"
        "while (i < 10000) {\n"
        "  ++i;\n"
        "}\n"
        "\n"
        "exit(i);"
    );

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(10000, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST stmt_for(void) {
    run(
        "int outer_i = 0;\n"
        "for (int i = 0; i < 10000; i++) {\n"
        "  outer_i = i;\n"
        "}\n"
        "\n"
        "exit(outer_i);"
    );

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(9999, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST for_without_init(void) {
    run(
        "int i = 0;\n"
        "for (; i < 10000; ++i) {\n"
        "  i + 1;\n"
        "}\n"
        "\n"
        "exit(i);"
    );

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(10000, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST for_without_cond(void) {
    run(
        "for (int i = 0;; ++i) {\n"
        "  if (i >= 2) exit(i);\n"
        "  i = i + 1;\n"
        "}"
    );

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(2, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST for_without_iter(void) {
    run(
        "int outer_i = 0;\n"
        "for (int i = 0; i < 10000;) {\n"
        "  i = i + 1;\n"
        "  outer_i = i;\n"
        "}\n"
        "\n"
        "exit(outer_i);"
    );

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(10000, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST infinite_for(void) {
    run(
        "int i = 0;\n"
        "for (;;) {\n"
        "  if (i >= 10000) exit(i);\n"
        "  ++i;"
        "}\n"
    );

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(10000, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

SUITE(statements) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(print);
    RUN_TEST(println);
    RUN_TEST(print_line_buffering);
    RUN_TEST(exit_stmt);
    RUN_TEST(if_true);
    RUN_TEST(if_false);
    RUN_TEST(stmt_else_if);
    RUN_TEST(stmt_else);
    RUN_TEST(stmt_while);
    RUN_TEST(stmt_for);
    RUN_TEST(for_without_init);
    RUN_TEST(for_without_cond);
    RUN_TEST(for_without_iter);
    RUN_TEST(infinite_for);
}
