#include "shared.h"

TEST div_by_zero(void) {
    Value v = eval("115 / 0");

    ASSERT_EQ(TYPE_ERROR, v.type->id);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(-1, g_interp.exit_code);
    ASSERT_EQ(true, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST mod_by_zero(void) {
    Value v = eval("115 % 0");

    ASSERT_EQ(TYPE_ERROR, v.type->id);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(-1, g_interp.exit_code);
    ASSERT_EQ(true, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

SUITE(invalid) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(div_by_zero);
    RUN_TEST(mod_by_zero);
}
