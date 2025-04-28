#include "shared.h"

#define CHECK_EXPR(_name, _expr)                                               \
    TEST _name(void) {                                                         \
        CHECK(_expr);                                                          \
                                                                               \
        PASS();                                                                \
    }

CHECK_EXPR(binary_int_int, "115 + 94")
CHECK_EXPR(binary_string_string, "\"Hello\" + \", World!\"")
CHECK_EXPR(unary_int, "-115; +94; !0")
CHECK_EXPR(var_int, "int a = 5 + -(10 / 2)")
CHECK_EXPR(var_string, "string var = \"Hello\" + \", World\" + \"!\"")
CHECK_EXPR(var_usage, "int a = 5; int b = a + 5")
CHECK_EXPR(var_assign, "int a = 5; a = 5; a = a * 2");
CHECK_EXPR(undef_var_assign, "int a; a = 5; int b = a + 5");

CHECK_EXPR(
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

CHECK_EXPR(fn_decl, "int foo();")

CHECK_EXPR(
    fn_decl_with_parameters, "int foo(int a, string b, int? c, string? d);"
)

CHECK_EXPR(
    fn_decl_recursion, "int foo(int a) { foo(a + 1); }"
)

CHECK_EXPR(fn_call, "string foo(); string b = foo();")
CHECK_EXPR(fn_call_with_param, "int foo(int a); foo(5 + 2); foo(foo(3));");

CHECK_EXPR(
    fn_call_with_params,
    "int foo(int a, string b, int? c, int? d); foo(3 + 4, \"Hello!\", nil, 3);"
)

CHECK_EXPR(array_sub, "[int, 5] array; int a = array[5 - 3] + 6");
CHECK_EXPR(array_sub_string, "string s = \"Hello, World!\"; s[7] == 87")
CHECK_EXPR(array_sub_assign, "[int, 5] array; array[5] = 5");

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
    RUN_TEST(var_assign);
    RUN_TEST(undef_var_assign);
    RUN_TEST(fn_decl);
    RUN_TEST(fn_decl_with_parameters);
    RUN_TEST(fn_decl_recursion);
    RUN_TEST(fn_call);
    RUN_TEST(fn_call_with_param);
    RUN_TEST(fn_call_with_params);
    RUN_TEST(array_sub);
    RUN_TEST(array_sub_string);
    RUN_TEST(array_sub_assign);
}
