#pragma once

#include "lexer.h"
#include "vector.h"

#include <stdint.h>
#include <stdio.h>

typedef enum AstNodeKind {
    AST_NODE_ERROR,
    AST_NODE_INTEGER,
    AST_NODE_UNARY,
    AST_NODE_BINARY,
    AST_NODE_GROUPING
} AstNodeKind;

typedef struct AstNode {
    AstNodeKind kind;

    union {
        union {
            int64_t i;
        } literal;

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
    };
} AstNode;

AstNode *astnode_new(AstNodeKind kind);
void astnode_destroy(AstNode *self);

typedef struct Ast {
    Vector nodes; /* Vector<AstNode *> */
} Ast;

void ast_destroy(Ast *self);
void ast_dump(const Ast *self, FILE *out);
