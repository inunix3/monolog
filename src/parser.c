#include <monolog/parser.h>
#include <monolog/strbuf.h>
#include <monolog/vector.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AstNode *integer_literal(Parser *self);
static AstNode *string_literal(Parser *self);
static AstNode *identifier(Parser *self);
static AstNode *unary(Parser *self);
static AstNode *prefix(Parser *self, PrecedenceLevel prec);
static AstNode *binary(Parser *self, AstNode *left);
static AstNode *grouping(Parser *self);
static AstNode *block(Parser *self);
static AstNode *statement(Parser *self);
static AstNode *declaration(Parser *self);

static ParseRule g_rules[] = {
    [TOKEN_UNKNOWN] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
    [TOKEN_INTEGER] = {integer_literal, NULL, PREC_NONE},
    [TOKEN_IDENTIFIER] = {identifier, NULL, PREC_NONE},
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
    [TOKEN_OP_ASSIGN] = {NULL, binary, PREC_ASSIGN},
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
    [TOKEN_KW_STRING] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_KW_PRINTLN] = {NULL, NULL, PREC_NONE}
};

static void verror(Parser *self, const char *fmt, va_list vargs) {
    if (self->panic_mode) {
        return;
    }

    if (self->log_errors) {
        fputs("error: ", stderr);
        vfprintf(stderr, fmt, vargs);
        fputc('\n', stderr);
    }

    self->error_state = true;
    self->panic_mode = true;
}

static void error(Parser *self, const char *fmt, ...) {
    va_list vargs;

    va_start(vargs, fmt);
    verror(self, fmt, vargs);
    va_end(vargs);
}

static void error_at(Parser *self, int line, int col, const char *fmt, ...) {
    va_list vargs;

    va_start(vargs, fmt);
    verror(self, fmt, vargs);
    va_end(vargs);
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
            self, "expected %s but got %s", token_kind_to_str(kind),
            token_kind_to_str(self->curr->kind)
        );

        return false;
    }

    return true;
}

static void
skip_to(Parser *self, TokenKind safe_point1, TokenKind safe_point2) {
    while (self->curr->kind != TOKEN_EOF) {
        if (self->curr->kind == safe_point1 ||
            self->curr->kind == safe_point2) {
            break;
        }

        advance(self);
    }
}

static void
stop_after(Parser *self, TokenKind safe_point1, TokenKind safe_point2) {
    while (self->curr->kind != TOKEN_EOF) {
        if (self->prev && (self->prev->kind == safe_point1 ||
                           self->prev->kind == safe_point2)) {
            break;
        }

        advance(self);
    }
}

static void sync(Parser *self) {
    self->panic_mode = false;

    while (self->curr->kind != TOKEN_EOF) {
        if (self->prev && (self->prev->kind == TOKEN_OP_SEMICOLON ||
                           self->prev->kind == TOKEN_OP_RBRACE)) {
            break;
        }

        switch (self->curr->kind) {
        case TOKEN_KW_IF:
        case TOKEN_KW_FOR:
        case TOKEN_KW_WHILE:
        case TOKEN_KW_PRINT:
        case TOKEN_KW_PRINTLN:
        case TOKEN_OP_LBRACE:
            return;
        default:
            break;
        }

        advance(self);
    }
}

static AstNode *expression(Parser *self, PrecedenceLevel prec);

static AstNode *integer_literal(Parser *self) {
    if (!self->curr->valid) {
        error(self, "invalid integer literal");
        advance(self);
        sync(self);

        return astnode_new(AST_NODE_ERROR);
    }

    char buf[65] = {0};
    strncpy(buf, self->curr->src, self->curr->len);

    int64_t value = atoll(buf); /* FIXME: change to strtoll */

    AstNode *node = astnode_new(AST_NODE_INTEGER);
    node->literal.i = value;

    advance(self); /* consume the integer */

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

    advance(self); /* consume the string */

    return node;
}

static AstNode *identifier(Parser *self) {
    AstNode *node = astnode_new(AST_NODE_IDENT);
    str_initn(&node->ident.str, self->curr->src, self->curr->len);

    advance(self); /* consume the identifier */

    return node;
}

static AstNode *unary(Parser *self) {
    AstNode *node = astnode_new(AST_NODE_UNARY);

    ParseRule *op_rule = &g_rules[self->curr->kind];

    advance(self); /* consume the operator */

    node->unary.op = self->prev->kind;
    node->unary.right = expression(self, PREC_PREFIX);

    return node;
}

static AstNode *binary(Parser *self, AstNode *left) {
    advance(self); /* consume the operator */

    ParseRule *op_rule = &g_rules[self->prev->kind];

    AstNode *node = astnode_new(AST_NODE_BINARY);
    node->binary.op = self->prev->kind;
    node->binary.left = left;
    node->binary.right = expression(self, op_rule->prec);

    return node;
}

static AstNode *grouping(Parser *self) {
    AstNode *node = astnode_new(AST_NODE_GROUPING);

    advance(self); /* consume left paren */

    node->grouping.expr = expression(self, PREC_NONE);

    if (!expect(self, TOKEN_OP_RPAREN)) {
        astnode_destroy(node);

        return astnode_new(AST_NODE_ERROR);
    }

    return node;
}

static AstNode *block(Parser *self) {
    AstNode *block = astnode_new(AST_NODE_BLOCK);
    vec_init(&block->block.nodes, sizeof(AstNode *));

    for (;;) {
        if (self->curr->kind == TOKEN_OP_RBRACE) {
            break;
        } else if (self->curr->kind == TOKEN_EOF) {
            error(self, "unterminated block");
            astnode_destroy(block);

            return astnode_new(AST_NODE_ERROR);
        }

        AstNode *stmt = statement(self);

        if (stmt) {
            vec_push(&block->block.nodes, &stmt);
        }
    }

    advance(self); /* consume right brace */

    return block;
}

static AstNode *prefix(Parser *self, PrecedenceLevel prec) {
    ParseRule *prefix_rule = &g_rules[self->curr->kind];

    if (!prefix_rule->prefix) {
        error(self, "unexpected %.*s", self->curr->len, self->curr->src);
        advance(self);
        stop_after(self, TOKEN_OP_SEMICOLON, TOKEN_OP_RPAREN);

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

static AstNode *wrapped_expression(Parser *self) {
    AstNode *expr = NULL;

    if (!expect(self, TOKEN_OP_LPAREN)) {
        skip_to(self, TOKEN_OP_SEMICOLON, TOKEN_OP_LBRACE);

        return astnode_new(AST_NODE_ERROR);
    }

    expr = expression(self, PREC_NONE);

    if (expr->kind != AST_NODE_ERROR && !expect(self, TOKEN_OP_RPAREN)) {
        skip_to(self, TOKEN_OP_SEMICOLON, TOKEN_OP_LBRACE);

        astnode_destroy(expr);
        expr = astnode_new(AST_NODE_ERROR);
    }

    return expr;
}

static AstNode *if_statement(Parser *self) {
    advance(self); /* consume if keyword */

    AstNode *stmt = astnode_new(AST_NODE_IF);
    AstNode *cond = wrapped_expression(self);
    AstNode *body = statement(self);

    if (body && body->kind == AST_NODE_ERROR) {
        sync(self);
    }

    stmt->kw_if.cond = cond;
    stmt->kw_if.body = body;

    if (match(self, TOKEN_KW_ELSE)) {
        stmt->kw_if.else_body = statement(self);
    }

    return stmt;
}

static AstNode *while_statement(Parser *self) {
    advance(self); /* consume while keyword */

    AstNode *stmt = astnode_new(AST_NODE_WHILE);
    AstNode *cond = wrapped_expression(self);
    AstNode *body = statement(self);

    if (body && body->kind == AST_NODE_ERROR) {
        sync(self);
    }

    stmt->kw_while.cond = cond;
    stmt->kw_while.body = body;

    return stmt;
}

static AstNode *
optional_expression_with_delimiter(Parser *self, TokenKind delim) {
    AstNode *expr = NULL;

    if (self->curr->kind != delim) {
        expr = expression(self, PREC_NONE);
    } else {
        advance(self);
    }

    if (expr && expr->kind != AST_NODE_ERROR && !expect(self, delim)) {
        stop_after(self, TOKEN_OP_SEMICOLON, TOKEN_OP_LBRACE);

        astnode_destroy(expr);
        expr = astnode_new(AST_NODE_ERROR);
    }

    return expr;
}

static AstNode *for_statement(Parser *self) {
    advance(self);

    if (!expect(self, TOKEN_OP_LPAREN)) {
        sync(self);

        return astnode_new(AST_NODE_ERROR);
    }

    AstNode *init =
        optional_expression_with_delimiter(self, TOKEN_OP_SEMICOLON);
    AstNode *cond =
        optional_expression_with_delimiter(self, TOKEN_OP_SEMICOLON);
    AstNode *iter = optional_expression_with_delimiter(self, TOKEN_OP_RPAREN);
    AstNode *body = statement(self);

    if (body && body->kind == AST_NODE_ERROR) {
        sync(self);
    }

    AstNode *stmt = astnode_new(AST_NODE_FOR);
    stmt->kw_for.init = init;
    stmt->kw_for.cond = cond;
    stmt->kw_for.iter = iter;
    stmt->kw_for.body = body;

    return stmt;
}

static AstNode *print_statement(Parser *self) {
    advance(self); /* consume print keyword */

    expect(self, TOKEN_OP_LPAREN);
    AstNode *expr = expression(self, PREC_NONE);
    expect(self, TOKEN_OP_RPAREN);

    AstNode *stmt = astnode_new(AST_NODE_PRINT);
    stmt->kw_print.expr = expr;

    return stmt;
}

static AstNode *println_statement(Parser *self) {
    advance(self); /* consume println keyword */

    expect(self, TOKEN_OP_LPAREN);
    AstNode *expr = expression(self, PREC_NONE);
    expect(self, TOKEN_OP_RPAREN);

    AstNode *stmt = astnode_new(AST_NODE_PRINTLN);
    stmt->kw_print.expr = expr;

    return stmt;
}

static AstNode *statement(Parser *self) {
    AstNode *stmt = NULL;
    TokenKind stmt_kind = self->curr->kind;

    switch (stmt_kind) {
    case TOKEN_OP_SEMICOLON:
        /* Allow empty statements */
        advance(self);

        return NULL;
    case TOKEN_KW_IF:
        return if_statement(self);
    case TOKEN_KW_WHILE:
        return while_statement(self);
    case TOKEN_KW_FOR:
        return for_statement(self);
    case TOKEN_KW_PRINT:
        stmt = print_statement(self);

        break;
    case TOKEN_KW_PRINTLN:
        stmt = println_statement(self);

        break;
    case TOKEN_OP_LBRACE:
        advance(self);
        stmt = block(self);

        break;
    case TOKEN_KW_INT:
    case TOKEN_KW_STRING:
        stmt = declaration(self);

        break;
    default:
        stmt = expression(self, PREC_NONE);

        break;
    }

    if (stmt->kind != AST_NODE_ERROR && self->prev &&
        self->prev->kind != TOKEN_OP_RBRACE && self->curr->kind != TOKEN_EOF &&
        !expect(self, TOKEN_OP_SEMICOLON)) {
        astnode_destroy(stmt);

        return astnode_new(AST_NODE_ERROR);
    }

    return stmt;
}

static AstNode *declaration(Parser *self) {
    TokenKind type = self->curr->kind;

    advance(self);

    if (self->curr->kind != TOKEN_IDENTIFIER) {
        error(self, "expected identifier");
        sync(self);

        return astnode_new(AST_NODE_ERROR);
    }

    AstNode *name = identifier(self);
    AstNode *rvalue = NULL;

    if (self->curr->kind == TOKEN_OP_ASSIGN) {
        advance(self);

        rvalue = expression(self, PREC_NONE);
    } else if (self->curr->kind != TOKEN_OP_SEMICOLON) {
        error(self, "expected assignment or semicolon");
        rvalue = astnode_new(AST_NODE_ERROR);
    }

    if (rvalue && rvalue->kind == AST_NODE_ERROR) {
        skip_to(self, TOKEN_OP_SEMICOLON, TOKEN_OP_LBRACE);
    }

    AstNode *node = astnode_new(AST_NODE_VAR_DECL);

    node->var_decl.type = type;
    node->var_decl.name = name;
    node->var_decl.rvalue = rvalue;

    return node;
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

        if (self->panic_mode) {
            sync(self);
        }
    }

    return ast;
}
