#pragma once

#include "vector.h"

#include <stdbool.h>
#include <stddef.h>

typedef enum TokenKind {
    TOKEN_UNKNOWN,
    TOKEN_EOF,
    TOKEN_INTEGER,
} TokenKind;

typedef struct Token {
    TokenKind kind;
    const char *src;
    size_t len;
    bool valid;
} Token;

typedef enum LexerState {
    LEXER_STATE_FIND_DATA,
    LEXER_STATE_EOF,
    LEXER_STATE_ERROR,
    LEXER_STATE_INTEGER
} LexerState;

typedef struct Lexer {
    const char *data;
    size_t idx;
    size_t len;
    char ch;
    char prev_ch;
    LexerState state;
    LexerState prev_state;
    Token token;
} Lexer;

void lexer_lex(const char *data, size_t len, Vector *tokens);
