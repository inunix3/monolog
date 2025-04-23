#pragma once

#include "ast.h"
#include "lexer.h"

#include <stdbool.h>
#include <stddef.h>

/* Precendence level in ascending order */
typedef enum PrecedenceLevel {
    PREC_NONE,       /* statements, declarations, literals */
    PREC_ASSIGN,     /* = */
    PREC_OR,         /* || */
    PREC_AND,        /* && */
    PREC_EQUALITY,   /* == != */
    PREC_INEQUALITY, /* < <= > >= */
    PREC_ADD,        /* + - */
    PREC_MUL,        /* * / % */
    PREC_PREFIX,     /* + - ! # $ * ++ -- */
    PREC_SUFFIX      /* ++ -- ? () [] */
} PrecedenceLevel;

typedef struct Parser {
    Token *toks;
    size_t tok_count;
    size_t tok_idx;

    Token *prev;
    Token *curr;

    /* Indicates if there was an error */
    bool error_state;
    /* Used for error recovery */
    bool panic_mode;
    bool log_errors;
} Parser;

Parser parser_new(Token *toks, size_t tok_count);
Ast parser_parse(Parser *self);

typedef AstNode *(*PrefixParseFn)(Parser *self);
typedef AstNode *(*InfixParseFn)(Parser *self, AstNode *left);
typedef AstNode *(*SuffixParseFn)(Parser *self, AstNode *left);

typedef struct ParseRule {
    PrefixParseFn prefix;
    InfixParseFn infix;
    SuffixParseFn suffix;
    PrecedenceLevel prec;
} ParseRule;
