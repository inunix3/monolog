/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/parser.h>
#include <monolog/strbuf.h>
#include <monolog/utils.h>
#include <monolog/vector.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AstNode *integer_literal(Parser *self);
static AstNode *string_literal(Parser *self);
static AstNode *identifier(Parser *self);
static AstNode *nil_constant(Parser *self);
static AstNode *unary(Parser *self);
static AstNode *prefix(Parser *self, PrecedenceLevel prec);
static AstNode *binary(Parser *self, AstNode *left);
static AstNode *suffix(Parser *self, AstNode *left);
static AstNode *grouping(Parser *self);
static AstNode *fn_call(Parser *self, AstNode *left);
static AstNode *subscript(Parser *self, AstNode *left);
static AstNode *block(Parser *self);
static AstNode *statement(Parser *self);
static AstNode *declaration(Parser *self);

/* clang-format off */
static ParseRule g_rules[] = {
    [TOKEN_UNKNOWN]       = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_EOF]           = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_INTEGER]       = {integer_literal, NULL,    NULL,      PREC_NONE},
    [TOKEN_IDENTIFIER]    = {identifier,      NULL,    NULL,      PREC_NONE},
    [TOKEN_STRING_LIT]    = {string_literal,  NULL,    NULL,      PREC_NONE},
    [TOKEN_COMMA]         = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_SEMICOLON]     = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_LPAREN]        = {grouping,        fn_call, NULL,      PREC_SUFFIX},
    [TOKEN_RPAREN]        = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_LBRACKET]      = {NULL,            NULL,    subscript, PREC_SUFFIX},
    [TOKEN_RBRACKET]      = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_LBRACE]        = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_RBRACE]        = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_PLUS]          = {unary,           binary,  NULL,      PREC_ADD},
    [TOKEN_MINUS]         = {unary,           binary,  NULL,      PREC_ADD},
    [TOKEN_MUL]           = {unary,           binary,  NULL,      PREC_MUL},
    [TOKEN_DIV]           = {NULL,            binary,  NULL,      PREC_MUL},
    [TOKEN_MOD]           = {NULL,            binary,  NULL,      PREC_MUL},
    [TOKEN_INC]           = {unary,           NULL,    suffix,    PREC_SUFFIX},
    [TOKEN_DEC]           = {unary,           NULL,    suffix,    PREC_SUFFIX},
    [TOKEN_EXCL]          = {unary,           NULL,    NULL,      PREC_PREFIX},
    [TOKEN_LESS]          = {NULL,            binary,  NULL,      PREC_INEQUALITY},
    [TOKEN_GREATER]       = {NULL,            binary,  NULL,      PREC_INEQUALITY},
    [TOKEN_ASSIGN]        = {NULL,            binary,  NULL,      PREC_ASSIGN},
    [TOKEN_ADD_ASSIGN]    = {NULL,            binary,  NULL,      PREC_ASSIGN},
    [TOKEN_SUB_ASSIGN]    = {NULL,            binary,  NULL,      PREC_ASSIGN},
    [TOKEN_EQUAL]         = {NULL,            binary,  NULL,      PREC_EQUALITY},
    [TOKEN_NOT_EQUAL]     = {NULL,            binary,  NULL,      PREC_EQUALITY},
    [TOKEN_LESS_EQUAL]    = {NULL,            binary,  NULL,      PREC_INEQUALITY},
    [TOKEN_GREATER_EQUAL] = {NULL,            binary,  NULL,      PREC_INEQUALITY},
    [TOKEN_AND]           = {NULL,            binary,  NULL,      PREC_AND},
    [TOKEN_OR]            = {NULL,            binary,  NULL,      PREC_OR},
    [TOKEN_QUEST]         = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_HASHTAG]       = {unary,           NULL,    NULL,      PREC_PREFIX},
    [TOKEN_DOLAR]         = {unary,           NULL,    NULL,      PREC_PREFIX},
    [TOKEN_IF]            = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_ELSE]          = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_FOR]           = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_WHILE]         = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_RETURN]        = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_BREAK]         = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_CONTINUE]      = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_NIL]           = {nil_constant,    NULL,    NULL,      PREC_NONE},
    [TOKEN_INT]           = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_VOID]          = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_STRING]        = {NULL,            NULL,    NULL,      PREC_NONE}
};
/* clang-format on */

static void error(Parser *self, const char *fmt, ...) {
    if (self->panic_mode) {
        return;
    }

    if (self->log_errors) {
        fputs("error: ", stderr);

        va_list vargs;
        va_start(vargs, fmt);
        vfprintf(stderr, fmt, vargs);
        va_end(vargs);

        fputc('\n', stderr);
    }

    self->had_error = true;
    self->panic_mode = true;
}

static void error_at(Parser *self, const char *fmt, ...) {
    if (self->panic_mode) {
        return;
    }

    if (self->log_errors) {
        const SourceInfo *src_info = &self->curr->src_info;

        fprintf(stderr, "%d:%d: error: ", src_info->line, src_info->col);

        va_list vargs;
        va_start(vargs, fmt);
        vfprintf(stderr, fmt, vargs);
        va_end(vargs);

        fputc('\n', stderr);
    }

    self->had_error = true;
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

static inline bool match(const Parser *self, TokenKind kind) {
    return self->curr->kind == kind;
}

static inline bool match_prev(const Parser *self, TokenKind kind) {
    return self->prev && self->prev->kind == kind;
}

static bool expect(Parser *self, TokenKind kind) {
    if (!match(self, kind)) {
        error_at(
            self, "expected %s, but got %s", token_kind_to_str(kind),
            token_kind_to_str(self->curr->kind)
        );

        return false;
    }

    advance(self);

    return true;
}

typedef enum SyncMode {
    SYNC_TO_SEMICOLON_KEEP = (1u << 0),
    SYNC_TO_SEMICOLON_SKIP = (1u << 1),
    SYNC_TO_RBRACE_KEEP = (1u << 2),
    SYNC_TO_RBRACE_SKIP = (1u << 3),
    SYNC_TO_RPAREN_KEEP = (1u << 4),
    SYNC_TO_RPAREN_SKIP = (1u << 5),
    SYNC_TO_RBRACKET_KEEP = (1u << 6),
    SYNC_TO_RBRACKET_SKIP = (1u << 7),
    SYNC_TO_COMMA_KEEP = (1u << 8),
    SYNC_TO_BLOCK = (1u << 9),
    SYNC_TO_STATEMENT = (1u << 10),
} SyncMode;

static void sync(Parser *self, unsigned modes) {
    self->panic_mode = false;

    while (!match(self, TOKEN_EOF)) {
        if (modes & SYNC_TO_SEMICOLON_KEEP && match(self, TOKEN_SEMICOLON)) {
            break;
        }

        if (modes & SYNC_TO_SEMICOLON_SKIP &&
            match_prev(self, TOKEN_SEMICOLON)) {
            break;
        }

        if (modes & SYNC_TO_RBRACE_SKIP && match(self, TOKEN_RBRACE)) {
            break;
        }

        if (modes & SYNC_TO_RBRACE_SKIP && match_prev(self, TOKEN_RBRACE)) {
            break;
        }

        if (modes & SYNC_TO_RPAREN_KEEP && match(self, TOKEN_RPAREN)) {
            break;
        }

        if (modes & SYNC_TO_RPAREN_SKIP && match_prev(self, TOKEN_RPAREN)) {
            break;
        }

        if (modes & SYNC_TO_RBRACKET_KEEP && match(self, TOKEN_RBRACKET)) {
            break;
        }

        if (modes & SYNC_TO_RBRACKET_SKIP && match(self, TOKEN_RBRACKET)) {
            break;
        }

        if (modes & SYNC_TO_COMMA_KEEP && match(self, TOKEN_COMMA)) {
            break;
        }

        if (modes & SYNC_TO_BLOCK && match(self, TOKEN_LBRACE)) {
            break;
        }

        if (modes & SYNC_TO_STATEMENT) {
            switch (self->curr->kind) {
            case TOKEN_IF:
            case TOKEN_FOR:
            case TOKEN_WHILE:
            case TOKEN_INT:
            case TOKEN_STRING:
            case TOKEN_VOID:
            case TOKEN_RETURN:
            case TOKEN_BREAK:
            case TOKEN_CONTINUE:
            case TOKEN_LBRACKET:
                return;
            default:
                break;
            }
        }

        advance(self);
    }
}

static AstNode *expression(Parser *self, PrecedenceLevel prec);

static AstNode *integer_literal(Parser *self) {
    if (!self->curr->valid) {
        error_at(self, "invalid integer literal");
        advance(self);

        return astnode_new(AST_NODE_ERROR, self->curr);
    }

    char buf[65] = {0};
    strncpy(buf, self->curr->src, self->curr->len);

    AstNode *node = NULL;
    int64_t value;

    if (!str_to_i64(buf, &value)) {
        /* This message is more appropriate, because lexer can detect if the
         * integer is invalid, but not if it's overflowing.
         */
        error_at(self, "integer is too big to fit");
        node = astnode_new(AST_NODE_ERROR, self->curr);
    } else {
        node = astnode_new(AST_NODE_INTEGER, self->curr);
        node->literal.i = value;
    }

    advance(self); /* consume the integer */

    return node;
}

static AstNode *string_literal(Parser *self) {
    if (!self->curr->valid) {
        error_at(self, "unterminated string");

        return astnode_new(AST_NODE_ERROR, self->curr);
    }

    AstNode *node = astnode_new(AST_NODE_STRING, self->curr);

    str_dup_n(&node->literal.str, self->curr->src + 1, self->curr->len - 2);

    advance(self); /* consume the string */

    return node;
}

static AstNode *identifier(Parser *self) {
    AstNode *node = astnode_new(AST_NODE_IDENT, self->curr);
    str_dup_n(&node->ident.str, self->curr->src, self->curr->len);

    advance(self); /* consume the identifier */

    return node;
}

static AstNode *nil_constant(Parser *self) {
    advance(self); /* consume the nil keyword */

    return astnode_new(AST_NODE_NIL, self->prev);
}

static AstNode *unary(Parser *self) {
    AstNode *node = astnode_new(AST_NODE_UNARY, self->curr);

    advance(self); /* consume the operator */

    node->unary.op = *self->prev;
    node->unary.right = expression(self, PREC_PREFIX);

    return node;
}

static AstNode *binary(Parser *self, AstNode *left) {
    advance(self); /* consume the operator */

    ParseRule *op_rule = &g_rules[self->prev->kind];

    AstNode *node = astnode_new(AST_NODE_BINARY, self->prev);
    node->binary.op = *self->prev;
    node->binary.left = left;
    node->binary.right = expression(self, op_rule->prec);

    return node;
}

static AstNode *suffix(Parser *self, AstNode *left) {
    AstNode *node = astnode_new(AST_NODE_SUFFIX, self->prev);

    advance(self); /* consume the operator */

    node->suffix.op = *self->prev;
    node->suffix.left = left;

    return node;
}

static AstNode *grouping(Parser *self) {
    AstNode *node = astnode_new(AST_NODE_GROUPING, self->curr);

    advance(self); /* consume left paren */

    node->grouping.expr = expression(self, PREC_NONE);

    if (!expect(self, TOKEN_RPAREN)) {
        astnode_destroy(node);

        return astnode_new(AST_NODE_ERROR, self->curr);
    }

    return node;
}

static bool
expr_list(Parser *self, Vector *values, AstNode *(*expr_fn)(Parser *self)) {
    if (match(self, TOKEN_RPAREN)) {
        advance(self);

        return true;
    }

    AstNode *expr = NULL;

    while (!match(self, TOKEN_EOF) && !match(self, TOKEN_RPAREN)) {
        expr = expr_fn(self);
        vec_push(values, &expr);

        if (expr->kind == AST_NODE_ERROR) {
            sync(self, SYNC_TO_COMMA_KEEP | SYNC_TO_RPAREN_KEEP);

            if (match(self, TOKEN_COMMA)) {
                advance(self);
            }
        } else {
            if (match(self, TOKEN_COMMA)) {
                advance(self);
            } else if (!match(self, TOKEN_RPAREN)) {
                AstNode *error_node = astnode_new(AST_NODE_ERROR, self->curr);
                vec_push(values, &error_node);

                error_at(self, "expected , or )");

                sync(self, SYNC_TO_COMMA_KEEP | SYNC_TO_RPAREN_KEEP);

                if (match(self, TOKEN_COMMA)) {
                    advance(self);
                }
            }
        }
    }

    if (!expect(self, TOKEN_RPAREN)) {
        return false;
    }

    return true;
}

static AstNode *value_item(Parser *self) { return expression(self, PREC_NONE); }

static bool value_list(Parser *self, Vector *values) {
    return expr_list(self, values, value_item);
}

static AstNode *fn_call(Parser *self, AstNode *left) {
    Token tok = *self->curr;
    advance(self); /* consume the ( */

    AstNode *name = left;

    if (name->kind != AST_NODE_IDENT) {
        error_at(self, "expected identifier");

        name = astnode_new(AST_NODE_ERROR, &left->tok);
    }

    Vector values;
    vec_init(&values, sizeof(AstNode *));

    if (!value_list(self, &values)) {
        sync(
            self, SYNC_TO_SEMICOLON_KEEP | SYNC_TO_RBRACKET_KEEP |
                      SYNC_TO_BLOCK | SYNC_TO_STATEMENT
        );
    }

    AstNode *node = astnode_new(AST_NODE_FN_CALL, &tok);
    node->fn_call.name = name;
    node->fn_call.values = values;

    return node;
}

static AstNode *subscript(Parser *self, AstNode *left) {
    Token tok = *self->curr;
    advance(self); /* consume the [ */

    AstNode *expr = expression(self, PREC_NONE);

    if (!expect(self, TOKEN_RBRACKET)) {
        astnode_destroy(expr);
        astnode_destroy(left);

        return astnode_new(AST_NODE_ERROR, self->curr);
    }

    AstNode *node = astnode_new(AST_NODE_SUBSCRIPT, &tok);
    node->subscript.expr = expr;
    node->subscript.left = left;

    return node;
}

static AstNode *block(Parser *self) {
    AstNode *block = astnode_new(AST_NODE_BLOCK, self->curr);
    vec_init(&block->block.nodes, sizeof(AstNode *));

    for (;;) {
        if (match(self, TOKEN_RBRACE)) {
            break;
        } else if (match(self, TOKEN_EOF)) {
            error_at(self, "unterminated block");
            astnode_destroy(block);

            return astnode_new(AST_NODE_ERROR, self->curr);
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
        if (self->curr->kind == TOKEN_EOF) {
            error(self, "unexpected EOF");
        } else {
            error_at(self, "unexpected %.*s", self->curr->len, self->curr->src);
        }

        Token tok = *self->curr;

        advance(self);
        sync(
            self, SYNC_TO_RBRACKET_KEEP | SYNC_TO_RBRACE_KEEP |
                      SYNC_TO_SEMICOLON_KEEP | SYNC_TO_RPAREN_KEEP
        );

        return astnode_new(AST_NODE_ERROR, &tok);
    }

    AstNode *left = prefix_rule->prefix(self);
    ParseRule *suffix_rule = &g_rules[self->curr->kind];

    while (suffix_rule->suffix && suffix_rule->prec == PREC_SUFFIX) {
        left = suffix_rule->suffix(self, left);

        suffix_rule = &g_rules[self->curr->kind];
    }

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

    if (!expect(self, TOKEN_LPAREN)) {
        return astnode_new(AST_NODE_ERROR, self->curr);
    }

    expr = expression(self, PREC_NONE);

    if (expr->kind != AST_NODE_ERROR && !expect(self, TOKEN_RPAREN)) {
        astnode_destroy(expr);
        expr = astnode_new(AST_NODE_ERROR, self->curr);
    }

    return expr;
}

static AstNode *if_statement(Parser *self) {
    advance(self); /* consume if keyword */

    AstNode *stmt = astnode_new(AST_NODE_IF, self->prev);
    AstNode *cond = wrapped_expression(self);

    if (cond->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_SEMICOLON_KEEP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
    }

    AstNode *body = statement(self);

    if (body && body->kind == AST_NODE_ERROR) {
        sync(
            self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_RBRACE_SKIP | SYNC_TO_BLOCK |
                      SYNC_TO_STATEMENT
        );
    }

    stmt->kw_if.cond = cond;
    stmt->kw_if.body = body;

    if (match(self, TOKEN_ELSE)) {
        advance(self);

        stmt->kw_if.else_body = statement(self);
    }

    return stmt;
}

static AstNode *while_statement(Parser *self) {
    advance(self); /* consume while keyword */

    AstNode *stmt = astnode_new(AST_NODE_WHILE, self->prev);
    AstNode *cond = wrapped_expression(self);

    if (cond->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_SEMICOLON_KEEP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
    }

    AstNode *body = statement(self);

    if (body && body->kind == AST_NODE_ERROR) {
        sync(
            self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_RBRACE_SKIP | SYNC_TO_BLOCK |
                      SYNC_TO_STATEMENT
        );
    }

    stmt->kw_while.cond = cond;
    stmt->kw_while.body = body;

    return stmt;
}

static AstNode *
optional_var_decl_with_delimiter(Parser *self, TokenKind delim) {
    AstNode *decl = NULL;
    Token *tok = self->curr;

    if (!match(self, delim)) {
        decl = declaration(self);
    } else {
        advance(self);
    }

    if (decl && decl->kind != AST_NODE_ERROR &&
        decl->kind != AST_NODE_VAR_DECL) {
        error_at(self, "function declaration is not allowed in for loop");

        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);

        astnode_destroy(decl);
        decl = astnode_new(AST_NODE_ERROR, tok);
    } else if (decl && decl->kind != AST_NODE_ERROR && !expect(self, delim)) {
        tok = self->curr;

        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
        astnode_destroy(decl);
        decl = astnode_new(AST_NODE_ERROR, tok);
    }

    return decl;
}

static AstNode *
optional_expression_with_delimiter(Parser *self, TokenKind delim) {
    AstNode *expr = NULL;

    if (!match(self, delim)) {
        expr = expression(self, PREC_NONE);
    } else {
        advance(self);
    }

    if (expr && expr->kind != AST_NODE_ERROR && !expect(self, delim)) {
        Token tok = *self->curr;

        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
        astnode_destroy(expr);
        expr = astnode_new(AST_NODE_ERROR, &tok);
    }

    return expr;
}

static bool token_is_type(TokenKind tok) {
    switch (tok) {
    case TOKEN_INT:
    case TOKEN_STRING:
    case TOKEN_VOID:
    case TOKEN_LBRACKET:
        return true;
    default:
        return false;
    }
}

static AstNode *for_statement(Parser *self) {
    Token for_tok = *self->curr;
    advance(self);

    if (!expect(self, TOKEN_LPAREN)) {
        Token tok = *self->curr;
        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);

        return astnode_new(AST_NODE_ERROR, &tok);
    }

    AstNode *init = NULL;

    if (token_is_type(self->curr->kind)) {
        init = optional_var_decl_with_delimiter(self, TOKEN_SEMICOLON);
    } else {
        init = optional_expression_with_delimiter(self, TOKEN_SEMICOLON);
    }

    AstNode *cond = optional_expression_with_delimiter(self, TOKEN_SEMICOLON);
    AstNode *iter = optional_expression_with_delimiter(self, TOKEN_RPAREN);
    AstNode *body = statement(self);

    if (body && body->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
    }

    AstNode *stmt = astnode_new(AST_NODE_FOR, &for_tok);
    stmt->kw_for.init = init;
    stmt->kw_for.cond = cond;
    stmt->kw_for.iter = iter;
    stmt->kw_for.body = body;

    return stmt;
}

static AstNode *return_statement(Parser *self) {
    Token ret_tok = *self->curr;
    advance(self); /* consume the return keyword */

    AstNode *expr = NULL;

    if (self->curr->kind != TOKEN_SEMICOLON) {
        expr = expression(self, PREC_NONE);
    }

    if (expr && expr->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
    }

    AstNode *node = astnode_new(AST_NODE_RETURN, &ret_tok);
    node->kw_return.expr = expr;

    return node;
}

static AstNode *statement(Parser *self) {
    AstNode *stmt = NULL;
    TokenKind stmt_kind = self->curr->kind;

    switch (stmt_kind) {
    case TOKEN_SEMICOLON:
        /* Allow empty statements */
        advance(self);

        return NULL;
    case TOKEN_IF:
        return if_statement(self);
    case TOKEN_WHILE:
        return while_statement(self);
    case TOKEN_FOR:
        return for_statement(self);
    case TOKEN_LBRACE:
        advance(self);
        stmt = block(self);

        break;
    case TOKEN_INT:
    case TOKEN_STRING:
    case TOKEN_VOID:
    case TOKEN_LBRACKET:
        stmt = declaration(self);

        break;
    case TOKEN_RETURN:
        stmt = return_statement(self);

        break;
    case TOKEN_BREAK:
        advance(self);

        stmt = astnode_new(AST_NODE_BREAK, self->prev);

        break;
    case TOKEN_CONTINUE:
        advance(self);

        stmt = astnode_new(AST_NODE_CONTINUE, self->prev);

        break;
    default:
        stmt = expression(self, PREC_NONE);

        break;
    }

    if (stmt->kind != AST_NODE_ERROR && stmt->kind != AST_NODE_FN_DECL &&
        !match_prev(self, TOKEN_RBRACE) && !match(self, TOKEN_EOF) &&
        !expect(self, TOKEN_SEMICOLON)) {
        astnode_destroy(stmt);

        return astnode_new(AST_NODE_ERROR, self->curr);
    }

    return stmt;
}

static AstNode *parse_type(Parser *self) {
    AstNodeKind type_id;

    switch (self->curr->kind) {
    case TOKEN_INT:
        type_id = AST_NODE_INT_TYPE;

        break;
    case TOKEN_STRING:
        type_id = AST_NODE_STRING_TYPE;

        break;
    case TOKEN_VOID:
        type_id = AST_NODE_VOID_TYPE;

        break;
    case TOKEN_LBRACKET:
        type_id = AST_NODE_LIST_TYPE;

        break;
    default:
        type_id = AST_NODE_ERROR;

        break;
    }

    advance(self);

    AstNode *node = NULL;
    Token type_tok = *self->prev;

    if (type_id == AST_NODE_ERROR) {
        error_at(self, "expected type specifier: int, string, void or list");

        return astnode_new(AST_NODE_ERROR, self->curr);
    } else if (type_id == AST_NODE_LIST_TYPE) {
        AstNode *type = parse_type(self);

        if (type->kind == AST_NODE_ERROR) {
            sync(self, SYNC_TO_RBRACKET_KEEP);
        }

        AstNode *size = NULL;

        if (match(self, TOKEN_COMMA)) {
            advance(self);

            size = expression(self, PREC_NONE);

            if (size->kind == AST_NODE_ERROR) {
                sync(self, SYNC_TO_RBRACKET_KEEP);
            }
        }

        if (!expect(self, TOKEN_RBRACKET)) {
            sync(self, SYNC_TO_RBRACKET_SKIP);
        }

        node = astnode_new(type_id, &type_tok);
        node->list_type.type = type;
        node->list_type.size = size;
    } else {
        node = astnode_new(type_id, &type_tok);
    }

    while (match(self, TOKEN_QUEST)) {
        advance(self);

        AstNode *parent_node = astnode_new(AST_NODE_OPTION_TYPE, &type_tok);
        parent_node->opt_type.type = node;

        node = parent_node;
    }

    return node;
}

static AstNode *param_decl(Parser *self) {
    Token type_tok = *self->curr;
    AstNode *type = parse_type(self);

    if (type->kind == AST_NODE_ERROR) {
        return type;
    }

    if (!match(self, TOKEN_IDENTIFIER)) {
        error_at(self, "expected identifier");
        astnode_destroy(type);

        return astnode_new(AST_NODE_ERROR, self->curr);
    }

    AstNode *name = identifier(self);

    AstNode *node = astnode_new(AST_NODE_PARAM_DECL, &type_tok);
    node->param_decl.type = type;
    node->param_decl.name = name;

    return node;
}

static bool fn_decl_param_list(Parser *self, Vector *params) {
    return expr_list(self, params, param_decl);
}

static AstNode *
var_decl(Parser *self, const Token *tok, AstNode *type, AstNode *name) {
    AstNode *rvalue = NULL;

    if (match(self, TOKEN_ASSIGN)) {
        advance(self);

        rvalue = expression(self, PREC_NONE);
    }

    if (rvalue && rvalue->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_SEMICOLON_KEEP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
    }

    AstNode *node = astnode_new(AST_NODE_VAR_DECL, tok);

    node->var_decl.type = type;
    node->var_decl.name = name;
    node->var_decl.rvalue = rvalue;

    return node;
}

static AstNode *
fn_decl(Parser *self, const Token *tok, AstNode *type, AstNode *name) {
    Vector params;
    vec_init(&params, sizeof(AstNode *));

    if (!fn_decl_param_list(self, &params)) {
        sync(self, SYNC_TO_SEMICOLON_KEEP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
    }

    Token body_tok = *self->curr;
    AstNode *body = statement(self);

    if (!body) {
        error_at(self, "expected statement or block");

        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);

        body = astnode_new(AST_NODE_ERROR, &body_tok);
    } else if (body->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
    }

    AstNode *node = astnode_new(AST_NODE_FN_DECL, tok);

    node->fn_decl.type = type;
    node->fn_decl.name = name;
    node->fn_decl.params = params;
    node->fn_decl.body = body;

    return node;
}

static AstNode *declaration(Parser *self) {
    Token decl_tok = *self->curr;

    AstNode *type = parse_type(self);

    if (!match(self, TOKEN_IDENTIFIER)) {
        Token tok = *self->curr;

        error_at(self, "expected identifier");
        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
        astnode_destroy(type);

        return astnode_new(AST_NODE_ERROR, &tok);
    }

    AstNode *name = identifier(self);

    if (match(self, TOKEN_SEMICOLON) || match(self, TOKEN_ASSIGN)) {
        return var_decl(self, &decl_tok, type, name);
    } else if (match(self, TOKEN_LPAREN)) {
        advance(self);

        return fn_decl(self, &decl_tok, type, name);
    }

    error_at(self, "expected assignment, function parameter list or ;");
    astnode_destroy(name);
    astnode_destroy(type);

    return astnode_new(AST_NODE_ERROR, self->curr);
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
            sync(
                self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT
            );
        }
    }

    return ast;
}
