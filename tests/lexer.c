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

#define NTH_TOKEN(_idx) (((Token *) g_tokens.data)[_idx])

#define ASSERT_EQUAL_TOKEN(_tok, _expected)  \
    TEST_ASSERT_EQUAL_INT(_tok.kind, _expected.kind); \
    TEST_ASSERT_EQUAL_PTR(_tok.src, _expected.src); \
    TEST_ASSERT_EQUAL_size_t(_tok.len, _expected.len); \
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
    const char *input = "123 456\n 789\t111\r\n\t34";
    lexer_lex(input, strlen(input), &g_tokens);

    TEST_ASSERT_EQUAL_size_t(6, g_tokens.len);

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

    TEST_ASSERT_EQUAL_INT(TOKEN_EOF, NTH_TOKEN(5).kind);
    TEST_ASSERT_EQUAL_PTR(input + strlen(input), NTH_TOKEN(5).src);
    TEST_ASSERT_EQUAL_size_t(0, NTH_TOKEN(5).len);
    TEST_ASSERT(NTH_TOKEN(5).valid);
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

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_empty_string_returns_eof_token);
    RUN_TEST(test_whitespaces_return_eof_token);
    RUN_TEST(test_integer_returns_integer_token);
    RUN_TEST(test_integers_separated_by_whitespace);
    RUN_TEST(test_identifier_returns_identifier_token);
    RUN_TEST(test_identifiers_separated_by_whitespace);

    return 0;
}
