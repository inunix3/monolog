#pragma once

#include <monolog/semck.h>
#include <monolog/diagnostic.h>
#include <monolog/lexer.h>
#include <monolog/parser.h>

#include <greatest.h>

#include <stdbool.h>
#include <string.h>

static Ast g_ast;
static TypeSystem g_types;
static SemChecker g_semck;

static void set_up(void *udata) {
    (void)udata;

    type_system_init(&g_types);
    semck_init(&g_semck, &g_types);
}

static void tear_down(void *udata) {
    (void)udata;

    semck_deinit(&g_semck);
    type_system_deinit(&g_types);
    ast_destroy(&g_ast);
}

static bool parse(const char *input) {
    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    lexer_lex(input, strlen(input), &tokens);
    Parser parser = parser_new(tokens.data, tokens.len);
    g_ast = parser_parse(&parser);

    vec_deinit(&tokens);

    return !parser.error_state;
}

#define CHECK(_input) \
    if (parse(_input)) { \
        ASSERT(semck_check(&g_semck, &g_ast, NULL, NULL)); \
    }

#define CHECK_FAIL(_input) \
    if (parse(_input)) { \
        ASSERT_FALSE(semck_check(&g_semck, &g_ast, NULL, NULL)); \
    }

#define NTH_DMSG(_idx) (((DiagnosticMessage *)g_semck.dmsgs.data)[_idx])
