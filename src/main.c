/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/lexer.h>
#include <monolog/vector.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static long file_size(FILE *file) {
    if (fseek(file, 0, SEEK_END) != 0) {
        return -1;
    }

    long pos = ftell(file);

    if (pos < 0) {
        return -1;
    }

    rewind(file);

    return pos;
}

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
        printf("  src excerpt: '");

        for (size_t j = 0; j < tok->len; ++j) {
            putchar(tok->src[j]);
        }

        puts("'\n");
    }

    vec_deinit(&tokens);
}

static void cmd_parse(char *buf, size_t size) {
    (void)buf;
    (void)size;
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

    FILE *file = fopen(filename, "r");

    if (!file) {
        fprintf(stderr, "cannot open '%s': %s\n", filename, strerror(errno));

        return EXIT_FAILURE;
    }

    long size = file_size(file);

    if (size < 0) {
        fprintf(stderr, "cannot get file size: %s\n", strerror(errno));
        fclose(file);

        return EXIT_FAILURE;
    }

    char *buf = malloc(size + 1);

    if (!buf) {
        fprintf(stderr, "cannot allocate buffer: %s\n", strerror(errno));
        fclose(file);

        return EXIT_FAILURE;
    }

    fread(buf, sizeof(*buf), size, file);
    buf[size] = 0;

    if (strcmp(cmd, "tokenize") == 0) {
        cmd_tokenize(buf, size);
    } else if (strcmp(cmd, "parse") == 0) {
        cmd_parse(buf, size);
    } else {
        fprintf(stderr, "bad command\n");
    }

    free(buf);
    fclose(file);

    return EXIT_SUCCESS;
}
