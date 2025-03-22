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

void test_empty_string_returns_eof_token() {
    lexer_lex("", 0, &g_tokens);
    Token expected = {TOKEN_EOF, "", 0, true };

    ASSERT_EQUAL_TOKEN(NTH_TOKEN(0), expected);
}

void test_whitespaces_return_eof_token() {
    const char *input = "         \t\n\r    \t    \n    \r     ";
    lexer_lex(input, strlen(input), &g_tokens);

    Token expected = {TOKEN_EOF, "", 0, true };

    ASSERT_EQUAL_TOKEN(NTH_TOKEN(0), expected);
}

void test_integer_returns_integer_token() {
    const char *input = "123456789";
    lexer_lex(input, strlen(input), &g_tokens);

    Token expected1 = { TOKEN_INTEGER, input, strlen(input), true };
    Token expected2 = { TOKEN_EOF, "", 0, true };

    printf("%s\n", NTH_TOKEN(0).src);
    ASSERT_EQUAL_TOKEN(NTH_TOKEN(0), expected1);
    ASSERT_EQUAL_TOKEN(NTH_TOKEN(1), expected2);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_empty_string_returns_eof_token);
    RUN_TEST(test_whitespaces_return_eof_token);
    RUN_TEST(test_integer_returns_integer_token);

    return 0;
}
