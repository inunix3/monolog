/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/lexer.h>
#include <monolog/parser.h>
#include <monolog/utils.h>
#include <monolog/vector.h>

#include <isocline.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void cmd_tokenize(char *buf, size_t size) {
    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    lexer_lex(buf, size, &tokens);

    for (size_t i = 0; i < tokens.len; ++i) {
        const Token *toks = tokens.data;
        const Token *tok = &toks[i];

        printf("Token %zu:\n", i + 1);
        printf("  kind: %s (%d)\n", token_kind_to_str(tok->kind), tok->kind);
        printf("  len: %zu\n", tok->len);
        printf("  src excerpt: '%.*s'\n", (int)tok->len, tok->src);
    }

    vec_deinit(&tokens);
}

static void cmd_parse(char *buf, size_t size) {
    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    lexer_lex(buf, size, &tokens);

    Parser parser = parser_new(tokens.data, tokens.len);
    parser.log_errors = true;

    Ast ast = parser_parse(&parser);

    ast_dump(&ast, stdout);

    vec_deinit(&tokens);
}

static void print_help(void) {
    printf("usage: monolog COMMAND FILENAME\n"
           "\n"
           "COMMAND can be one of:\n"
           "  tokenize - print tokens after lexing and exit\n"
           "  parse - print the AST after parsing and exit\n"
           "  help - print this message and exit\n");
}

static void cmd_repl(void) {
    Vector tokens;
    vec_init(&tokens, sizeof(Token));

    ic_term_init();
    ic_set_history(".monologhist", -1); /* -1 for default 200 entries */

    char *input;
    while ((input = ic_readline(""))) {
        bool stop = strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0;

        if (!stop) {
            lexer_lex(input, strlen(input), &tokens);

            Parser parser = parser_new(tokens.data, tokens.len);
            parser.log_errors = true;
            Ast ast = parser_parse(&parser);

            ast_dump(&ast, stdout);
            ast_destroy(&ast);
        }

        free(input);
        vec_clear(&tokens);

        if (stop) {
            break;
        }
    }

    ic_term_done();

    vec_deinit(&tokens);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_help();

        return EXIT_FAILURE;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "repl") == 0) {
        cmd_repl();

        return EXIT_SUCCESS;
    }

    const char *filename = argv[2];

    char *input = read_file(filename);
    size_t size = strlen(input);

    if (strcmp(cmd, "tokenize") == 0) {
        cmd_tokenize(input, size);
    } else if (strcmp(cmd, "parse") == 0) {
        cmd_parse(input, size);
    } else {
        fprintf(stderr, "bad command\n");
    }

    free(input);

    return EXIT_SUCCESS;
}
