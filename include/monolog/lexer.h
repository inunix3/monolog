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
    TOKEN_OP_MUL,
    TOKEN_OP_DIV,
    TOKEN_OP_MOD,
    TOKEN_OP_INC,
    TOKEN_OP_DEC,
    TOKEN_OP_EXCL,
    TOKEN_OP_AMP,
    TOKEN_OP_PIPE,
    TOKEN_OP_LESS,
    TOKEN_OP_GREATER,
    TOKEN_OP_ASSIGN,
    TOKEN_OP_EQUAL,
    TOKEN_OP_NOT_EQUAL,
    TOKEN_OP_LESS_EQUAL,
    TOKEN_OP_GREATER_EQUAL,
    TOKEN_OP_AND,
    TOKEN_OP_OR,
    TOKEN_OP_QUEST,
    TOKEN_OP_HASHTAG,
    TOKEN_OP_DOLAR,
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_FOR,
    TOKEN_KW_WHILE,
    TOKEN_KW_RETURN,
    TOKEN_KW_BREAK,
    TOKEN_KW_CONTINUE,
    TOKEN_KW_NIL,
    TOKEN_KW_INT,
    TOKEN_KW_VOID,
    TOKEN_KW_STRING
} TokenKind;

typedef struct Token {
    TokenKind kind;
    const char *src;
    size_t len;
    bool valid;
    size_t line;
    size_t col;
} Token;

typedef struct Lexer {
    const char *data;
    size_t next_ch_idx;
    size_t len;
    char ch;
    char prev_ch;
    char line;
    char col;
} Lexer;

void lexer_lex(const char *data, size_t len, Vector *tokens);
