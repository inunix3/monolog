/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/lexer.h>

#include <assert.h>
#include <string.h>

static bool at_eof(const Lexer *self) {
    return self->ch == '\0' && self->idx >= self->len;
}

static bool is_digit(char ch) { return ch >= '0' && ch <= '9'; }

static bool is_ws(char ch) {
    switch (ch) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
        return true;
    default:
        return false;
    }
}

static bool is_operator(char ch) {
    switch (ch) {
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
    case '/':
    case '%':
    case '"':
    case '?':
    case '#':
    case '$':
        return true;
    default:
        return false;
    }
}

static bool is_alpha(char ch) {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

static bool is_identifier(char ch) {
    return is_alpha(ch) || ch == '_';
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

static void read_int(Lexer *self) {
    self->token.src = self->data + self->idx;

    while (!at_eof(self)) {
        ++self->token.len;

        advance(self);
    }
}

static void reset_token(Lexer *self, TokenKind kind) {
    self->token.kind = kind;

    if (self->idx > 0) {
        self->token.src = self->data + self->idx - 1;
    } else {
        self->token.src = self->data;
    }

    self->token.len = 0;
    self->token.valid = true;
}

void lexer_lex(const char *data, size_t len, Vector *tokens) {
    Lexer lexer;
    memset(&lexer, 0, sizeof(lexer));

    lexer.data = data;
    lexer.len = len;
    lexer.state = LEXER_STATE_FIND_DATA;

    advance(&lexer);

    for (;;) {
        char ch = lexer.ch;

        switch (lexer.state) {
        case LEXER_STATE_FIND_DATA:
            if (at_eof(&lexer)) {
                lexer.state = LEXER_STATE_EOF;

                break;
            } else if (is_digit(ch)) {
                reset_token(&lexer, TOKEN_INTEGER);

                lexer.state = LEXER_STATE_INTEGER;
            } else if (is_identifier(ch)) {
                reset_token(&lexer, TOKEN_IDENTIFIER);

                lexer.state = LEXER_STATE_IDENTIFIER;
            } else if (is_operator(ch)) {
                reset_token(&lexer, TOKEN_OPERATOR);
                ++lexer.token.len;
                push_token(&lexer, tokens);

                lexer.state = LEXER_STATE_FIND_DATA;

                break;
            } else if (!is_ws(ch)) {
                reset_token(&lexer, TOKEN_UNKNOWN);
                lexer.token.valid = false;

                lexer.state = LEXER_STATE_ERROR;
            }

            ++lexer.token.len;

            break;
        case LEXER_STATE_EOF:
            reset_token(&lexer, TOKEN_EOF);
            push_token(&lexer, tokens);

            return;
        case LEXER_STATE_ERROR:
            if (at_eof(&lexer)) {
                push_token(&lexer, tokens);
                lexer.state = LEXER_STATE_EOF;
            } else if (is_ws(ch)) {
                push_token(&lexer, tokens);
                lexer.state = LEXER_STATE_FIND_DATA;

                break;
            }

            ++lexer.token.len;

            break;
        case LEXER_STATE_INTEGER:
            if (at_eof(&lexer)) {
                push_token(&lexer, tokens);
                lexer.state = LEXER_STATE_EOF;

                break;
            } else if (is_ws(ch)) {
                push_token(&lexer, tokens);
                lexer.state = LEXER_STATE_FIND_DATA;

                break;
            } else if (!is_digit(ch)) {
                lexer.token.valid = false;
                lexer.state = LEXER_STATE_ERROR;

                break;
            }

            ++lexer.token.len;

            break;
        case LEXER_STATE_IDENTIFIER:
            if (at_eof(&lexer)) {
                push_token(&lexer, tokens);
                lexer.state = LEXER_STATE_EOF;

                break;
            } else if (is_ws(ch)) {
                push_token(&lexer, tokens);
                lexer.state = LEXER_STATE_FIND_DATA;

                break;
            } else if (is_operator(ch)) {
                push_token(&lexer, tokens);

                // reset_token(&lexer, TOKEN_OPERATOR);
                // ++lexer.token.len;
                // push_token(&lexer, tokens);

                lexer.state = LEXER_STATE_FIND_DATA;

                    continue;
            }

            ++lexer.token.len;

            break;
        }

        advance(&lexer);
    }
}
