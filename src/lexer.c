/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/lexer.h>

#include <assert.h>
#include <string.h>

#define ARRAY_SIZE(_a) (sizeof(_a) / (sizeof((_a)[0])))

static bool at_eof(const Lexer *self) {
    return self->ch == '\0' && self->next_ch_idx >= self->len;
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
    case '?':
    case '#':
    case '$':
        return true;
    default:
        return false;
    }
}

static TokenKind keyword_kind(const char *s, size_t len) {
    static const char *keywords[] = {"if",     "else",  "for",      "while",
                                     "return", "break", "continue", "nil",
                                     "int",    "void",  "string"};

    for (int i = 0; i < ARRAY_SIZE(keywords); ++i) {
        if (strncmp(s, keywords[i], len) == 0) {
            return TOKEN_KW_IF + i;
        }
    }

    return TOKEN_UNKNOWN;
}

static TokenKind char_op_kind(char ch) {
    static const TokenKind kinds[] = {
        [','] = TOKEN_OP_COMMA,    [';'] = TOKEN_OP_SEMICOLON,
        ['('] = TOKEN_OP_LPAREN,   [')'] = TOKEN_OP_RPAREN,
        ['['] = TOKEN_OP_LBRACKET, [']'] = TOKEN_OP_RBRACKET,
        ['{'] = TOKEN_OP_LBRACE,   ['}'] = TOKEN_OP_RBRACE,
        ['+'] = TOKEN_OP_PLUS,     ['-'] = TOKEN_OP_MINUS,
        ['*'] = TOKEN_OP_ASTERISK, ['/'] = TOKEN_OP_DIV,
        ['%'] = TOKEN_OP_MOD,      ['?'] = TOKEN_OP_QUESTION_MARK,
        ['#'] = TOKEN_OP_HASHTAG,  ['$'] = TOKEN_OP_DOLAR
    };

    return kinds[ch];
}

static bool is_alpha(char ch) {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

static bool is_identifier(char ch) { return is_alpha(ch) || ch == '_'; }

static Token new_token(const Lexer *self, TokenKind kind) {
    return (Token){kind, self->data + self->next_ch_idx - 1, 0, true};
}

static char advance(Lexer *self) {
    if (at_eof(self)) {
        return '\0';
    }

    self->prev_ch = self->ch;
    char ch = self->data[self->next_ch_idx++];
    self->ch = ch;

    return ch;
}

static void skip_ws(Lexer *self) {
    while (is_ws(self->ch)) {
        advance(self);
    }
}

static Token lex_invalid(Lexer *self) {
    Token tok = new_token(self, TOKEN_UNKNOWN);
    tok.valid = false;

    while (!at_eof(self) && !is_ws(self->ch)) {
        advance(self);
        ++tok.len;
    }

    return tok;
}

static Token lex_int(Lexer *self) {
    Token tok = new_token(self, TOKEN_INTEGER);

    while (!at_eof(self) && !is_ws(self->ch) && !is_operator(self->ch)) {
        if (!is_digit(self->ch)) {
            tok.valid = false;
        }

        advance(self);
        ++tok.len;
    }

    return tok;
}

static Token lex_identifier(Lexer *self) {
    Token tok = new_token(self, TOKEN_IDENTIFIER);

    while (!at_eof(self) && (is_identifier(self->ch) || is_digit(self->ch))) {
        advance(self);
        ++tok.len;
    }

    TokenKind kind = keyword_kind(tok.src, tok.len);

    if (kind != TOKEN_UNKNOWN) {
        tok.kind = kind;
    }

    return tok;
}

static Token lex_string(Lexer *self) {
    Token tok = new_token(self, TOKEN_STRING);

    /* eat the first quote */
    advance(self);
    ++tok.len;

    for (;;) {
        if (at_eof(self)) {
            tok.valid = false;

            return tok;
        }

        if (self->ch == '"' && self->prev_ch != '\\') {
            break;
        }

        advance(self);
        ++tok.len;
    }

    /* eat the last quote */
    advance(self);
    ++tok.len;

    return tok;
}

Token next_token(Lexer *self) {
    skip_ws(self);

    if (at_eof(self)) {
        return (Token){TOKEN_EOF, self->data + self->next_ch_idx - 1, 0, true};
    } else if (is_digit(self->ch)) {
        return lex_int(self);
    } else if (is_identifier(self->ch)) {
        return lex_identifier(self);
    } else if (is_operator(self->ch)) {
        Token tok = new_token(self, char_op_kind(self->ch));

        advance(self);
        ++tok.len;

        return tok;
    } else if (self->ch == '"') {
        return lex_string(self);
    }

    return lex_invalid(self);
}

void lexer_lex(const char *data, size_t len, Vector *tokens) {
    Lexer lexer;
    memset(&lexer, 0, sizeof(lexer));

    lexer.data = data;
    lexer.len = len;
    lexer.ch = *data;
    lexer.next_ch_idx = 1;

    for (;;) {
        Token tok = next_token(&lexer);

        vec_push(tokens, &tok);

        if (tok.kind == TOKEN_EOF) {
            break;
        }
    }
}
