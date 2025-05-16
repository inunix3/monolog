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
CHECK_EXPR(var_opt, "string? var = \"Hello\" + \", World\" + \"!\"")
CHECK_EXPR(var_opt_nil, "int? var = nil")
CHECK_EXPR(var_list, "[int] var;")
CHECK_EXPR(var_list_nested, "[[[[[int]]]]] var;")
CHECK_EXPR(var_list_with_initial_size, "[int, 5] var;")
CHECK_EXPR(var_usage, "int a = 5; int b = a + 5")
CHECK_EXPR(var_assign, "int a = 5; a = 5; a = a * 2");
CHECK_EXPR(undef_var_assign, "int a; a = 5; int b = a + 5");

CHECK_EXPR(
    scoping, "int global;"
             "{"
             "  int a = global;"
             "  {"
             "    int b = a;"
             "  }"
             "  int c = a;"
             "  {"
             "    {"
             "      {"
             "        int d = global + a + c;"
             "      }"
             "    }"
             "  }"
             "}"
)

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

CHECK_EXPR(fn_decl, "int foo() {}")

CHECK_EXPR(
    fn_decl_with_parameters, "int foo(int a, string b, int? c, string? d) {}"
)

CHECK_EXPR(fn_decl_recursion, "int foo(int a) { return foo(a + 1); }")

CHECK_EXPR(fn_call, "string foo() {} string b = foo();")
CHECK_EXPR(fn_call_with_param, "int foo(int a) {} foo(5 + 2); foo(foo(3));");

CHECK_EXPR(
    fn_call_with_params, "int foo(int a, string b, int? c, int? d) {} foo(3 + "
                         "4, \"Hello!\", nil, 3);"
)

CHECK_EXPR(
    fn_scope_global, "int a = 5;"
                     "void foo() {"
                     "  a = 115;"
                     "  int b = a + 5;"
                     "}"
)

CHECK_EXPR(
    fn_scope_nested, "int a = 5;"
                     "void foo() {"
                     "  int b = a;"
                     "  {"
                     "    int c = b;"
                     "    {"
                     "      int d = c;"
                     "    }"
                     "  }"
                     "}"
)

CHECK_EXPR(
    fn_short_scope, "int a = 5;"
                    "int foo(int x, int y)"
                    "  return a * x + a * y;"
)

CHECK_EXPR(list_sub, "[int] list; int a = list[5 - 3] + 6");
CHECK_EXPR(list_sub_assign, "[int] list; list[5] = 5");
CHECK_EXPR(string_sub, "string s = \"Hello, World!\"; s[7] == 87")
CHECK_EXPR(string_sub_assign, "string s = \"Hello, World!\"; s[7] = 87")

CHECK_EXPR(list_size, "[int] list; int a = #list; #list == 5")
CHECK_EXPR(string_len, "string str = \"Hi, World!\"; int a = #str; #str == 10")
CHECK_EXPR(string_literal_len, "#\"Hello, World!\" == 13")
CHECK_EXPR(option_nil, "int? a = nil; a == nil")
CHECK_EXPR(option_deref, "int? a = 5; *a == 5")
CHECK_EXPR(prefix_inc, "int a = 5; ++a; [int] b; ++b[0]")
CHECK_EXPR(prefix_dec, "int a = 5; --a; [int] b; --b[0]")
CHECK_EXPR(suffix_inc, "int a = 5; a++; [int] b; b[0]++")
CHECK_EXPR(suffix_dec, "int a = 5; a--; [int] b; b[0]--")
CHECK_EXPR(negation, "int a = 5; int b = !a")

CHECK_EXPR(
    list_push,
    "[int] a; a += 5; [string] b; b += \"aaa\"; [int?] c; c += nil; c += 50;"
);

CHECK_EXPR(list_pop, "[int] a; a -= 115");

CHECK_EXPR(
    if_stmt, "if (1)"
             "  if (0) {"
             "    115;"
             "  }"
)

CHECK_EXPR(
    if_else, "if (1)"
             "  if(1);"
             "else"
             "  if (0);"
)

CHECK_EXPR(
    while_stmt,
    "while (1)"
    "  while (0)"
    "    while (1) { break; continue; }"
)

CHECK_EXPR(
    for_stmt,
    "for (1; 2; 3)"
    "  for (1 + 1; 2 + 2; 3 * 3) {"
    "    break;"
    "    continue;"
    "  }"
)

CHECK_EXPR(
    for_decl_clause,
    "for (int i = 0; i < 5; ++i) {"
    "  println($i);"
    "}"
)

CHECK_EXPR(
    for_decl_clause_shadow,
    "int i = 115;"
    "for (int i = 0; i < 5; ++i) {"
    "  println($i);"
    "}"
    "println($i);"
)

SUITE(valid) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(binary_int_int);
    RUN_TEST(binary_string_string);
    RUN_TEST(unary_int);
    RUN_TEST(var_int);
    RUN_TEST(var_string);
    RUN_TEST(var_opt);
    RUN_TEST(var_opt_nil);
    RUN_TEST(var_list);
    RUN_TEST(var_list_nested);
    RUN_TEST(var_list_with_initial_size);
    RUN_TEST(var_usage);
    RUN_TEST(var_scope);
    RUN_TEST(var_assign);
    RUN_TEST(undef_var_assign);
    RUN_TEST(scoping);
    RUN_TEST(fn_decl);
    RUN_TEST(fn_decl_with_parameters);
    RUN_TEST(fn_decl_recursion);
    RUN_TEST(fn_call);
    RUN_TEST(fn_call_with_param);
    RUN_TEST(fn_call_with_params);
    RUN_TEST(fn_scope_global);
    RUN_TEST(fn_scope_nested);
    RUN_TEST(fn_short_scope);
    RUN_TEST(list_sub);
    RUN_TEST(string_sub);
    RUN_TEST(list_sub_assign);
    RUN_TEST(list_size);
    RUN_TEST(string_len);
    RUN_TEST(string_literal_len);
    RUN_TEST(option_nil);
    RUN_TEST(option_deref);
    RUN_TEST(prefix_inc);
    RUN_TEST(prefix_dec);
    RUN_TEST(suffix_inc);
    RUN_TEST(suffix_dec);
    RUN_TEST(negation);
    RUN_TEST(list_push);
    RUN_TEST(list_pop);
    RUN_TEST(if_stmt);
    RUN_TEST(if_else);
    RUN_TEST(while_stmt);
    RUN_TEST(for_stmt);
    RUN_TEST(for_decl_clause);
    RUN_TEST(for_decl_clause_shadow);
}
