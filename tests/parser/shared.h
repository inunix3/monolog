#pragma once

#ifndef GOLDEN_FILES_PATH
#define GOLDEN_FILES_PATH "./tests/golden-files"
#endif

#include <monolog/lexer.h>
#include <monolog/parser.h>
#include <monolog/utils.h>

#include <greatest.h>

#include <string.h>

static Vector g_tokens;
static Parser g_parser;
static Ast g_ast;

static void parse(const char *input) {
    lexer_lex(input, strlen(input), &g_tokens);
    g_parser = parser_new(g_tokens.data, g_tokens.len);
    g_ast = parser_parse(&g_parser);
}

static void set_up(void *udata) {
    (void)udata;

    vec_init(&g_tokens, sizeof(Token));
}

static void tear_down(void *udata) {
    (void)udata;

    ast_destroy(&g_ast);
    vec_deinit(&g_tokens);
}

static greatest_test_res
assert_string_against_file(const char *input, const char *golden_file_name) {
    char *expected = read_file(golden_file_name);

    if (!expected) {
        FAIL();
    }

    parse(input);

#if PARSER_SHOULD_FAIL
    ASSERT_EQ(true, g_parser.had_error);
#endif

    FILE *tmp = fopen(".temp.txt", "w+");

    if (!tmp) {
        perror("cannot create .temp.txt");
        free(expected);

        FAIL();
    }

    ast_dump(&g_ast, tmp);

    rewind(tmp);
    char *got = read_file_stream(tmp);

    if (!got) {
        fclose(tmp);
        free(expected);

        FAIL();
    }

    ASSERT_STR_EQ(expected, got);

    free(got);
    free(expected);
    fclose(tmp);

    PASS();
}

#define XSTRINGIFY(a) #a
#define STRINGIFY(a) XSTRINGIFY(a)

#ifdef SUITE_NAME
#define TEST_STRING_AGAINST_FILE(_name, _input, _filename)                     \
    TEST _name(void) {                                                         \
        return assert_string_against_file(                                     \
            _input, GOLDEN_FILES_PATH "/" STRINGIFY(SUITE_NAME) "/" _filename  \
        );                                                                     \
    }
#else
#define TEST_STRING_AGAINST_FILE(_name, _input, _filename)                     \
    TEST _name(void) {                                                         \
        return assert_string_against_file(                                     \
            _input, GOLDEN_FILES_PATH "/" _filename                            \
        );                                                                     \
    }
#endif
