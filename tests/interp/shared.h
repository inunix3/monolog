#pragma once

#include <monolog/diagnostic.h>
#include <monolog/interp.h>
#include <monolog/lexer.h>
#include <monolog/parser.h>
#include <monolog/semck.h>

#include <greatest.h>

#include <stdbool.h>
#include <string.h>

static Ast g_ast;
static TypeSystem g_types;
static SemChecker g_semck;
static Interpreter g_interp;

static void set_up(void *udata) {
    (void)udata;

    type_system_init(&g_types);
    semck_init(&g_semck, &g_types);
    interp_init(&g_interp, &g_ast, &g_types);
}

static void tear_down(void *udata) {
    (void)udata;

    interp_deinit(&g_interp);
    semck_deinit(&g_semck);
    type_system_deinit(&g_types);
    ast_destroy(&g_ast);
}

static bool parse(const char *input) {
    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    lexer_lex(input, strlen(input), &tokens);
    Parser parser = parser_new(tokens.data, tokens.len);
    ast_destroy(&g_ast);
    g_ast = parser_parse(&parser);

    vec_deinit(&tokens);

    semck_reset(&g_semck);

    if (!parser.error_state &&
        semck_check(
            &g_semck, &g_ast, &g_interp.env.global_scope->vars,
            &g_interp.env.funcs
        )) {
        return true;
    } else {
        return false;
    }
}

static Value eval(const char *input) {
    if (parse(input)) {
        return interp_eval(&g_interp);
    }

    return (Value){g_types.error_type};
}

static void run(const char *input) {
    if (parse(input)) {
        interp_walk(&g_interp);
    }
}
