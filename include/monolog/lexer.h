/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

#include "vector.h"

#include <stdbool.h>
#include <stddef.h>

typedef enum TokenKind {
    TOKEN_UNKNOWN,
    TOKEN_EOF,
    TOKEN_INTEGER,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_OP_COMMA,
    TOKEN_OP_SEMICOLON,
    TOKEN_OP_LPAREN,
    TOKEN_OP_RPAREN,
    TOKEN_OP_LBRACKET,
    TOKEN_OP_RBRACKET,
    TOKEN_OP_LBRACE,
    TOKEN_OP_RBRACE,
    TOKEN_OP_PLUS,
    TOKEN_OP_MINUS,
    TOKEN_OP_ASTERISK,
    TOKEN_OP_DIV,
    TOKEN_OP_MOD,
    TOKEN_OP_QUESTION_MARK,
    TOKEN_OP_HASHTAG,
    TOKEN_OP_DOLAR
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
    LEXER_STATE_INTEGER,
    LEXER_STATE_IDENTIFIER,
    LEXER_STATE_STRING,
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
