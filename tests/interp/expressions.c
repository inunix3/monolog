#include "shared.h"

TEST empty_input(void) {
    run("");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST int_literal(void) {
    run("exit(123456789);");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(123456789, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST unary_minus_int(void) {
    run("exit(-115);");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(-115, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST unary_plus_int(void) {
    run("exit(+115);");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(115, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

SUITE(expressions) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(empty_input);
    RUN_TEST(int_literal);
    RUN_TEST(unary_minus_int);
    RUN_TEST(unary_plus_int);
}
