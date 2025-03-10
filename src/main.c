#include <monolog/lexer.h>
#include <monolog/vector.h>

#include <stdio.h>
#include <string.h>

int main(void) {
    Vector tokens;

    vec_init(&tokens, sizeof(Token));

    const char *src = "123\t      \n1\n 45";
    lexer_lex(src, strlen(src), &tokens);

    for (size_t i = 0; i < tokens.len; ++i) {
        const Token *toks = tokens.data;
        const Token *tok = &toks[i];

        printf("id: %d\n", tok->kind);
        printf("len: %zu\n", tok->len);
        printf("src: \n");

        for (size_t j = 0; j < tok->len; ++j) {
            putchar(tok->src[j]);
        }

        putchar('\n');
    }

    vec_deinit(&tokens);

    return 0;
}
