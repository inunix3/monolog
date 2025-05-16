/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#pragma once

#include "src_info.h"
#include "vector.h"

#include <stdbool.h>
#include <stddef.h>

typedef enum TokenKind {
    TOKEN_UNKNOWN,
    TOKEN_EOF,
    TOKEN_INTEGER,
    TOKEN_IDENTIFIER,
    TOKEN_STRING_LIT,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_MOD,
    TOKEN_INC,
    TOKEN_DEC,
    TOKEN_EXCL,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_ASSIGN,
    TOKEN_ADD_ASSIGN,
    TOKEN_SUB_ASSIGN,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_EQUAL,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_QUEST,
    TOKEN_HASHTAG,
    TOKEN_DOLAR,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_NIL,
    TOKEN_INT,
    TOKEN_VOID,
    TOKEN_STRING
} TokenKind;

const char *token_kind_to_str(TokenKind kind);
const char *token_kind_to_name(TokenKind kind);

typedef struct Token {
    TokenKind kind;
    const char *src;
    size_t len;
    bool valid;
    SourceInfo src_info;
} Token;

typedef struct Lexer {
    const char *data;
    size_t next_ch_idx;
    size_t len;
    char ch;
    char prev_ch;
    int line;
    int col;
} Lexer;

void lexer_lex(const char *data, size_t len, Vector *tokens);
