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
           exp_tok->len == got_tok->len && exp_tok->valid == got_tok->valid &&
           exp_tok->src_info.line == got_tok->src_info.line &&
           exp_tok->src_info.col == got_tok->src_info.col;
}

static int token_printf_cb(const void *data, void *udata) {
    (void)udata;

    const Token *tok = data;

    return printf(
        "{ kind = %d; src = %p; len = %zu; valid = %s; line = %d; col = %d; "
        "}",
        tok->kind, tok->src, tok->len, tok->valid ? "true" : "false", tok->src_info.line,
        tok->src_info.col
    );
}

static greatest_type_info g_token_type_info = {token_equal_cb, token_printf_cb};

static void set_up(void *udata) {
    (void)udata;

    vec_init(&g_tokens, sizeof(Token));
}

static void tear_down(void *udata) {
    (void)udata;

    vec_deinit(&g_tokens);
}

#define NTH_TOKEN(_idx) (((Token *)g_tokens.data)[_idx])

TEST empty_input_returns_eof_token(void) {
    const char *input = "";
    lexer_lex(input, 0, &g_tokens);

    ASSERT_EQ(1, g_tokens.len);

    Token expected = {TOKEN_EOF, input, 0, true, {1, 1}};
    ASSERT_EQUAL_T(&expected, &NTH_TOKEN(0), &g_token_type_info, NULL);

    PASS();
}

TEST whitespaces_return_eof_token(void) {
    const char *input = "         \t\n\r    \t    \n    \r     ";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(1, g_tokens.len);

    Token expected = {TOKEN_EOF, input + strlen(input), 0, true, {3, 11}};
    ASSERT_EQUAL_T(&expected, &NTH_TOKEN(0), &g_token_type_info, NULL);

    PASS();
}

TEST invalid_tokens(void) {
    const char *input = "`~~@^,ads*`fd#$~3432";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(9, g_tokens.len);

    Token expected[] = {
        {TOKEN_UNKNOWN, input, 5, false, {1, 1}},
        {TOKEN_OP_COMMA, input + 5, 1, true, {1, 6}},
        {TOKEN_IDENTIFIER, input + 6, 3, true, {1, 7}},
        {TOKEN_OP_MUL, input + 9, 1, true, {1, 10}},
        {TOKEN_UNKNOWN, input + 10, 3, false, {1, 11}},
        {TOKEN_OP_HASHTAG, input + 13, 1, true, {1, 14}},
        {TOKEN_OP_DOLAR, input + 14, 1, true, {1, 15}},
        {TOKEN_UNKNOWN, input + 15, 5, false, {1, 16}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 21}}
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
        {TOKEN_INTEGER, input, 9, true, {1, 1}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 10}}
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
        {TOKEN_INTEGER, input, 3, true, {1, 1}},
        {TOKEN_INTEGER, input + 4, 3, true, {1, 5}},
        {TOKEN_INTEGER, input + 9, 3, true, {2, 2}},
        {TOKEN_INTEGER, input + 13, 3, true, {2, 6}},
        {TOKEN_INTEGER, input + 19, 2, true, {3, 2}},
        {TOKEN_EOF, input + strlen(input), 0, true, {3, 4}}
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
        {TOKEN_IDENTIFIER, input, strlen(input), true, {1, 1}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 26}}
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
        {TOKEN_IDENTIFIER, input, 10, true, {1, 1}},
        {TOKEN_IDENTIFIER, input + 11, 5, true, {1, 12}},
        {TOKEN_IDENTIFIER, input + 19, 5, true, {2, 3}},
        {TOKEN_IDENTIFIER, input + 25, 8, true, {2, 9}},
        {TOKEN_IDENTIFIER, input + 36, 4, true, {3, 2}},
        {TOKEN_EOF, input + strlen(input), 0, true, {3, 6}}
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
        {TOKEN_OP_ASSIGN, input, 1, true, {1, 1}},
        {TOKEN_OP_COMMA, input + 1, 1, true, {1, 2}},
        {TOKEN_OP_SEMICOLON, input + 2, 1, true, {1, 3}},
        {TOKEN_OP_PLUS, input + 3, 1, true, {1, 4}},
        {TOKEN_OP_MINUS, input + 4, 1, true, {1, 5}},
        {TOKEN_OP_DIV, input + 5, 1, true, {1, 6}},
        {TOKEN_OP_MOD, input + 6, 1, true, {1, 7}},
        {TOKEN_OP_LPAREN, input + 7, 1, true, {1, 8}},
        {TOKEN_OP_RPAREN, input + 8, 1, true, {1, 9}},
        {TOKEN_OP_LBRACKET, input + 9, 1, true, {1, 10}},
        {TOKEN_OP_RBRACKET, input + 10, 1, true, {1, 11}},
        {TOKEN_OP_LBRACE, input + 11, 1, true, {1, 12}},
        {TOKEN_OP_RBRACE, input + 12, 1, true, {1, 13}},
        {TOKEN_OP_DOLAR, input + 13, 1, true, {1, 14}},
        {TOKEN_OP_HASHTAG, input + 14, 1, true, {1, 15}},
        {TOKEN_OP_QUEST, input + 15, 1, true, {1, 16}},
        {TOKEN_OP_EXCL, input + 16, 1, true, {1, 17}},
        {TOKEN_OP_AMP, input + 17, 1, true, {1, 18}},
        {TOKEN_OP_PIPE, input + 18, 1, true, {1, 19}},
        {TOKEN_OP_LESS, input + 19, 1, true, {1, 20}},
        {TOKEN_OP_GREATER, input + 20, 1, true, {1, 21}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 22}}
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
        {TOKEN_OP_NOT_EQUAL, input, 2, true, {1, 1}},
        {TOKEN_OP_EQUAL, input + 3, 2, true, {1, 4}},
        {TOKEN_OP_LESS_EQUAL, input + 6, 2, true, {1, 7}},
        {TOKEN_OP_GREATER_EQUAL, input + 9, 2, true, {1, 10}},
        {TOKEN_OP_INC, input + 12, 2, true, {1, 13}},
        {TOKEN_OP_DEC, input + 15, 2, true, {1, 16}},
        {TOKEN_OP_AND, input + 18, 2, true, {1, 19}},
        {TOKEN_OP_OR, input + 21, 2, true, {1, 22}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 24}}
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
        {TOKEN_OP_EXCL, input, 1, true, {1, 1}},
        {TOKEN_OP_ASSIGN, input + 2, 1, true, {1, 3}},
        {TOKEN_OP_ASSIGN, input + 4, 1, true, {1, 5}},
        {TOKEN_OP_ASSIGN, input + 6, 1, true, {2, 1}},
        {TOKEN_OP_LESS, input + 8, 1, true, {2, 3}},
        {TOKEN_OP_ASSIGN, input + 10, 1, true, {2, 5}},
        {TOKEN_OP_GREATER, input + 12, 1, true, {2, 7}},
        {TOKEN_OP_ASSIGN, input + 17, 1, true, {2, 12}},
        {TOKEN_OP_PLUS, input + 19, 1, true, {2, 14}},
        {TOKEN_OP_PLUS, input + 21, 1, true, {2, 16}},
        {TOKEN_OP_MINUS, input + 23, 1, true, {2, 18}},
        {TOKEN_OP_MINUS, input + 25, 1, true, {2, 20}},
        {TOKEN_OP_AMP, input + 27, 1, true, {2, 22}},
        {TOKEN_OP_AMP, input + 29, 1, true, {2, 24}},
        {TOKEN_OP_PIPE, input + 31, 1, true, {2, 26}},
        {TOKEN_OP_PIPE, input + 34, 1, true, {2, 29}},
        {TOKEN_EOF, input + strlen(input), 0, true, {2, 30}}
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
        {TOKEN_OP_LBRACE, input, 1, true, {1, 1}},
        {TOKEN_IDENTIFIER, input + 1, 6, true, {1, 2}}, /* printf */
        {TOKEN_OP_LPAREN, input + 7, 1, true, {1, 8}},
        {TOKEN_IDENTIFIER, input + 8, 3, true, {1, 9}}, /* fmt */
        {TOKEN_OP_RPAREN, input + 11, 1, true, {1, 12}},
        {TOKEN_OP_COMMA, input + 12, 1, true, {1, 13}},
        {TOKEN_IDENTIFIER, input + 13, 4, true, {1, 14}}, /* puts */
        {TOKEN_OP_LPAREN, input + 17, 1, true, {1, 18}},
        {TOKEN_IDENTIFIER, input + 18, 7, true, {1, 19}}, /* strings */
        {TOKEN_OP_LBRACKET, input + 25, 1, true, {1, 26}},
        {TOKEN_IDENTIFIER, input + 26, 3, true, {1, 27}}, /* idx */
        {TOKEN_OP_RBRACKET, input + 29, 1, true, {1, 30}},
        {TOKEN_OP_RPAREN, input + 30, 1, true, {1, 31}},
        {TOKEN_OP_MUL, input + 31, 1, true, {1, 32}},
        {TOKEN_IDENTIFIER, input + 32, 8, true, {1, 33}}, /* __int128 */
        {TOKEN_OP_PLUS, input + 40, 1, true, {1, 41}},
        {TOKEN_OP_MINUS, input + 41, 1, true, {1, 42}},
        {TOKEN_OP_DIV, input + 42, 1, true, {1, 43}},
        {TOKEN_OP_MOD, input + 43, 1, true, {1, 44}},
        {TOKEN_IDENTIFIER, input + 44, 5, true, {1, 45}}, /* t_3__ */
        {TOKEN_OP_SEMICOLON, input + 49, 1, true, {1, 50}},
        {TOKEN_OP_RBRACE, input + 50, 1, true, {1, 51}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 52}}
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
        {TOKEN_OP_LBRACE, input, 1, true, {1, 1}},
        {TOKEN_IDENTIFIER, input + 2, 6, true, {1, 3}}, /* printf */
        {TOKEN_OP_LPAREN, input + 11, 1, true, {1, 12}},
        {TOKEN_IDENTIFIER, input + 13, 3, true, {1, 14}}, /* fmt */
        {TOKEN_OP_RPAREN, input + 17, 1, true, {1, 18}},
        {TOKEN_OP_COMMA, input + 22, 1, true, {1, 23}},
        {TOKEN_IDENTIFIER, input + 25, 4, true, {2, 2}}, /* puts */
        {TOKEN_OP_LPAREN, input + 31, 1, true, {2, 8}},
        {TOKEN_IDENTIFIER, input + 33, 7, true, {2, 10}}, /* strings */
        {TOKEN_OP_LBRACKET, input + 43, 1, true, {2, 20}},
        {TOKEN_IDENTIFIER, input + 45, 3, true, {3, 1}}, /* idx */
        {TOKEN_OP_RBRACKET, input + 49, 1, true, {3, 5}},
        {TOKEN_OP_RPAREN, input + 52, 1, true, {3, 8}},
        {TOKEN_OP_MUL, input + 54, 1, true, {3, 10}},
        {TOKEN_IDENTIFIER, input + 56, 8, true, {3, 12}}, /* __int128 */
        {TOKEN_OP_PLUS, input + 67, 1, true, {3, 23}},
        {TOKEN_OP_MINUS, input + 72, 1, true, {3, 28}},
        {TOKEN_OP_DIV, input + 75, 1, true, {3, 31}},
        {TOKEN_OP_MOD, input + 78, 1, true, {3, 34}},
        {TOKEN_IDENTIFIER, input + 84, 5, true, {3, 40}}, /* t_3__ */
        {TOKEN_OP_SEMICOLON, input + 90, 1, true, {3, 46}},
        {TOKEN_OP_RBRACE, input + 92, 1, true, {3, 48}},
        {TOKEN_EOF, input + strlen(input), 0, true, {4, 1}}
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
        {TOKEN_STRING, input, 2, true, {1, 1}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 3}}
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
        {TOKEN_STRING, input, 15, true, {1, 1}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 16}}
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
        {TOKEN_STRING, input, 14, false, {1, 1}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 15}}
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
        {TOKEN_STRING, input, 14, true, {1, 1}},
        {TOKEN_EOF, input + strlen(input), 0, true, {3, 5}}
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
        {TOKEN_STRING, input, 13, false, {1, 1}},
        {TOKEN_EOF, input + strlen(input), 0, true, {3, 4}}
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
        {TOKEN_STRING, input, 19, true, {1, 1}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 20}}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST keywords(void) {
    const char *input = "if else for while return break continue nil int void "
                        "string print println exit";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(15, g_tokens.len);

    Token expected[] = {
        {TOKEN_KW_IF, input, 2, true, {1, 1}},
        {TOKEN_KW_ELSE, input + 3, 4, true, {1, 4}},
        {TOKEN_KW_FOR, input + 8, 3, true, {1, 9}},
        {TOKEN_KW_WHILE, input + 12, 5, true, {1, 13}},
        {TOKEN_KW_RETURN, input + 18, 6, true, {1, 19}},
        {TOKEN_KW_BREAK, input + 25, 5, true, {1, 26}},
        {TOKEN_KW_CONTINUE, input + 31, 8, true, {1, 32}},
        {TOKEN_KW_NIL, input + 40, 3, true, {1, 41}},
        {TOKEN_KW_INT, input + 44, 3, true, {1, 45}},
        {TOKEN_KW_VOID, input + 48, 4, true, {1, 49}},
        {TOKEN_KW_STRING, input + 53, 6, true, {1, 54}},
        {TOKEN_KW_PRINT, input + 60, 5, true, {1, 61}},
        {TOKEN_KW_PRINTLN, input + 66, 7, true, {1, 67}},
        {TOKEN_KW_EXIT, input + 74, 4, true, {1, 75}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 79}}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST keywords_case_sensitivity(void) {
    const char *input = "iF ELSE fOr WhIlE REtuRn BrEAK ContInuE Nil Int VoId "
                        "sTRIng PRINT printLn eXIt";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(15, g_tokens.len);

    Token expected[] = {
        {TOKEN_IDENTIFIER, input, 2, true, {1, 1}},
        {TOKEN_IDENTIFIER, input + 3, 4, true, {1, 4}},
        {TOKEN_IDENTIFIER, input + 8, 3, true, {1, 9}},
        {TOKEN_IDENTIFIER, input + 12, 5, true, {1, 13}},
        {TOKEN_IDENTIFIER, input + 18, 6, true, {1, 19}},
        {TOKEN_IDENTIFIER, input + 25, 5, true, {1, 26}},
        {TOKEN_IDENTIFIER, input + 31, 8, true, {1, 32}},
        {TOKEN_IDENTIFIER, input + 40, 3, true, {1, 41}},
        {TOKEN_IDENTIFIER, input + 44, 3, true, {1, 45}},
        {TOKEN_IDENTIFIER, input + 48, 4, true, {1, 49}},
        {TOKEN_IDENTIFIER, input + 53, 6, true, {1, 54}},
        {TOKEN_IDENTIFIER, input + 60, 5, true, {1, 61}},
        {TOKEN_IDENTIFIER, input + 66, 7, true, {1, 67}},
        {TOKEN_IDENTIFIER, input + 74, 4, true, {1, 75}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 79}}
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

    Token expected = {TOKEN_EOF, input + strlen(input), 0, true, {1, 30}};
    ASSERT_EQUAL_T(&expected, &NTH_TOKEN(0), &g_token_type_info, NULL);

    PASS();
}

TEST oneline_comment_at_the_end(void) {
    const char *input = "123456789 //some //\tinteger";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(2, g_tokens.len);

    Token expected[] = {
        {TOKEN_INTEGER, input, 9, true, {1, 1}},
        {TOKEN_EOF, input + strlen(input), 0, true, {1, 28}}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

TEST prog1_factorial(void) {
    const char *input = "int factorial(int n) {\n"
                        "\tint f = 1;\n"
                        "\n"
                        "\tint i = 2;\n"
                        "\twhile (i <= n) {\n"
                        "\t\tf = f * i;\n"
                        "\n"
                        "\t\t++i;\n"
                        "\t}\n"
                        "\n"
                        "\treturn f;\n"
                        "}";
    lexer_lex(input, strlen(input), &g_tokens);

    ASSERT_EQ(39, g_tokens.len);

    Token expected[] = {
        {TOKEN_KW_INT, input, 3, true, {1, 1}},
        {TOKEN_IDENTIFIER, input + 4, 9, true, {1, 5}},
        {TOKEN_OP_LPAREN, input + 13, 1, true, {1, 14}},
        {TOKEN_KW_INT, input + 14, 3, true, {1, 15}},
        {TOKEN_IDENTIFIER, input + 18, 1, true, {1, 19}},
        {TOKEN_OP_RPAREN, input + 19, 1, true, {1, 20}},
        {TOKEN_OP_LBRACE, input + 21, 1, true, {1, 22}},
        {TOKEN_KW_INT, input + 24, 3, true, {2, 2}},
        {TOKEN_IDENTIFIER, input + 28, 1, true, {2, 6}},
        {TOKEN_OP_ASSIGN, input + 30, 1, true, {2, 8}},
        {TOKEN_INTEGER, input + 32, 1, true, {2, 10}},
        {TOKEN_OP_SEMICOLON, input + 33, 1, true, {2, 11}},
        {TOKEN_KW_INT, input + 37, 3, true, {4, 2}},
        {TOKEN_IDENTIFIER, input + 41, 1, true, {4, 6}},
        {TOKEN_OP_ASSIGN, input + 43, 1, true, {4, 8}},
        {TOKEN_INTEGER, input + 45, 1, true, {4, 10}},
        {TOKEN_OP_SEMICOLON, input + 46, 1, true, {4, 11}},
        {TOKEN_KW_WHILE, input + 49, 5, true, {5, 2}},
        {TOKEN_OP_LPAREN, input + 55, 1, true, {5, 8}},
        {TOKEN_IDENTIFIER, input + 56, 1, true, {5, 9}},
        {TOKEN_OP_LESS_EQUAL, input + 58, 2, true, {5, 11}},
        {TOKEN_IDENTIFIER, input + 61, 1, true, {5, 14}},
        {TOKEN_OP_RPAREN, input + 62, 1, true, {5, 15}},
        {TOKEN_OP_LBRACE, input + 64, 1, true, {5, 17}},
        {TOKEN_IDENTIFIER, input + 68, 1, true, {6, 3}},
        {TOKEN_OP_ASSIGN, input + 70, 1, true, {6, 5}},
        {TOKEN_IDENTIFIER, input + 72, 1, true, {6, 7}},
        {TOKEN_OP_MUL, input + 74, 1, true, {6, 9}},
        {TOKEN_IDENTIFIER, input + 76, 1, true, {6, 11}},
        {TOKEN_OP_SEMICOLON, input + 77, 1, true, {6, 12}},
        {TOKEN_OP_INC, input + 82, 2, true, {8, 3}},
        {TOKEN_IDENTIFIER, input + 84, 1, true, {8, 5}},
        {TOKEN_OP_SEMICOLON, input + 85, 1, true, {8, 6}},
        {TOKEN_OP_RBRACE, input + 88, 1, true, {9, 2}},
        {TOKEN_KW_RETURN, input + 92, 6, true, {11, 2}},
        {TOKEN_IDENTIFIER, input + 99, 1, true, {11, 9}},
        {TOKEN_OP_SEMICOLON, input + 100, 1, true, {11, 10}},
        {TOKEN_OP_RBRACE, input + 102, 1, true, {12, 1}},
        {TOKEN_EOF, input + strlen(input), 0, true, {12, 2}}
    };

    for (int i = 0; i < g_tokens.len; ++i) {
        ASSERT_EQUAL_T(&expected[i], &NTH_TOKEN(i), &g_token_type_info, NULL);
    }

    PASS();
}

SUITE(lexer) {
    GREATEST_SET_SETUP_CB(set_up, NULL);
    GREATEST_SET_TEARDOWN_CB(tear_down, NULL);

    RUN_TEST(empty_input_returns_eof_token);
    RUN_TEST(whitespaces_return_eof_token);
    RUN_TEST(invalid_tokens);
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
    RUN_TEST(prog1_factorial);
}

GREATEST_MAIN_DEFS();

int main(int argc, char *argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(lexer);

    GREATEST_MAIN_END();
}
