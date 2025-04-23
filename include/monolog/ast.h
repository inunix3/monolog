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
    AST_NODE_FN_CALL,
    AST_NODE_BLOCK,
    AST_NODE_PRINT,
    AST_NODE_PRINTLN,
    AST_NODE_IF,
    AST_NODE_WHILE,
    AST_NODE_FOR,
    AST_NODE_INT_TYPE,
    AST_NODE_STRING_TYPE,
    AST_NODE_VOID_TYPE,
    AST_NODE_OPTION_TYPE,
    AST_NODE_ARRAY_TYPE,
    AST_NODE_VAR_DECL,
    AST_NODE_PARAM_DECL,
    AST_NODE_FN_DECL
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
            struct AstNode *name;
            Vector values; /* Vector<AstNode *> */
        } fn_call;

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

        struct {
            struct AstNode *type;
        } opt_type;

        struct {
            struct AstNode *type;
            struct AstNode *size;
        } array_type;

        struct {
            struct AstNode *type;
            struct AstNode *name;
            struct AstNode *rvalue;
        } var_decl;

        struct {
            struct AstNode *type;
            struct AstNode *name;
        } param_decl;

        struct {
            struct AstNode *type;
            struct AstNode *name;
            Vector params; /* Vector<AstNode *> */
            struct AstNode *body;
        } fn_decl;
    };
} AstNode;

AstNode *astnode_new(AstNodeKind kind);
void astnode_destroy(AstNode *self);

typedef struct Ast {
    Vector nodes; /* Vector<AstNode *> */
} Ast;

void ast_destroy(Ast *self);
void ast_dump(const Ast *self, FILE *out);
