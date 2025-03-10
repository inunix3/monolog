#include <monolog/lexer.h>

static bool at_eof(const Lexer *self) {
    return self->idx >= self->len;
}

static bool is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

static char advance(Lexer *self) {
    if (at_eof(self)) {
        return '\0';
    }

    self->prev_ch = self->ch;
    char ch = self->data[self->idx++];
    self->ch = ch;

    return ch;
}

static void push_token(Lexer *self, Vector *tokens) {
    vec_push(tokens, &self->token);
    memset(&self->token, 0, sizeof(self->token));
}

void lexer_lex(const char *data, size_t len, Vector *tokens) {
    Lexer lexer;

    for (;;) {
        char ch = advance(&lexer);

        if (at_eof(&lexer)) {
            break;
        }

        if (is_digit(ch)) {
            lex_int();

            continue;
        }
    }
}
