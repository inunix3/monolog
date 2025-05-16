/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/cli.h>
#include <monolog/interp.h>
#include <monolog/lexer.h>
#include <monolog/parser.h>
#include <monolog/semck.h>
#include <monolog/utils.h>
#include <monolog/vector.h>

#include <isocline.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
highlighter(ic_highlight_env_t *henv, const char *input, void *arg) {
    (void) arg;

    static const char *keywords[] = {"break",     "continue",     "return",
                                     "print",     "println",      "exit",
                                     "input_int", "input_string", NULL};
    static const char *controls[] = {"if", "else", "while", "for", NULL};
    static const char *types[] = {"int", "string", "void", NULL};

    long len = (long) strlen(input);

    for (long i = 0; i < len; ++i) {
        switch (input[i]) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
        case ',':
        case ';':
        case '(':
        case ')':
        case ']':
        case '[':
        case '{':
        case '}':
        case '+':
        case '-':
        case '*':
        case '%':
        case '!':
        case '&':
        case '|':
        case '<':
        case '>':
        case '=':
        case '?':
        case '#':
        case '$':
            continue;
        default:
            break;
        }

        if (input[i] == '/' && i < len - 1 && input[i + 1] != '/') {
            continue;
        }

        long tok_len = 0;

        if ((tok_len =
                 ic_match_any_token(input, i, &ic_char_is_idletter, keywords)) >
            0) {
            ic_highlight(henv, i, tok_len, "keyword");
        } else if ((tok_len = ic_match_any_token(
                        input, i, &ic_char_is_idletter, controls
                    )) > 0) {
            ic_highlight(henv, i, tok_len, "plum");
        } else if ((tok_len = ic_match_any_token(
                        input, i, &ic_char_is_idletter, types
                    )) > 0) {
            ic_highlight(henv, i, tok_len, "type");
        } else if ((tok_len = ic_is_token(input, i, &ic_char_is_digit)) > 0) {
            ic_highlight(henv, i, tok_len, "number");
        } else if (ic_starts_with(input + i, "//")) {
            tok_len = 2;

            while (i + tok_len < len && input[i + tok_len] != '\n') {
                tok_len++;
            }

            ic_highlight(henv, i, tok_len, "comment");
        } else {
            ic_highlight(henv, i, 1, NULL);

            tok_len = 1;
        }

        i += tok_len;
    }
}

int cmd_run(int argc, char **argv) {
    UNUSED(argc);

    const char *filename = argv[2];

    char *input = read_file(filename);

    if (!input) {
        perror("error: cannot read input file");

        return -1;
    }

    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    lexer_lex(input, strlen(input), &tokens);

    Parser parser = parser_new(tokens.data, tokens.len);
    parser.log_errors = true;

    Ast ast = parser_parse(&parser);

    TypeSystem types;
    type_system_init(&types);

    bool had_error = parser.had_error;

    if (!had_error) {
        SemChecker semck;
        semck_init(&semck, &types);
        had_error = !semck_check(&semck, &ast, NULL, NULL);
        DiagnosticMessage *dmsgs = semck.dmsgs.data;

        for (size_t i = 0; i < semck.dmsgs.len; ++i) {
            const DiagnosticMessage *dmsg = &dmsgs[i];

            printf(
                "%d:%d: error: %s\n", dmsg->src_info.line, dmsg->src_info.col,
                dmsg_to_str(dmsg)
            );
        }

        semck_deinit(&semck);
    }

    int exit_code = -1;

    if (!had_error) {
        Interpreter interp;
        interp_init(&interp, &ast, &types);
        interp.log_errors = true;

        exit_code = interp_walk(&interp);
        interp_deinit(&interp);
    }

    type_system_deinit(&types);
    ast_destroy(&ast);
    vec_deinit(&tokens);
    free(input);

    return exit_code;
}

int cmd_scan(int argc, char **argv) {
    UNUSED(argc);

    const char *filename = argv[2];

    char *input = read_file(filename);

    if (!input) {
        perror("error: cannot read input file");

        return -1;
    }

    Vector tokens;
    vec_init(&tokens, sizeof(Token));
    lexer_lex(input, strlen(input), &tokens);

    for (size_t i = 0; i < tokens.len; ++i) {
        const Token *toks = tokens.data;
        const Token *tok = &toks[i];

        printf("Token %zu:\n", i + 1);
        printf("  kind: %s (%d)\n", token_kind_to_str(tok->kind), tok->kind);
        printf("  len: %zu\n", tok->len);
        printf("  src: '%.*s'\n", (int) tok->len, tok->src);
    }

    vec_deinit(&tokens);
    free(input);

    return 0;
}

int cmd_parse(int argc, char **argv) {
    UNUSED(argc);

    const char *filename = argv[2];

    char *input = read_file(filename);

    if (!input) {
        perror("error: cannot read input file");

        return -1;
    }

    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    lexer_lex(input, strlen(input), &tokens);

    Parser parser = parser_new(tokens.data, tokens.len);
    parser.log_errors = true;

    Ast ast = parser_parse(&parser);
    ast_dump(&ast, stdout);

    if (!parser.had_error) {
        TypeSystem types;
        type_system_init(&types);

        SemChecker semck;
        semck_init(&semck, &types);
        semck_check(&semck, &ast, NULL, NULL);

        DiagnosticMessage *dmsgs = semck.dmsgs.data;

        for (size_t i = 0; i < semck.dmsgs.len; ++i) {
            const DiagnosticMessage *dmsg = &dmsgs[i];

            printf(
                "%d:%d: error: %s\n", dmsg->src_info.line, dmsg->src_info.col,
                dmsg_to_str(dmsg)
            );
        }

        semck_deinit(&semck);
        type_system_deinit(&types);
    }

    vec_deinit(&tokens);
    free(input);

    return parser.had_error ? -1 : 0;
}

static void print_help(void) {
    printf("usage: monolog [run] FILENAME\n"
           "       monolog scan FILENAME\n"
           "       monolog parse FILENAME\n"
           "       monolog repl\n");
}

int cmd_repl(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    ic_set_default_highlighter(highlighter, NULL);
    ic_set_history(".monologhist", -1); /* -1 for default 200 entries */

    TypeSystem types;
    type_system_init(&types);

    SemChecker semck;
    semck_init(&semck, &types);

    Interpreter interp;
    interp_init(&interp, NULL, &types);
    interp.log_errors = true;

    char *input;
    while ((input = ic_readline(""))) {
        interp.had_error = false;
        interp.halt = false;

        lexer_lex(input, strlen(input), &tokens);

        Parser parser = parser_new(tokens.data, tokens.len);
        parser.log_errors = true;

        Ast ast = parser_parse(&parser);
        bool had_error = parser.had_error;

        if (!had_error) {
            semck_reset(&semck);

            had_error = !semck_check(
                &semck, &ast, &interp.env.global_scope->vars, &interp.env.funcs
            );

            DiagnosticMessage *dmsgs = semck.dmsgs.data;

            for (size_t i = 0; i < semck.dmsgs.len; ++i) {
                const DiagnosticMessage *dmsg = &dmsgs[i];

                printf(
                    "%d:%d: error: %s\n", dmsg->src_info.line,
                    dmsg->src_info.col, dmsg_to_str(dmsg)
                );
            }
        }

        if (!had_error) {
            interp.ast = &ast;
            interp_walk(&interp);
        }

        ast_destroy(&ast);
        free(input);
        vec_clear(&tokens);

        if (!interp.had_error && interp.halt) {
            break;
        }
    }

    interp_deinit(&interp);
    semck_deinit(&semck);
    type_system_deinit(&types);
    vec_deinit(&tokens);

    return 0;
}

static CliCommand g_cmds[] = {
    {.name = "run", .args = 1, .fn = cmd_run},
    {.name = "scan", .args = 1, .fn = cmd_scan},
    {.name = "parse", .args = 1, .fn = cmd_parse},
    {.name = "repl", .args = 0, .fn = cmd_repl}
};

int main(int argc, char **argv) {
    /* monolog run FILENAME */
    if (argc < 2) {
        print_help();

        return -1;
    }

    const char *cmd = argv[1];

    for (size_t i = 0; i < ARRAY_SIZE(g_cmds); ++i) {
        if (strcmp(g_cmds[i].name, cmd) == 0) {
            return g_cmds[i].fn(argc, argv);
        }
    }

    fprintf(stderr, "error: invalid command %s\n", cmd);
    print_help();

    return -1;
}
