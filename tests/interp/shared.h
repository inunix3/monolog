#pragma once

#include <monolog/semck.h>
#include <monolog/diagnostic.h>
#include <monolog/interp.h>
#include <monolog/lexer.h>
#include <monolog/parser.h>

#include <greatest.h>

#include <stdbool.h>
#include <string.h>

static Ast g_ast;
static SemChecker g_semck;
static Interpreter g_interp;

static void set_up(void *udata) {
    (void)udata;

    semck_init(&g_semck);
    interp_init(&g_interp, NULL);
}

static void tear_down(void *udata) {
    (void)udata;

    interp_deinit(&g_interp);
    semck_deinit(&g_semck);
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

static void run(const char *input) {
    if (parse(input)) {
        interp_init(&g_interp, &g_ast);
        interp_run(&g_interp);
    }
}
