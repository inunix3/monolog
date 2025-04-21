#pragma once

#include "lexer.h"
#include "strbuf.h"
#include "vector.h"

#include <stdint.h>
#include <stdio.h>

typedef enum AstNodeKind {
    AST_NODE_ERROR,
    AST_NODE_INTEGER,
    AST_NODE_STRING,
    AST_NODE_IDENT,
    AST_NODE_UNARY,
    AST_NODE_BINARY,
    AST_NODE_GROUPING,
    AST_NODE_BLOCK,
    AST_NODE_PRINT,
    AST_NODE_PRINTLN,
    AST_NODE_IF,
    AST_NODE_WHILE,
    AST_NODE_FOR,
} AstNodeKind;

typedef struct AstNode {
    AstNodeKind kind;

    union {
        union {
            int64_t i;
            StrBuf str;
        } literal;

        struct {
            StrBuf str;
        } ident;

        struct {
            TokenKind op;
            struct AstNode *right;
        } unary;

        struct {
            TokenKind op;
            struct AstNode *left;
            struct AstNode *right;
        } binary;

        struct {
            struct AstNode *expr;
        } grouping;

        struct {
            Vector nodes; /* Vector<AstNode *> */
        } block;

        struct {
            struct AstNode *expr;
        } kw_print;

        struct {
            struct AstNode *cond;
            struct AstNode *body;
            struct AstNode *else_body;
        } kw_if;

        struct {
            struct AstNode *cond;
            struct AstNode *body;
        } kw_while;

        struct {
            struct AstNode *init;
            struct AstNode *cond;
            struct AstNode *iter;
            struct AstNode *body;
        } kw_for;
    };
} AstNode;

AstNode *astnode_new(AstNodeKind kind);
void astnode_destroy(AstNode *self);

typedef struct Ast {
    Vector nodes; /* Vector<AstNode *> */
} Ast;

void ast_destroy(Ast *self);
void ast_dump(const Ast *self, FILE *out);
