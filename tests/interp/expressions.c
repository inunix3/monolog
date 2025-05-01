#include "shared.h"

TEST empty_input(void) {
    Value v = eval("");

    ASSERT_EQ(TYPE_VOID, v.type);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST int_literal(void) {
    Value v = eval("123456789");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(123456789, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST unary_minus_int(void) {
    Value v = eval("-115");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(-115, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST unary_nested_int(void) {
    Value v = eval("-(-(+(-5)))");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(-5, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST unary_plus_int(void) {
    Value v = eval("+115");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(115, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_int_plus_int(void) {
    Value v = eval("115 + 94");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(209, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_int_minus_int(void) {
    Value v = eval("115 - 94");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(21, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_int_mul_int(void) {
    Value v = eval("115 * 94");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(10810, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_int_div_int(void) {
    Value v = eval("115 / 94");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_int_mod_int(void) {
    Value v = eval("115 % 94");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(21, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_string_plus_string(void) {
    Value v = eval("\"Hello\" + \", \" + \"World\" + \"!\"");

    ASSERT_EQ(TYPE_STRING, v.type);
    ASSERT_STR_EQ("Hello, World!", v.s.data);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST precedence(void) {
    Value v = eval("1 + 2 * 3 - 4 / 10");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(7, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST left_associativity(void) {
    Value v = eval("115 + 94 + 3 - 1 - 10");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(201, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST mul_left_associativity(void) {
    Value v = eval("115 * 94 / 3 % 4 * 34");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(102, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST grouping(void) {
    Value v = eval("(115 + 94) / 34");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(6, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST nested_grouping(void) {
    Value v = eval("(((94 + 115) * 2) % 2 ) - 1");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(-1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST arithmetic_combination(void) {
    Value v = eval("(1 + 2) * (3 - 4) / (9 * -(+10000 % (34 - -99999)))");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST equality_true(void) {
    Value v = eval("2 + 2 == 4");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST equality_false(void) {
    Value v = eval("2 + 2 == 5");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST not_equal_true(void) {
    Value v = eval("3 + 5 * 3 - 4 / 5 != 4");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST not_equal_false(void) {
    Value v = eval("3 + 5 * 3 - 4 / 5 != 18");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST greater_true(void) {
    Value v = eval("5 > 3");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST greater_false(void) {
    Value v = eval("3 > 5");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST greater_equal_1_true(void) {
    Value v = eval("115 >= 94");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST greater_equal_2_true(void) {
    Value v = eval("94 >= 94");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST greater_equal_false(void) {
    Value v = eval("93 >= 94");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST less_true(void) {
    Value v = eval("-256 < 1024");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST less_false(void) {
    Value v = eval("-256 > 1024");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST less_equal_1_true(void) {
    Value v = eval("94 <= 115");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST less_equal_2_true(void) {
    Value v = eval("94 <= 94");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST less_equal_false(void) {
    Value v = eval("-115 <= -116");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST and_true(void) {
    Value v = eval("1 && 1");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST and_1_false(void) {
    Value v = eval("0 && 1");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST and_2_false(void) {
    Value v = eval("1 && 0");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST and_3_false(void) {
    Value v = eval("0 && 0");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST or_1_true(void) {
    Value v = eval("1 || 1");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST or_2_true(void) {
    Value v = eval("0 || 1");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST or_3_true(void) {
    Value v = eval("1 || 0");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST or_false(void) {
    Value v = eval("0 || 0");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST string_len(void) {
    Value v = eval("#\"Hello, World!\"");

    ASSERT_EQ(TYPE_INT, v.type);
    ASSERT_EQ(13, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST int_to_string(void) {
    Value v = eval("$115");

    ASSERT_EQ(TYPE_STRING, v.type);
    ASSERT_STR_EQ("115", v.s.data);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

SUITE(expressions) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(empty_input);
    RUN_TEST(int_literal);
    RUN_TEST(unary_minus_int);
    RUN_TEST(unary_plus_int);
    RUN_TEST(binary_int_plus_int);
    RUN_TEST(binary_int_minus_int);
    RUN_TEST(binary_int_mul_int);
    RUN_TEST(binary_int_div_int);
    RUN_TEST(binary_int_mod_int);
    RUN_TEST(binary_string_plus_string);
    RUN_TEST(precedence);
    RUN_TEST(left_associativity);
    RUN_TEST(mul_left_associativity);
    RUN_TEST(grouping);
    RUN_TEST(nested_grouping);
    RUN_TEST(arithmetic_combination);
    RUN_TEST(equality_true);
    RUN_TEST(equality_false);
    RUN_TEST(not_equal_true);
    RUN_TEST(not_equal_false);
    RUN_TEST(greater_true);
    RUN_TEST(greater_false);
    RUN_TEST(greater_equal_1_true);
    RUN_TEST(greater_equal_2_true);
    RUN_TEST(greater_equal_false);
    RUN_TEST(less_true);
    RUN_TEST(less_false);
    RUN_TEST(less_equal_1_true);
    RUN_TEST(less_equal_2_true);
    RUN_TEST(less_equal_false);
    RUN_TEST(and_true);
    RUN_TEST(and_1_false);
    RUN_TEST(and_2_false);
    RUN_TEST(and_3_false);
    RUN_TEST(or_1_true);
    RUN_TEST(or_2_true);
    RUN_TEST(or_3_true);
    RUN_TEST(or_false);
    RUN_TEST(string_len);
    RUN_TEST(int_to_string);
}
