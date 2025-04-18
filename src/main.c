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
        printf("  src excerpt: '%.*s'\n", (int) tok->len, tok->src);
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

int main(int argc, char **argv) {
    if (argc < 3) {
        print_help();

        return EXIT_FAILURE;
    }

    const char *cmd = argv[1];
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
