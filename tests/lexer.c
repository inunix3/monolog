/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/lexer.h>

#include <greatest.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static Vector g_tokens;

static int token_equal_cb(const void *exp, const void *got, void *udata) {
    (void)udata;

    const Token *exp_tok = exp;
    const Token *got_tok = got;

    return exp_tok->kind == got_tok->kind && exp_tok->src == got_tok->src &&
           exp_tok->len == got_tok->len && exp_tok->valid == got_tok->valid;
}

static int token_printf_cb(const void *data, void *udata) {
    (void)udata;

    const Token *tok = data;

    return printf(
        "{ kind = %d; src = %p; len = %zu; valid = %s; }", tok->kind, tok->src,
        tok->len, tok->valid ? "true" : "false"
    );
}

static greatest_type_info g_token_type_info = {token_equal_cb, token_printf_cb};

void set_up(void *udata) {
    (void)udata;

    vec_init(&g_tokens, sizeof(Token));
}

void tear_down(void *udata) {
    (void)udata;

    vec_deinit(&g_tokens);
}

#define NTH_TOKEN(_idx) (((Token *)g_tokens.data)[_idx])

TEST empty_input_returns_eof_token(void) {
    const char *input = "";
    lexer_lex(input, 0, &g_tokens);

    ASSERT_EQ(1, g_tokens.len);

    Token expected = {TOKEN_EOF, input, 0, true};
    ASSERT_EQUAL_T(&expected, &NTH_TOKEN(0), &g_token_type_info, NULL);

    PASS();
}

TEST whitespaces_return_eof_token(void) {
    const char *input = "         \t\n\r    \t    \n    \r     ";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(1, g_tokens.len);

    Token expected = {TOKEN_EOF, input + strlen(input), 0, true};
    ASSERT_EQUAL_T(&expected, &NTH_TOKEN(0), &g_token_type_info, NULL);

    PASS();
}

TEST invalid_token(void) {
    const char *input = "`~!@^adsfd#$!3432";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(2, g_tokens.len);

    Token expected[] = {
        {TOKEN_UNKNOWN, input, strlen(input), false},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST integer(void) {
    const char *input = "123456789";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(2, g_tokens.len);

    Token expected[] = {
        {TOKEN_INTEGER, input, 9, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST integers_separated_with_whitespace(void) {
    const char *input = "123 456\n 789\t111\r\n\t34";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(6, g_tokens.len);

    Token expected[] = {
        {TOKEN_INTEGER, input, 3, true},
        {TOKEN_INTEGER, input + 4, 3, true},
        {TOKEN_INTEGER, input + 9, 3, true},
        {TOKEN_INTEGER, input + 13, 3, true},
        {TOKEN_INTEGER, input + 19, 2, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST identifier(void) {
    const char *input = "abcdefAFGHJK__34343sdf231";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(2, g_tokens.len);

    Token expected[] = {
        {TOKEN_IDENTIFIER, input, strlen(input), true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST identifiers_separated_with_whitespace(void) {
    const char *input = "_asdasd123 hello\n  WORLD\t__int128\r\n\t_3__";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(6, g_tokens.len);

    Token expected[] = {
        {TOKEN_IDENTIFIER, input, 10, true},
        {TOKEN_IDENTIFIER, input + 11, 5, true},
        {TOKEN_IDENTIFIER, input + 19, 5, true},
        {TOKEN_IDENTIFIER, input + 25, 8, true},
        {TOKEN_IDENTIFIER, input + 36, 4, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST single_operators(void) {
    const char *input = "=,;+-/%()[]{}$#?!&|<>";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(strlen(input) + 1, g_tokens.len);

    Token expected[] = {
        {TOKEN_OP_ASSIGN, input, 1, true},
        {TOKEN_OP_COMMA, input + 1, 1, true},
        {TOKEN_OP_SEMICOLON, input + 2, 1, true},
        {TOKEN_OP_PLUS, input + 3, 1, true},
        {TOKEN_OP_MINUS, input + 4, 1, true},
        {TOKEN_OP_DIV, input + 5, 1, true},
        {TOKEN_OP_MOD, input + 6, 1, true},
        {TOKEN_OP_LPAREN, input + 7, 1, true},
        {TOKEN_OP_RPAREN, input + 8, 1, true},
        {TOKEN_OP_LBRACKET, input + 9, 1, true},
        {TOKEN_OP_RBRACKET, input + 10, 1, true},
        {TOKEN_OP_LBRACE, input + 11, 1, true},
        {TOKEN_OP_RBRACE, input + 12, 1, true},
        {TOKEN_OP_DOLAR, input + 13, 1, true},
        {TOKEN_OP_HASHTAG, input + 14, 1, true},
        {TOKEN_OP_QUEST, input + 15, 1, true},
        {TOKEN_OP_EXCL, input + 16, 1, true},
        {TOKEN_OP_AMP, input + 17, 1, true},
        {TOKEN_OP_PIPE, input + 18, 1, true},
        {TOKEN_OP_LESS, input + 19, 1, true},
        {TOKEN_OP_GREATER, input + 20, 1, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST double_operators(void) {
    const char *input = "!= == <= >= ++ -- && ||";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(9, g_tokens.len);

    Token expected[] = {
        {TOKEN_OP_NOT_EQUAL, input, 2, true},
        {TOKEN_OP_EQUAL, input + 3, 2, true},
        {TOKEN_OP_LESS_EQUAL, input + 6, 2, true},
        {TOKEN_OP_GREATER_EQUAL, input + 9, 2, true},
        {TOKEN_OP_INC, input + 12, 2, true},
        {TOKEN_OP_DEC, input + 15, 2, true},
        {TOKEN_OP_AND, input + 18, 2, true},
        {TOKEN_OP_OR, input + 21, 2, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST double_operators_cannot_be_splitted(void) {
    const char *input = "! = =\n= <\t= >    = + + - - & & |  |";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(17, g_tokens.len);

    Token expected[] = {
        {TOKEN_OP_EXCL, input, 1, true},
        {TOKEN_OP_ASSIGN, input + 2, 1, true},
        {TOKEN_OP_ASSIGN, input + 4, 1, true},
        {TOKEN_OP_ASSIGN, input + 6, 1, true},
        {TOKEN_OP_LESS, input + 8, 1, true},
        {TOKEN_OP_ASSIGN, input + 10, 1, true},
        {TOKEN_OP_GREATER, input + 12, 1, true},
        {TOKEN_OP_ASSIGN, input + 17, 1, true},
        {TOKEN_OP_PLUS, input + 19, 1, true},
        {TOKEN_OP_PLUS, input + 21, 1, true},
        {TOKEN_OP_MINUS, input + 23, 1, true},
        {TOKEN_OP_MINUS, input + 25, 1, true},
        {TOKEN_OP_AMP, input + 27, 1, true},
        {TOKEN_OP_AMP, input + 29, 1, true},
        {TOKEN_OP_PIPE, input + 31, 1, true},
        {TOKEN_OP_PIPE, input + 34, 1, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST identifiers_and_operators(void) {
    const char *input = "{printf(fmt),puts(strings[idx])*__int128+-/%t_3__;}";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(23, g_tokens.len);

    Token expected[] = {
        {TOKEN_OP_LBRACE, input, 1, true},
        {TOKEN_IDENTIFIER, input + 1, 6, true}, /* printf */
        {TOKEN_OP_LPAREN, input + 7, 1, true},
        {TOKEN_IDENTIFIER, input + 8, 3, true}, /* fmt */
        {TOKEN_OP_RPAREN, input + 11, 1, true},
        {TOKEN_OP_COMMA, input + 12, 1, true},
        {TOKEN_IDENTIFIER, input + 13, 4, true}, /* puts */
        {TOKEN_OP_LPAREN, input + 17, 1, true},
        {TOKEN_IDENTIFIER, input + 18, 7, true}, /* strings */
        {TOKEN_OP_LBRACKET, input + 25, 1, true},
        {TOKEN_IDENTIFIER, input + 26, 3, true}, /* idx */
        {TOKEN_OP_RBRACKET, input + 29, 1, true},
        {TOKEN_OP_RPAREN, input + 30, 1, true},
        {TOKEN_OP_ASTERISK, input + 31, 1, true},
        {TOKEN_IDENTIFIER, input + 32, 8, true}, /* __int128 */
        {TOKEN_OP_PLUS, input + 40, 1, true},
        {TOKEN_OP_MINUS, input + 41, 1, true},
        {TOKEN_OP_DIV, input + 42, 1, true},
        {TOKEN_OP_MOD, input + 43, 1, true},
        {TOKEN_IDENTIFIER, input + 44, 5, true}, /* t_3__ */
        {TOKEN_OP_SEMICOLON, input + 49, 1, true},
        {TOKEN_OP_RBRACE, input + 50, 1, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST identifiers_and_operators_separated_with_whitespaces(void) {
    const char *input = "{\tprintf   ( fmt )   \t,\n\rputs  ( strings   "
                        "[\nidx\r]  ) * __int128   +"
                        "    -  /  %     t_3__ ;\t}\n";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(23, g_tokens.len);

    Token expected[] = {
        {TOKEN_OP_LBRACE, input, 1, true},
        {TOKEN_IDENTIFIER, input + 2, 6, true}, /* printf */
        {TOKEN_OP_LPAREN, input + 11, 1, true},
        {TOKEN_IDENTIFIER, input + 13, 3, true}, /* fmt */
        {TOKEN_OP_RPAREN, input + 17, 1, true},
        {TOKEN_OP_COMMA, input + 22, 1, true},
        {TOKEN_IDENTIFIER, input + 25, 4, true}, /* puts */
        {TOKEN_OP_LPAREN, input + 31, 1, true},
        {TOKEN_IDENTIFIER, input + 33, 7, true}, /* strings */
        {TOKEN_OP_LBRACKET, input + 43, 1, true},
        {TOKEN_IDENTIFIER, input + 45, 3, true}, /* idx */
        {TOKEN_OP_RBRACKET, input + 49, 1, true},
        {TOKEN_OP_RPAREN, input + 52, 1, true},
        {TOKEN_OP_ASTERISK, input + 54, 1, true},
        {TOKEN_IDENTIFIER, input + 56, 8, true}, /* __int128 */
        {TOKEN_OP_PLUS, input + 67, 1, true},
        {TOKEN_OP_MINUS, input + 72, 1, true},
        {TOKEN_OP_DIV, input + 75, 1, true},
        {TOKEN_OP_MOD, input + 78, 1, true},
        {TOKEN_IDENTIFIER, input + 84, 5, true}, /* t_3__ */
        {TOKEN_OP_SEMICOLON, input + 90, 1, true},
        {TOKEN_OP_RBRACE, input + 92, 1, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST empty_string_returns_string_token(void) {
    const char *input = "\"\"";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(2, g_tokens.len);

    Token expected[] = {
        {TOKEN_STRING, input, 2, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST string_returns_string_token(void) {
    const char *input = "\"Hello, World!\"";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(2, g_tokens.len);

    Token expected[] = {
        {TOKEN_STRING, input, 15, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST open_string(void) {
    const char *input = "\"Hello, World!";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(2, g_tokens.len);

    Token expected[] = {
        {TOKEN_STRING, input, 14, false},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST multiline_string(void) {
    const char *input = "\"Hello\nWo\nrld\"";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(2, g_tokens.len);

    Token expected[] = {
        {TOKEN_STRING, input, 14, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST multiline_open_string(void) {
    const char *input = "\"Hello\nWo\nrld";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(2, g_tokens.len);

    Token expected[] = {
        {TOKEN_STRING, input, 13, false},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST embedded_strings(void) {
    const char *input = "\"Hello, \\\"World!\\\"\"";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(2, g_tokens.len);

    Token expected[] = {
        {TOKEN_STRING, input, 19, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST keywords(void) {
    const char *input =
        "if else for while return break continue nil int void string";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(12, g_tokens.len);

    Token expected[] = {
        {TOKEN_KW_IF, input, 2, true},
        {TOKEN_KW_ELSE, input + 3, 4, true},
        {TOKEN_KW_FOR, input + 8, 3, true},
        {TOKEN_KW_WHILE, input + 12, 5, true},
        {TOKEN_KW_RETURN, input + 18, 6, true},
        {TOKEN_KW_BREAK, input + 25, 5, true},
        {TOKEN_KW_CONTINUE, input + 31, 8, true},
        {TOKEN_KW_NIL, input + 40, 3, true},
        {TOKEN_KW_INT, input + 44, 3, true},
        {TOKEN_KW_VOID, input + 48, 4, true},
        {TOKEN_KW_STRING, input + 53, 6, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST keywords_case_sensitivity(void) {
    const char *input =
        "iF ELse fOr WhIlE REtuRn BrEAK ContInuE Nil Int VoId sTRIng";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(12, g_tokens.len);

    Token expected[] = {
        {TOKEN_IDENTIFIER, input, 2, true},
        {TOKEN_IDENTIFIER, input + 3, 4, true},
        {TOKEN_IDENTIFIER, input + 8, 3, true},
        {TOKEN_IDENTIFIER, input + 12, 5, true},
        {TOKEN_IDENTIFIER, input + 18, 6, true},
        {TOKEN_IDENTIFIER, input + 25, 5, true},
        {TOKEN_IDENTIFIER, input + 31, 8, true},
        {TOKEN_IDENTIFIER, input + 40, 3, true},
        {TOKEN_IDENTIFIER, input + 44, 3, true},
        {TOKEN_IDENTIFIER, input + 48, 4, true},
        {TOKEN_IDENTIFIER, input + 53, 6, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST oneline_comment_at_the_beginning(void) {
    const char *input = "// this is a oneline comment.";
    lexer_lex(input, 0, &g_tokens);

    ASSERT_EQ(1, g_tokens.len);

    Token expected = {TOKEN_EOF, input + strlen(input), 0, true};
    ASSERT_EQUAL_T(&expected, &NTH_TOKEN(0), &g_token_type_info, NULL);

    PASS();
}

TEST oneline_comment_at_the_end(void) {
    const char *input = "123456789 //some //\tinteger";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(2, g_tokens.len);

    Token expected[] = {
        {TOKEN_INTEGER, input, 9, true},
        {TOKEN_EOF, input + strlen(input), 0, true}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

SUITE(g_test_suite) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(empty_input_returns_eof_token);
    RUN_TEST(whitespaces_return_eof_token);
    RUN_TEST(invalid_token);
    RUN_TEST(integer);
    RUN_TEST(integers_separated_with_whitespace);
    RUN_TEST(identifier);
    RUN_TEST(identifiers_separated_with_whitespace);
    RUN_TEST(single_operators);
    RUN_TEST(double_operators);
    RUN_TEST(double_operators_cannot_be_splitted);
    RUN_TEST(identifiers_and_operators);
    RUN_TEST(identifiers_and_operators_separated_with_whitespaces);
    RUN_TEST(empty_string_returns_string_token);
    RUN_TEST(string_returns_string_token);
    RUN_TEST(open_string);
    RUN_TEST(multiline_string);
    RUN_TEST(multiline_open_string);
    RUN_TEST(embedded_strings);
    RUN_TEST(keywords);
    RUN_TEST(keywords_case_sensitivity);
    RUN_TEST(oneline_comment_at_the_beginning);
    RUN_TEST(oneline_comment_at_the_end);
}

GREATEST_MAIN_DEFS();

int main(int argc, char *argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(g_test_suite);

    GREATEST_MAIN_END();
}
