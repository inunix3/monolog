/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/lexer.h>
#include <monolog/vector.h>

#include <stdio.h>
#include <string.h>

int main(void) {
    Vector tokens;

    vec_init(&tokens, sizeof(Token));

    const char *input = "int x = 5;\nprintf(\"%d\\n\", x);";
    lexer_lex(input, strlen(input), &tokens);

    for (size_t i = 0; i < tokens.len; ++i) {
        const Token *toks = tokens.data;
        const Token *tok = &toks[i];

        printf("Token %zu:\n", i + 1);
        printf("  kind: %d\n", tok->kind);
        printf("  len: %zu\n", tok->len);
        printf("  src: '");

        for (size_t j = 0; j < tok->len; ++j) {
            putchar(tok->src[j]);
        }

        puts("'\n");
    }

    vec_deinit(&tokens);

    return 0;
}
