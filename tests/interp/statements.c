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

TEST stmt_if_true(void) {
    run("if (1) { exit(1); }");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(1, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST stmt_if_false(void) {
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

/* TODO: test loop statements */

SUITE(statements) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(print);
    RUN_TEST(println);
    RUN_TEST(print_line_buffering);
    RUN_TEST(exit_stmt);
    RUN_TEST(stmt_if_true);
    RUN_TEST(stmt_if_false);
    RUN_TEST(stmt_else_if);
    RUN_TEST(stmt_else);
}
