#include "shared.h"

TEST empty_input(void) {
    Value v = eval("");

    ASSERT_EQ(TYPE_VOID, v.type->id);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST int_literal(void) {
    Value v = eval("123456789");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(123456789, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST unary_minus_int(void) {
    Value v = eval("-115");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(-115, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST unary_nested_int(void) {
    Value v = eval("-(-(+(-5)))");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(-5, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST unary_plus_int(void) {
    Value v = eval("+115");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(115, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_int_plus_int(void) {
    Value v = eval("115 + 94");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(209, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_int_minus_int(void) {
    Value v = eval("115 - 94");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(21, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_int_mul_int(void) {
    Value v = eval("115 * 94");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(10810, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_int_div_int(void) {
    Value v = eval("115 / 94");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_int_mod_int(void) {
    Value v = eval("115 % 94");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(21, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST binary_string_plus_string(void) {
    Value v = eval("\"Hello\" + \", \" + \"World\" + \"!\"");

    ASSERT_EQ(TYPE_STRING, v.type->id);
    ASSERT_STR_EQ("Hello, World!", v.s->data);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST precedence(void) {
    Value v = eval("1 + 2 * 3 - 4 / 10");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(7, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST left_associativity(void) {
    Value v = eval("115 + 94 + 3 - 1 - 10");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(201, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST mul_left_associativity(void) {
    Value v = eval("115 * 94 / 3 % 4 * 34");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(102, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST grouping(void) {
    Value v = eval("(115 + 94) / 34");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(6, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST nested_grouping(void) {
    Value v = eval("(((94 + 115) * 2) % 2 ) - 1");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(-1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST arithmetic_combination(void) {
    Value v = eval("(1 + 2) * (3 - 4) / (9 * -(+10000 % (34 - -99999)))");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST equality_true(void) {
    Value v = eval("2 + 2 == 4");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST equality_false(void) {
    Value v = eval("2 + 2 == 5");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST not_equal_true(void) {
    Value v = eval("3 + 5 * 3 - 4 / 5 != 4");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST not_equal_false(void) {
    Value v = eval("3 + 5 * 3 - 4 / 5 != 18");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST greater_true(void) {
    Value v = eval("5 > 3");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST greater_false(void) {
    Value v = eval("3 > 5");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST greater_equal_1_true(void) {
    Value v = eval("115 >= 94");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST greater_equal_2_true(void) {
    Value v = eval("94 >= 94");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST greater_equal_false(void) {
    Value v = eval("93 >= 94");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST less_true(void) {
    Value v = eval("-256 < 1024");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST less_false(void) {
    Value v = eval("-256 > 1024");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST less_equal_1_true(void) {
    Value v = eval("94 <= 115");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST less_equal_2_true(void) {
    Value v = eval("94 <= 94");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST less_equal_false(void) {
    Value v = eval("-115 <= -116");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST and_true(void) {
    Value v = eval("1 && 1");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST and_1_false(void) {
    Value v = eval("0 && 1");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST and_2_false(void) {
    Value v = eval("1 && 0");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST and_3_false(void) {
    Value v = eval("0 && 0");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST or_1_true(void) {
    Value v = eval("1 || 1");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST or_2_true(void) {
    Value v = eval("0 || 1");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST or_3_true(void) {
    Value v = eval("1 || 0");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST or_false(void) {
    Value v = eval("0 || 0");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST string_len(void) {
    Value v = eval("#\"Hello, World!\"");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(13, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST string_subscript(void) {
    Value first = eval("\"Hello, World!\"[0]");

    ASSERT_EQ(TYPE_INT, first.type->id);
    ASSERT_EQ((Int) 'H', first.i);

    Value last = eval("\"Hello, World!\"[12]");

    ASSERT_EQ(TYPE_INT, last.type->id);
    ASSERT_EQ((Int) '!', last.i);

    Value random = eval("\"Hello, World!\"[5]");

    ASSERT_EQ(TYPE_INT, random.type->id);
    ASSERT_EQ((Int) ',', random.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST int_to_string(void) {
    Value v = eval("$115");

    ASSERT_EQ(TYPE_STRING, v.type->id);
    ASSERT_STR_EQ("115", v.s->data);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST prefix_increment(void) {
    run("int a = 5;");
    Value v = eval("++a");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(6, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST suffix_increment(void) {
    run("int a = 5;");
    Value v = eval("a++");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(5, v.i);

    v = eval("a");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(6, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST prefix_decrement(void) {
    run("int a = 5;");
    Value v = eval("--a");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(4, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST suffix_decrement(void) {
    run("int a = 5;");
    Value v = eval("a--");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(5, v.i);

    v = eval("a");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(4, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST int_var(void) {
    run("int a = 115");
    Value v = eval("a + 5");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(120, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST string_var(void) {
    run("string s = \"Hello, World!\"");
    Value v = eval("s + s + \"String\"");

    ASSERT_EQ(TYPE_STRING, v.type->id);
    ASSERT_STR_EQ("Hello, World!Hello, World!String", v.s->data);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_nil(void) {
    run("int? o = nil");
    Value v = eval("o == nil");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_int(void) {
    run("int? o = 115");
    Value v = eval("o != nil");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_deref(void) {
    run("int? o = 115");
    Value v = eval("*o");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(115, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_reassign(void) {
    run(
        "int? o = 115;"
        "o = 94;"
    );

    Value v = eval("*o");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(94, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_deref_assign(void) {
    run(
        "int? o = 115;"
        "*o = 94;"
    );

    Value v = eval("*o");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(94, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_assign_nil(void) {
    run(
        "int? o = 115;"
        "o = nil;"
    );

    Value v = eval("o == nil");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_nested(void) {
    run(
        "int? a = 115;"
        "int?? b = a;"
        "int??? c = b;"
    );

    Value v = eval("***c");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(115, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_clone(void) {
    run(
        "string? a = \"Hello\";"
        "string? b = a;"
        "*b = *b + \", World!\";"
    );

    Value v1 = eval("*a");

    ASSERT_EQ(TYPE_STRING, v1.type->id);
    ASSERT_STR_EQ("Hello", v1.s->data);

    Value v2 = eval("*b");

    ASSERT_EQ(TYPE_STRING, v2.type->id);
    ASSERT_STR_EQ("Hello, World!", v2.s->data);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_nested_clone(void) {
    run(
        "string? a = \"Hello\";"
        "string?? b = a;"
        "string??? c = b;"
        "***c = ***c + \", World!\";"
    );

    Value v1 = eval("*a");

    ASSERT_EQ(TYPE_STRING, v1.type->id);
    ASSERT_STR_EQ("Hello", v1.s->data);

    Value v2 = eval("**b");

    ASSERT_EQ(TYPE_STRING, v2.type->id);
    ASSERT_STR_EQ("Hello", v2.s->data);

    Value v3 = eval("***c");

    ASSERT_EQ(TYPE_STRING, v3.type->id);
    ASSERT_STR_EQ("Hello, World!", v3.s->data);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_compare_not_equal(void) {
    run(
        "int? a = 5;"
        "int? b = 10;"
    );

    Value v = eval("a == b");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_compare_equal(void) {
    run(
        "int? a = 5;"
        "int? b = 5;"
    );

    Value v = eval("a == b");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_compare_nil_to_nonnil(void) {
    run(
        "int? a = 5;"
        "int? b = nil;"
    );

    Value v = eval("a == b");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(0, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST option_compare_nil_to_nil(void) {
    run(
        "int? a = nil;"
        "int? b = nil;"
    );

    Value v = eval("a == b");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST fn_call_without_args(void) {
    run(
        "void foo() { exit(115); }"
    );

    Value v = eval("foo()");

    ASSERT_EQ(TYPE_VOID, v.type->id);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(115, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(true, g_interp.halt);

    PASS();
}

TEST fn_return_int(void) {
    run(
        "int foo() { return 3; }"
    );

    Value v = eval("foo()");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(3, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST fn_return_string(void) {
    run(
        "string foo() { return \"Hello, World!\"; }"
    );

    Value v = eval("foo()");

    ASSERT_EQ(TYPE_STRING, v.type->id);
    ASSERT_STR_EQ("Hello, World!", v.s->data);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST fn_return_nested_opt(void) {
    run(
        "int?? foo() {"
        "  int? temp = 5;"
        "  return temp;"
        "}"
    );

    Value v = eval("**foo()");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(5, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST fn_return_opt(void) {
    run(
        "int? foo(int x) {"
        "  if (x == 1) {"
        "    return 115;"
        "  } else {"
        "    return nil;"
        "  }"
        "}"
    );

    Value v1 = eval("foo(1)");

    ASSERT_EQ(TYPE_OPTION, v1.type->id);
    ASSERT_NEQ(NULL, v1.opt.val);

    ASSERT_EQ(TYPE_INT, v1.opt.val->type->id);
    ASSERT_EQ(115, v1.opt.val->i);

    Value v2 = eval("foo(0)");

    ASSERT_EQ(TYPE_OPTION, v2.type->id);
    ASSERT_EQ(NULL, v2.opt.val);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST fn_with_param(void) {
    run(
        "int abs(int x) {"
        "  if (x < 0) {"
        "    return -x;"
        " } else {"
        "    return x;"
        " }"
        "}"
    );

    Value v1 = eval("abs(-115)");

    ASSERT_EQ(TYPE_INT, v1.type->id);
    ASSERT_EQ(115, v1.i);

    Value v2 = eval("abs(0)");

    ASSERT_EQ(TYPE_INT, v2.type->id);
    ASSERT_EQ(0, v2.i);

    Value v3 = eval("abs(94)");

    ASSERT_EQ(TYPE_INT, v3.type->id);
    ASSERT_EQ(94, v3.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST fn_with_multiple_params(void) {
    run(
        "string list(int i, string s, int? o_i, string? o_s) {"
        "  return $i + \", \" + s + \", \"+ $*o_i + \", \" + *o_s;"
        "}"
        ""
        "int? o_i = 115;"
    );

    Value v = eval("list(94, \"string\", o_i, \"option\")");

    ASSERT_EQ(TYPE_STRING, v.type->id);
    ASSERT_STR_EQ("94, string, 115, option", v.s->data);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST fn_isolation(void) {
    run(
        "int x = 1;"
        "int foo() {"
        "  int x = 2;"
        "  return x;"
        "}"
    );

    Value v1 = eval("foo()");

    ASSERT_EQ(TYPE_INT, v1.type->id);
    ASSERT_EQ(2, v1.i);

    Value v2 = eval("x");

    ASSERT_EQ(TYPE_INT, v2.type->id);
    ASSERT_EQ(1, v2.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST fn_global_scope(void) {
    run(
        "int global = 115;"
        ""
        "int foo(int n) {"
        "  global = 94;"
        "  int x = 5;"
        "  return global + n + x;"
        "}"
    );

    Value v1 = eval("foo(200);");

    ASSERT_EQ(TYPE_INT, v1.type->id);
    ASSERT_EQ(299, v1.i);

    Value v2 = eval("global");

    ASSERT_EQ(TYPE_INT, v2.type->id);
    ASSERT_EQ(94, v2.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST fn_early_return(void) {
    run(
        "int global = 115;"
        ""
        "void foo() {"
        "  return;"
        "  global = 94;"
        "}"
    );

    Value v1 = eval("foo();");

    ASSERT_EQ(TYPE_VOID, v1.type->id);

    Value v2 = eval("global");

    ASSERT_EQ(TYPE_INT, v2.type->id);
    ASSERT_EQ(115, v2.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST fn_recursion_factorial(void) {
    run(
        "int factorial(int n) {"
        "  if (n == 0) return 1;"
        "  return n * factorial(n - 1);"
        "}"
    );

    Value v = eval("factorial(15);");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(1307674368000, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST fn_recursion_fibonacci(void) {
    run(
        "int fib(int n) {"
        "  if (n <= 1) return n;"
        "  return fib(n - 1) + fib(n - 2);"
        "}"
    );

    Value v = eval("fib(20)");

    ASSERT_EQ(TYPE_INT, v.type->id);
    ASSERT_EQ(6765, v.i);

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

TEST array(void) {
    run("[int, 5] a;");
    run("[int, 6] b = a;");

    ASSERT_EQ(&g_ast, g_interp.ast);
    ASSERT_EQ(0, g_interp.exit_code);
    ASSERT_EQ(false, g_interp.had_error);
    ASSERT_EQ(false, g_interp.halt);

    PASS();
}

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

SUITE(valid) {
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
    RUN_TEST(string_subscript);
    RUN_TEST(prefix_increment);
    RUN_TEST(prefix_decrement);
    RUN_TEST(suffix_increment);
    RUN_TEST(suffix_decrement);
    RUN_TEST(int_var);
    RUN_TEST(string_var);
    RUN_TEST(option_nil);
    RUN_TEST(option_int);
    RUN_TEST(option_deref);
    RUN_TEST(option_reassign);
    RUN_TEST(option_deref_assign);
    RUN_TEST(option_assign_nil);
    RUN_TEST(option_nested);
    RUN_TEST(option_clone);
    RUN_TEST(option_nested_clone);
    RUN_TEST(option_compare_not_equal);
    RUN_TEST(option_compare_equal);
    RUN_TEST(option_compare_nil_to_nonnil);
    RUN_TEST(option_compare_nil_to_nil);
    RUN_TEST(fn_call_without_args);
    RUN_TEST(fn_return_int);
    RUN_TEST(fn_return_string);
    RUN_TEST(fn_return_opt);
    RUN_TEST(fn_return_nested_opt);
    RUN_TEST(fn_with_param);
    RUN_TEST(fn_with_multiple_params);
    RUN_TEST(fn_isolation);
    RUN_TEST(fn_global_scope);
    RUN_TEST(fn_early_return);
    RUN_TEST(fn_recursion_factorial);
    RUN_TEST(fn_recursion_fibonacci);
    RUN_TEST(array);
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
