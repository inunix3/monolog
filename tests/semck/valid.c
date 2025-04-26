#include "shared.h"

#define CHECK_EXPRESSION(_name, _expr)                                         \
    TEST _name(void) {                                                         \
        CHECK(_expr);                                                          \
                                                                               \
        PASS();                                                                \
    }

CHECK_EXPRESSION(binary_int_int, "115 + 94")
CHECK_EXPRESSION(binary_string_string, "\"Hello\" + \", World!\"")
CHECK_EXPRESSION(unary_int, "-115; +94; !0")
CHECK_EXPRESSION(var_int, "int a = 5 + -(10 / 2)")
CHECK_EXPRESSION(var_string, "string var = \"Hello\" + \", World\" + \"!\"")
CHECK_EXPRESSION(var_usage, "int a = 5; int b = a + 5")

CHECK_EXPRESSION(
    var_scope, "int a = 5;\n"
               "{\n"
               "  int b = a + 5;\n"
               "  string s = \"monolog\";\n"
               "  {\n"
               "    int s = 5;\n"
               "    int c = a + b * s;\n"
               "  }\n"
               "}"
)

SUITE(valid) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(binary_int_int);
    RUN_TEST(binary_string_string);
    RUN_TEST(unary_int);
    RUN_TEST(var_int);
    RUN_TEST(var_string);
    RUN_TEST(var_usage);
    RUN_TEST(var_scope);
}
