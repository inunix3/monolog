#include <monolog/parser.h>
#include <monolog/strbuf.h>
#include <monolog/vector.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AstNode *integer_literal(Parser *self);
static AstNode *string_literal(Parser *self);
static AstNode *unary(Parser *self);
static AstNode *prefix(Parser *self, PrecedenceLevel prec);
static AstNode *binary(Parser *self, AstNode *left);
static AstNode *grouping(Parser *self);

static ParseRule g_rules[] = {
    [TOKEN_UNKNOWN] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
    [TOKEN_INTEGER] = {integer_literal, NULL, PREC_NONE},
    [TOKEN_IDENTIFIER] = {NULL, NULL, PREC_NONE},
    [TOKEN_STRING] = {string_literal, NULL, PREC_NONE},
    [TOKEN_OP_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_LPAREN] = {grouping, NULL, PREC_SUFFIX},
    [TOKEN_OP_RPAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_LBRACKET] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_RBRACKET] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_LBRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_RBRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_PLUS] = {unary, binary, PREC_ADD},
    [TOKEN_OP_MINUS] = {unary, binary, PREC_ADD},
    [TOKEN_OP_MUL] = {NULL, binary, PREC_MUL},
    [TOKEN_OP_DIV] = {NULL, binary, PREC_MUL},
    [TOKEN_OP_MOD] = {NULL, binary, PREC_MUL},
    [TOKEN_OP_INC] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_DEC] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_EXCL] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_AMP] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_PIPE] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_LESS] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_GREATER] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_ASSIGN] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_NOT_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_LESS_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_GREATER_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_AND] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_OR] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_QUEST] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_HASHTAG] = {NULL, NULL, PREC_NONE},
    [TOKEN_OP_DOLAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_BREAK] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_CONTINUE] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_NIL] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_INT] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_VOID] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_STRING] = {NULL, NULL, PREC_NONE}
};

static void error(Parser *self, const char *fmt, ...) {
    if (self->log_errors) {
        va_list vargs;

        fputs("error: ", stderr);

        va_start(vargs, fmt);
        vfprintf(stderr, fmt, vargs);
        va_end(vargs);

        fputc('\n', stderr);
    }

    self->error_state = true;
    self->panic_mode = true;
}

static void advance(Parser *self) {
    Token *tok = &self->toks[self->tok_idx];

    if (tok->kind != TOKEN_EOF && self->tok_idx < self->tok_count) {
        ++self->tok_idx;
    }

    self->prev = self->curr;
    self->curr = tok;
}

static Token *peek(const Parser *self) { return &self->toks[self->tok_idx]; }

static bool match(Parser *self, TokenKind kind) {
    if (self->curr->kind != kind) {
        return false;
    }

    advance(self);

    return true;
}

static bool expect(Parser *self, TokenKind kind) {
    if (!match(self, kind)) {
        error(
            self, "expected %s, got %s", token_kind_to_str(kind),
            token_kind_to_str(self->curr->kind)
        );

        return false;
    }

    return true;
}

static AstNode *expression(Parser *self, PrecedenceLevel prec);

static AstNode *integer_literal(Parser *self) {
    char buf[65] = {0};
    strncpy(buf, self->curr->src, self->curr->len);

    int64_t value = atoll(buf); /* FIXME: change to strtoll */

    AstNode *node = astnode_new(AST_NODE_INTEGER);
    node->literal.i = value;

    advance(self);

    return node;
}

static AstNode *string_literal(Parser *self) {
    if (!self->curr->valid) {
        error(self, "unterminated string");

        return astnode_new(AST_NODE_ERROR);
    }

    AstNode *node = astnode_new(AST_NODE_STRING);

    /* TODO: handle escaped characters */
    str_initn(&node->literal.str, self->curr->src + 1, self->curr->len - 2);

    advance(self);

    return node;
}

static AstNode *unary(Parser *self) {
    AstNode *node = astnode_new(AST_NODE_UNARY);

    ParseRule *op_rule = &g_rules[self->curr->kind];

    advance(self);

    node->unary.op = self->prev->kind;
    node->unary.right = expression(self, PREC_PREFIX);

    return node;
}

static AstNode *binary(Parser *self, AstNode *left) {
    AstNode *node = astnode_new(AST_NODE_BINARY);

    ParseRule *op_rule = &g_rules[self->curr->kind];

    advance(self);

    node->binary.op = self->prev->kind;
    node->binary.left = left;
    node->binary.right = expression(self, op_rule->prec);

    return node;
}

static AstNode *grouping(Parser *self) {
    AstNode *node = astnode_new(AST_NODE_GROUPING);

    advance(self);

    node->grouping.expr = expression(self, PREC_NONE);

    if (!expect(self, TOKEN_OP_RPAREN)) {
        astnode_destroy(node);

        return astnode_new(AST_NODE_ERROR);
    }

    return node;
}

static AstNode *prefix(Parser *self, PrecedenceLevel prec) {
    ParseRule *prefix_rule = &g_rules[self->curr->kind];

    if (!prefix_rule->prefix) {
        error(self, "unexpected token %s", token_kind_to_str(self->curr->kind));

        return astnode_new(AST_NODE_ERROR);
    }

    AstNode *left = prefix_rule->prefix(self);

    while (g_rules[self->curr->kind].prec > prec) {
        ParseRule *infix_rule = &g_rules[self->curr->kind];

        if (!infix_rule->infix) {
            break;
        }

        left = infix_rule->infix(self, left);
    }

    return left;
}

static AstNode *expression(Parser *self, PrecedenceLevel prec) {
    return prefix(self, prec);
}

static AstNode *statement(Parser *self) {
    /* Allow empty statements */
    if (self->curr->kind == TOKEN_OP_SEMICOLON) {
        advance(self);

        return NULL;
    }

    AstNode *expr = expression(self, PREC_NONE);

    if (self->curr->kind != TOKEN_EOF && !expect(self, TOKEN_OP_SEMICOLON)) {
        astnode_destroy(expr);

        return astnode_new(AST_NODE_ERROR);
    }

    return expr;
}

static void sync(Parser *self) {
    self->panic_mode = false;

    while (self->curr->kind != TOKEN_EOF) {
        if (self->prev && self->prev->kind == TOKEN_OP_SEMICOLON) {
            break;
        }

        advance(self);
    }
}

Parser parser_new(Token *toks, size_t tok_count) {
    Parser p = {0};

    p.toks = toks;
    p.tok_count = tok_count;
    p.tok_idx = 0;
    advance(&p);

    return p;
}

Ast parser_parse(Parser *self) {
    Ast ast;

    vec_init(&ast.nodes, sizeof(AstNode *));

    while (!match(self, TOKEN_EOF)) {
        AstNode *node = statement(self);

        if (node) {
            vec_push(&ast.nodes, &node);
        }

        sync(self);
    }

    return ast;
}
