/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/lexer.h>

#include <unity.h>

#include <stdbool.h>
#include <string.h>

static Vector g_tokens;

void setUp(void) { vec_init(&g_tokens, sizeof(Token)); }

void tearDown(void) { vec_deinit(&g_tokens); }

#define NTH_TOKEN(_idx) (((Token *)g_tokens.data)[_idx])

#define ASSERT_EQUAL_TOKEN(_tok, _expected)                                    \
    TEST_ASSERT_EQUAL_INT(_tok.kind, _expected.kind);                          \
    TEST_ASSERT_EQUAL_PTR(_tok.src, _expected.src);                            \
    TEST_ASSERT_EQUAL_size_t(_tok.len, _expected.len);                         \
    TEST_ASSERT(_tok.valid == _expected.valid)

void test_empty_string_returns_eof_token(void) {
    lexer_lex("", 0, &g_tokens);

    TEST_ASSERT_EQUAL_size_t(1, g_tokens.len);
    TEST_ASSERT_EQUAL_INT(TOKEN_EOF, NTH_TOKEN(0).kind);
    TEST_ASSERT_EQUAL_PTR("", NTH_TOKEN(0).src);
    TEST_ASSERT_EQUAL_size_t(0, NTH_TOKEN(0).len);
    TEST_ASSERT(NTH_TOKEN(0).valid);
}

void test_whitespaces_return_eof_token(void) {
    const char *input = "         \t\n\r    \t    \n    \r     ";
    lexer_lex(input, strlen(input), &g_tokens);

    TEST_ASSERT_EQUAL_size_t(1, g_tokens.len);

    TEST_ASSERT_EQUAL_INT(TOKEN_EOF, NTH_TOKEN(0).kind);
    TEST_ASSERT_EQUAL_PTR(input + strlen(input), NTH_TOKEN(0).src);
    TEST_ASSERT_EQUAL_size_t(0, NTH_TOKEN(0).len);
    TEST_ASSERT(NTH_TOKEN(0).valid);
}

void test_invalid_characters(void) {
    const char *input = "`~!@#$%^&";
    lexer_lex(input, strlen(input), &g_tokens);

    TEST_ASSERT_EQUAL_size_t(2, g_tokens.len);

    TEST_ASSERT_EQUAL_INT(TOKEN_UNKNOWN, NTH_TOKEN(0).kind);
    TEST_ASSERT_EQUAL_PTR(input, NTH_TOKEN(0).src);
    TEST_ASSERT_EQUAL_size_t(strlen(input), NTH_TOKEN(0).len);
    TEST_ASSERT(!NTH_TOKEN(0).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_EOF, NTH_TOKEN(1).kind);
    TEST_ASSERT_EQUAL_PTR(input + strlen(input), NTH_TOKEN(1).src);
    TEST_ASSERT_EQUAL_size_t(0, NTH_TOKEN(1).len);
    TEST_ASSERT(NTH_TOKEN(1).valid);
}

void test_integer_returns_integer_token(void) {
    const char *input = "123456789";
    lexer_lex(input, strlen(input), &g_tokens);

    TEST_ASSERT_EQUAL_size_t(2, g_tokens.len);

    TEST_ASSERT_EQUAL_INT(TOKEN_INTEGER, NTH_TOKEN(0).kind);
    TEST_ASSERT_EQUAL_PTR(input, NTH_TOKEN(0).src);
    TEST_ASSERT_EQUAL_size_t(9, NTH_TOKEN(0).len);
    TEST_ASSERT(NTH_TOKEN(0).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_EOF, NTH_TOKEN(1).kind);
    TEST_ASSERT_EQUAL_PTR(input + strlen(input), NTH_TOKEN(1).src);
    TEST_ASSERT_EQUAL_size_t(0, NTH_TOKEN(1).len);
    TEST_ASSERT(NTH_TOKEN(1).valid);
}

void test_integers_separated_by_whitespace(void) {
    const char *input = "123 456\n 789\t111\r\n\t34 @#$$$";
    lexer_lex(input, strlen(input), &g_tokens);

    TEST_ASSERT_EQUAL_size_t(7, g_tokens.len);

    TEST_ASSERT_EQUAL_INT(TOKEN_INTEGER, NTH_TOKEN(0).kind);
    TEST_ASSERT_EQUAL_PTR(input, NTH_TOKEN(0).src);
    TEST_ASSERT_EQUAL_size_t(3, NTH_TOKEN(0).len);
    TEST_ASSERT(NTH_TOKEN(0).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_INTEGER, NTH_TOKEN(1).kind);
    TEST_ASSERT_EQUAL_PTR(input + 4, NTH_TOKEN(1).src);
    TEST_ASSERT_EQUAL_size_t(3, NTH_TOKEN(1).len);
    TEST_ASSERT(NTH_TOKEN(1).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_INTEGER, NTH_TOKEN(2).kind);
    TEST_ASSERT_EQUAL_PTR(input + 9, NTH_TOKEN(2).src);
    TEST_ASSERT_EQUAL_size_t(3, NTH_TOKEN(2).len);
    TEST_ASSERT(NTH_TOKEN(2).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_INTEGER, NTH_TOKEN(3).kind);
    TEST_ASSERT_EQUAL_PTR(input + 13, NTH_TOKEN(3).src);
    TEST_ASSERT_EQUAL_size_t(3, NTH_TOKEN(3).len);
    TEST_ASSERT(NTH_TOKEN(3).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_INTEGER, NTH_TOKEN(4).kind);
    TEST_ASSERT_EQUAL_PTR(input + 19, NTH_TOKEN(4).src);
    TEST_ASSERT_EQUAL_size_t(2, NTH_TOKEN(4).len);
    TEST_ASSERT(NTH_TOKEN(4).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_UNKNOWN, NTH_TOKEN(5).kind);
    TEST_ASSERT_EQUAL_PTR(input + 22, NTH_TOKEN(5).src);
    TEST_ASSERT_EQUAL_size_t(5, NTH_TOKEN(5).len);
    TEST_ASSERT(!NTH_TOKEN(5).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_EOF, NTH_TOKEN(6).kind);
    TEST_ASSERT_EQUAL_PTR(input + strlen(input), NTH_TOKEN(6).src);
    TEST_ASSERT_EQUAL_size_t(0, NTH_TOKEN(6).len);
    TEST_ASSERT(NTH_TOKEN(6).valid);
}

void test_identifier_returns_identifier_token(void) {
    const char *input = "abcdefAFGHJK__34343sdf231";
    lexer_lex(input, strlen(input), &g_tokens);

    TEST_ASSERT_EQUAL_size_t(2, g_tokens.len);

    TEST_ASSERT_EQUAL_INT(TOKEN_IDENTIFIER, NTH_TOKEN(0).kind);
    TEST_ASSERT_EQUAL_PTR(input, NTH_TOKEN(0).src);
    TEST_ASSERT_EQUAL_size_t(strlen(input), NTH_TOKEN(0).len);
    TEST_ASSERT(NTH_TOKEN(0).valid);
}

void test_identifiers_separated_by_whitespace(void) {
    const char *input = "_asdasd123 hello\n  WORLD\t__int128\r\n\t_3__";
    lexer_lex(input, strlen(input), &g_tokens);

    TEST_ASSERT_EQUAL_size_t(6, g_tokens.len);

    TEST_ASSERT_EQUAL_INT(TOKEN_IDENTIFIER, NTH_TOKEN(0).kind);
    TEST_ASSERT_EQUAL_PTR(input, NTH_TOKEN(0).src);
    TEST_ASSERT_EQUAL_size_t(10, NTH_TOKEN(0).len);
    TEST_ASSERT(NTH_TOKEN(0).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_IDENTIFIER, NTH_TOKEN(1).kind);
    TEST_ASSERT_EQUAL_PTR(input + 11, NTH_TOKEN(1).src);
    TEST_ASSERT_EQUAL_size_t(5, NTH_TOKEN(1).len);
    TEST_ASSERT(NTH_TOKEN(1).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_IDENTIFIER, NTH_TOKEN(2).kind);
    TEST_ASSERT_EQUAL_PTR(input + 19, NTH_TOKEN(2).src);
    TEST_ASSERT_EQUAL_size_t(5, NTH_TOKEN(2).len);
    TEST_ASSERT(NTH_TOKEN(2).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_IDENTIFIER, NTH_TOKEN(3).kind);
    TEST_ASSERT_EQUAL_PTR(input + 25, NTH_TOKEN(3).src);
    TEST_ASSERT_EQUAL_size_t(8, NTH_TOKEN(3).len);
    TEST_ASSERT(NTH_TOKEN(3).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_IDENTIFIER, NTH_TOKEN(4).kind);
    TEST_ASSERT_EQUAL_PTR(input + 36, NTH_TOKEN(4).src);
    TEST_ASSERT_EQUAL_size_t(4, NTH_TOKEN(4).len);
    TEST_ASSERT(NTH_TOKEN(4).valid);

    TEST_ASSERT_EQUAL_INT(TOKEN_EOF, NTH_TOKEN(5).kind);
    TEST_ASSERT_EQUAL_PTR(input + strlen(input), NTH_TOKEN(5).src);
    TEST_ASSERT_EQUAL_size_t(0, NTH_TOKEN(5).len);
    TEST_ASSERT(NTH_TOKEN(5).valid);
}

void test_operator_returns_operator_token(void) {
    const char *input = ",;+-/%()[]{}\"'";
    lexer_lex(input, strlen(input), &g_tokens);

    TEST_ASSERT_EQUAL_size_t(strlen(input) + 1, g_tokens.len);

    for (int i = 0; i < strlen(input); ++i) {
        TEST_ASSERT_EQUAL_INT(TOKEN_OPERATOR, NTH_TOKEN(i).kind);
        TEST_ASSERT_EQUAL_PTR(input + i, NTH_TOKEN(i).src);
        TEST_ASSERT_EQUAL_size_t(1, NTH_TOKEN(i).len);
        TEST_ASSERT(NTH_TOKEN(i).valid);
    }

    const size_t eof_idx = strlen(input);

    TEST_ASSERT_EQUAL_INT(TOKEN_EOF, NTH_TOKEN(eof_idx).kind);
    TEST_ASSERT_EQUAL_PTR(input + strlen(input), NTH_TOKEN(eof_idx).src);
    TEST_ASSERT_EQUAL_size_t(0, NTH_TOKEN(eof_idx).len);
    TEST_ASSERT(NTH_TOKEN(eof_idx).valid);
}

void test_identifiers_and_operators(void) {
    const char *input = "{printf(fmt),puts(strings[idx])*__int128+-/%t_3__;}";
    lexer_lex(input, strlen(input), &g_tokens);

    TEST_ASSERT_EQUAL_size_t(23, g_tokens.len);

    Token expected[] = {
        {TOKEN_OPERATOR, input, 1, true},        /* { */
        {TOKEN_IDENTIFIER, input + 1, 6, true},  /* printf */
        {TOKEN_OPERATOR, input + 7, 1, true},    /* ( */
        {TOKEN_IDENTIFIER, input + 8, 3, true},  /* fmt */
        {TOKEN_OPERATOR, input + 11, 1, true},   /* ) */
        {TOKEN_OPERATOR, input + 12, 1, true},   /* , */
        {TOKEN_IDENTIFIER, input + 13, 4, true}, /* puts */
        {TOKEN_OPERATOR, input + 17, 1, true},   /* ( */
        {TOKEN_IDENTIFIER, input + 18, 7, true}, /* strings */
        {TOKEN_OPERATOR, input + 25, 1, true},   /* [ */
        {TOKEN_IDENTIFIER, input + 26, 3, true}, /* idx */
        {TOKEN_OPERATOR, input + 29, 1, true},   /* ] */
        {TOKEN_OPERATOR, input + 30, 1, true},   /* ) */
        {TOKEN_OPERATOR, input + 31, 1, true},   /* * */
        {TOKEN_IDENTIFIER, input + 32, 8, true}, /* __int128 */
        {TOKEN_OPERATOR, input + 40, 1, true},   /* + */
        {TOKEN_OPERATOR, input + 41, 1, true},   /* - */
        {TOKEN_OPERATOR, input + 42, 1, true},   /* / */
        {TOKEN_OPERATOR, input + 43, 1, true},   /* % */
        {TOKEN_IDENTIFIER, input + 44, 5, true}, /* t_3__ */
        {TOKEN_OPERATOR, input + 49, 1, true},   /* ; */
        {TOKEN_OPERATOR, input + 50, 1, true},   /* } */
    };

    for (int i = 0; i < 22; ++i) {
        TEST_ASSERT_EQUAL_INT(expected[i].kind, NTH_TOKEN(i).kind);
        TEST_ASSERT_EQUAL_PTR(expected[i].src, NTH_TOKEN(i).src);
        TEST_ASSERT_EQUAL_size_t(expected[i].len, NTH_TOKEN(i).len);
        TEST_ASSERT(expected[i].valid == NTH_TOKEN(i).valid);
    }

    TEST_ASSERT_EQUAL_INT(TOKEN_EOF, NTH_TOKEN(22).kind);
    TEST_ASSERT_EQUAL_PTR(input + strlen(input), NTH_TOKEN(22).src);
    TEST_ASSERT_EQUAL_size_t(0, NTH_TOKEN(22).len);
    TEST_ASSERT(NTH_TOKEN(22).valid);
}

void test_identifiers_and_operators_separated_by_whitespaces(void) {
    const char *input = "{\tprintf   ( fmt )   \t,\n\rputs  ( strings   "
                        "[\nidx\r]  ) * __int128   +"
                        "    -  /  %     t_3__ ;\t}\n";
    lexer_lex(input, strlen(input), &g_tokens);

    TEST_ASSERT_EQUAL_size_t(23, g_tokens.len);

    Token expected[] = {
        {TOKEN_OPERATOR, input, 1, true},        /* { */
        {TOKEN_IDENTIFIER, input + 2, 6, true},  /* printf */
        {TOKEN_OPERATOR, input + 11, 1, true},   /* ( */
        {TOKEN_IDENTIFIER, input + 13, 3, true}, /* fmt */
        {TOKEN_OPERATOR, input + 17, 1, true},   /* ) */
        {TOKEN_OPERATOR, input + 22, 1, true},   /* , */
        {TOKEN_IDENTIFIER, input + 25, 4, true}, /* puts */
        {TOKEN_OPERATOR, input + 31, 1, true},   /* ( */
        {TOKEN_IDENTIFIER, input + 33, 7, true}, /* strings */
        {TOKEN_OPERATOR, input + 43, 1, true},   /* [ */
        {TOKEN_IDENTIFIER, input + 45, 3, true}, /* idx */
        {TOKEN_OPERATOR, input + 49, 1, true},   /* ] */
        {TOKEN_OPERATOR, input + 52, 1, true},   /* ) */
        {TOKEN_OPERATOR, input + 54, 1, true},   /* * */
        {TOKEN_IDENTIFIER, input + 56, 8, true}, /* __int128 */
        {TOKEN_OPERATOR, input + 67, 1, true},   /* + */
        {TOKEN_OPERATOR, input + 72, 1, true},   /* - */
        {TOKEN_OPERATOR, input + 75, 1, true},   /* / */
        {TOKEN_OPERATOR, input + 78, 1, true},   /* % */
        {TOKEN_IDENTIFIER, input + 84, 5, true}, /* t_3__ */
        {TOKEN_OPERATOR, input + 90, 1, true},   /* ; */
        {TOKEN_OPERATOR, input + 92, 1, true},   /* } */
    };

    for (int i = 0; i < 22; ++i) {
        TEST_ASSERT_EQUAL_INT(expected[i].kind, NTH_TOKEN(i).kind);
        TEST_ASSERT_EQUAL_PTR(expected[i].src, NTH_TOKEN(i).src);
        TEST_ASSERT_EQUAL_size_t(expected[i].len, NTH_TOKEN(i).len);
        TEST_ASSERT(expected[i].valid == NTH_TOKEN(i).valid);
    }

    TEST_ASSERT_EQUAL_INT(TOKEN_EOF, NTH_TOKEN(22).kind);
    TEST_ASSERT_EQUAL_PTR(input + strlen(input), NTH_TOKEN(22).src);
    TEST_ASSERT_EQUAL_size_t(0, NTH_TOKEN(22).len);
    TEST_ASSERT(NTH_TOKEN(22).valid);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_empty_string_returns_eof_token);
    RUN_TEST(test_whitespaces_return_eof_token);
    RUN_TEST(test_invalid_characters);
    RUN_TEST(test_integer_returns_integer_token);
    RUN_TEST(test_integers_separated_by_whitespace);
    RUN_TEST(test_identifier_returns_identifier_token);
    RUN_TEST(test_identifiers_separated_by_whitespace);
    RUN_TEST(test_operator_returns_operator_token);
    RUN_TEST(test_identifiers_and_operators);
    RUN_TEST(test_identifiers_and_operators_separated_by_whitespaces);

    return 0;
}
