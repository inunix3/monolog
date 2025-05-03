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
static AstNode *nil_constant(Parser *self);
static AstNode *unary(Parser *self);
static AstNode *prefix(Parser *self, PrecedenceLevel prec);
static AstNode *binary(Parser *self, AstNode *left);
static AstNode *suffix(Parser *self, AstNode *left);
static AstNode *grouping(Parser *self);
static AstNode *fn_call(Parser *self, AstNode *left);
static AstNode *array_sub(Parser *self, AstNode *left);
static AstNode *block(Parser *self);
static AstNode *statement(Parser *self);
static AstNode *declaration(Parser *self);

/* clang-format off */
static ParseRule g_rules[] = {
    [TOKEN_UNKNOWN]          = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_EOF]              = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_INTEGER]          = {integer_literal, NULL,    NULL,      PREC_NONE},
    [TOKEN_IDENTIFIER]       = {identifier,      NULL,    NULL,      PREC_NONE},
    [TOKEN_STRING]           = {string_literal,  NULL,    NULL,      PREC_NONE},
    [TOKEN_OP_COMMA]         = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_OP_SEMICOLON]     = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_OP_LPAREN]        = {grouping,        fn_call, NULL,      PREC_SUFFIX},
    [TOKEN_OP_RPAREN]        = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_OP_LBRACKET]      = {NULL,            NULL,    array_sub, PREC_SUFFIX},
    [TOKEN_OP_RBRACKET]      = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_OP_LBRACE]        = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_OP_RBRACE]        = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_OP_PLUS]          = {unary,           binary,  NULL,      PREC_ADD},
    [TOKEN_OP_MINUS]         = {unary,           binary,  NULL,      PREC_ADD},
    [TOKEN_OP_MUL]           = {unary,           binary,  NULL,      PREC_MUL},
    [TOKEN_OP_DIV]           = {NULL,            binary,  NULL,      PREC_MUL},
    [TOKEN_OP_MOD]           = {NULL,            binary,  NULL,      PREC_MUL},
    [TOKEN_OP_INC]           = {unary,           NULL,    suffix,    PREC_SUFFIX},
    [TOKEN_OP_DEC]           = {unary,           NULL,    suffix,    PREC_SUFFIX},
    [TOKEN_OP_EXCL]          = {unary,           NULL,    NULL,      PREC_PREFIX},
    [TOKEN_OP_AMP]           = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_OP_PIPE]          = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_OP_LESS]          = {NULL,            binary,  NULL,      PREC_INEQUALITY},
    [TOKEN_OP_GREATER]       = {NULL,            binary,  NULL,      PREC_INEQUALITY},
    [TOKEN_OP_ASSIGN]        = {NULL,            binary,  NULL,      PREC_ASSIGN},
    [TOKEN_OP_EQUAL]         = {NULL,            binary,  NULL,      PREC_EQUALITY},
    [TOKEN_OP_NOT_EQUAL]     = {NULL,            binary,  NULL,      PREC_EQUALITY},
    [TOKEN_OP_LESS_EQUAL]    = {NULL,            binary,  NULL,      PREC_INEQUALITY},
    [TOKEN_OP_GREATER_EQUAL] = {NULL,            binary,  NULL,      PREC_INEQUALITY},
    [TOKEN_OP_AND]           = {NULL,            binary,  NULL,      PREC_AND},
    [TOKEN_OP_OR]            = {NULL,            binary,  NULL,      PREC_OR},
    [TOKEN_OP_QUEST]         = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_OP_HASHTAG]       = {unary,           NULL,    NULL,      PREC_PREFIX},
    [TOKEN_OP_DOLAR]         = {unary,           NULL,    NULL,      PREC_PREFIX},
    [TOKEN_KW_IF]            = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_ELSE]          = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_FOR]           = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_WHILE]         = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_RETURN]        = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_BREAK]         = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_CONTINUE]      = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_NIL]           = {nil_constant,    NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_INT]           = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_VOID]          = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_STRING]        = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_PRINT]         = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_PRINTLN]       = {NULL,            NULL,    NULL,      PREC_NONE},
    [TOKEN_KW_EXIT]          = {NULL,            NULL,    NULL,      PREC_NONE}
};
/* clang-format on */

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
        error(
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
        if (modes & SYNC_TO_SEMICOLON_KEEP && match(self, TOKEN_OP_SEMICOLON)) {
            break;
        }

        if (modes & SYNC_TO_SEMICOLON_SKIP &&
            match_prev(self, TOKEN_OP_SEMICOLON)) {
            break;
        }

        if (modes & SYNC_TO_RBRACE_SKIP && match(self, TOKEN_OP_RBRACE)) {
            break;
        }

        if (modes & SYNC_TO_RBRACE_SKIP && match_prev(self, TOKEN_OP_RBRACE)) {
            break;
        }

        if (modes & SYNC_TO_RPAREN_KEEP && match(self, TOKEN_OP_RPAREN)) {
            break;
        }

        if (modes & SYNC_TO_RPAREN_SKIP && match_prev(self, TOKEN_OP_RPAREN)) {
            break;
        }

        if (modes & SYNC_TO_RBRACKET_KEEP && match(self, TOKEN_OP_RBRACKET)) {
            break;
        }

        if (modes & SYNC_TO_RBRACKET_SKIP && match(self, TOKEN_OP_RBRACKET)) {
            break;
        }

        if (modes & SYNC_TO_COMMA_KEEP && match(self, TOKEN_OP_COMMA)) {
            break;
        }

        if (modes & SYNC_TO_BLOCK && match(self, TOKEN_OP_LBRACE)) {
            break;
        }

        if (modes & SYNC_TO_STATEMENT) {
            switch (self->curr->kind) {
            case TOKEN_KW_IF:
            case TOKEN_KW_FOR:
            case TOKEN_KW_WHILE:
            case TOKEN_KW_PRINT:
            case TOKEN_KW_PRINTLN:
            case TOKEN_KW_EXIT:
            case TOKEN_KW_INT:
            case TOKEN_KW_STRING:
            case TOKEN_KW_VOID:
            case TOKEN_KW_RETURN:
            case TOKEN_KW_BREAK:
            case TOKEN_KW_CONTINUE:
            case TOKEN_OP_LBRACKET:
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
        error(self, "invalid integer literal");
        advance(self);

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
    str_dup_n(&node->literal.str, self->curr->src + 1, self->curr->len - 2);

    advance(self); /* consume the string */

    return node;
}

static AstNode *identifier(Parser *self) {
    AstNode *node = astnode_new(AST_NODE_IDENT);
    str_dup_n(&node->ident.str, self->curr->src, self->curr->len);

    advance(self); /* consume the identifier */

    return node;
}

static AstNode *nil_constant(Parser *self) {
    advance(self); /* consume the nil keyword */

    return astnode_new(AST_NODE_NIL);
}

static AstNode *unary(Parser *self) {
    AstNode *node = astnode_new(AST_NODE_UNARY);

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

static AstNode *suffix(Parser *self, AstNode *left) {
    AstNode *node = astnode_new(AST_NODE_SUFFIX);

    advance(self); /* consume the operator */

    node->suffix.op = self->prev->kind;
    node->suffix.left = left;

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

/* TODO: union value_list() and fn_decl_param_list() */
static bool value_list(Parser *self, Vector *values) {
    if (match(self, TOKEN_OP_RPAREN)) {
        advance(self);

        return true;
    }

    AstNode *expr = NULL;

    while (!match(self, TOKEN_EOF) && !match(self, TOKEN_OP_RPAREN)) {
        expr = expression(self, PREC_NONE);
        vec_push(values, &expr);

        if (expr->kind == AST_NODE_ERROR) {
            sync(self, SYNC_TO_COMMA_KEEP | SYNC_TO_RPAREN_KEEP);

            if (match(self, TOKEN_OP_COMMA)) {
                advance(self);
            }
        } else {
            if (match(self, TOKEN_OP_COMMA)) {
                advance(self);
            } else if (!match(self, TOKEN_OP_RPAREN)) {
                AstNode *error_node = astnode_new(AST_NODE_ERROR);
                vec_push(values, &error_node);

                error(self, "expected , or )");

                sync(self, SYNC_TO_COMMA_KEEP | SYNC_TO_RPAREN_KEEP);

                if (match(self, TOKEN_OP_COMMA)) {
                    advance(self);
                }
            }
        }
    }

    if (!expect(self, TOKEN_OP_RPAREN)) {
        return false;
    }

    return true;
}

static AstNode *fn_call(Parser *self, AstNode *left) {
    advance(self); /* consume the ( */

    AstNode *name = left;

    if (name->kind != AST_NODE_IDENT) {
        error(self, "expected identifier");

        name = astnode_new(AST_NODE_ERROR);
    }

    Vector values;
    vec_init(&values, sizeof(AstNode *));

    if (!value_list(self, &values)) {
        sync(
            self, SYNC_TO_SEMICOLON_KEEP | SYNC_TO_RBRACKET_KEEP |
                      SYNC_TO_BLOCK | SYNC_TO_STATEMENT
        );
    }

    AstNode *node = astnode_new(AST_NODE_FN_CALL);
    node->fn_call.name = name;
    node->fn_call.values = values;

    return node;
}

static AstNode *array_sub(Parser *self, AstNode *left) {
    advance(self); /* consume the [ */

    AstNode *expr = expression(self, PREC_NONE);

    if (!expect(self, TOKEN_OP_RBRACKET)) {
        astnode_destroy(expr);
        astnode_destroy(left);

        return astnode_new(AST_NODE_ERROR);
    }

    AstNode *node = astnode_new(AST_NODE_ARRAY_SUBSCRIPT);
    node->array_sub.expr = expr;
    node->array_sub.left = left;

    return node;
}

static AstNode *block(Parser *self) {
    AstNode *block = astnode_new(AST_NODE_BLOCK);
    vec_init(&block->block.nodes, sizeof(AstNode *));

    for (;;) {
        if (match(self, TOKEN_OP_RBRACE)) {
            break;
        } else if (match(self, TOKEN_EOF)) {
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
        if (self->curr->kind == TOKEN_EOF) {
            error(self, "unexpected EOF");
        } else {
            error(self, "unexpected %.*s", self->curr->len, self->curr->src);
        }

        advance(self);
        sync(
            self, SYNC_TO_RBRACKET_KEEP | SYNC_TO_RBRACE_KEEP |
                      SYNC_TO_SEMICOLON_KEEP | SYNC_TO_RPAREN_KEEP
        );

        return astnode_new(AST_NODE_ERROR);
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

    if (!expect(self, TOKEN_OP_LPAREN)) {
        return astnode_new(AST_NODE_ERROR);
    }

    expr = expression(self, PREC_NONE);

    if (expr->kind != AST_NODE_ERROR && !expect(self, TOKEN_OP_RPAREN)) {
        astnode_destroy(expr);
        expr = astnode_new(AST_NODE_ERROR);
    }

    return expr;
}

static AstNode *if_statement(Parser *self) {
    advance(self); /* consume if keyword */

    AstNode *stmt = astnode_new(AST_NODE_IF);
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

    if (match(self, TOKEN_KW_ELSE)) {
        advance(self);

        stmt->kw_if.else_body = statement(self);
    }

    return stmt;
}

static AstNode *while_statement(Parser *self) {
    advance(self); /* consume while keyword */

    AstNode *stmt = astnode_new(AST_NODE_WHILE);
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

/* TODO: union optional_var_decl_with_delimiter() and
 * optional_expression_with_delimiter() */
static AstNode *
optional_var_decl_with_delimiter(Parser *self, TokenKind delim) {
    AstNode *decl = NULL;

    if (!match(self, delim)) {
        /* TODO: allow only variable declarations */
        decl = declaration(self);
    } else {
        advance(self);
    }

    if (decl && decl->kind != AST_NODE_ERROR && !expect(self, delim)) {
        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);

        astnode_destroy(decl);
        decl = astnode_new(AST_NODE_ERROR);
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
        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);

        astnode_destroy(expr);
        expr = astnode_new(AST_NODE_ERROR);
    }

    return expr;
}

static bool token_is_type(TokenKind tok) {
    switch (tok) {
    case TOKEN_KW_INT:
    case TOKEN_KW_STRING:
    case TOKEN_KW_VOID:
    case TOKEN_OP_LBRACKET:
        return true;
    default:
        return false;
    }
}

static AstNode *for_statement(Parser *self) {
    advance(self);

    if (!expect(self, TOKEN_OP_LPAREN)) {
        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);

        return astnode_new(AST_NODE_ERROR);
    }

    AstNode *init = NULL;

    if (token_is_type(self->curr->kind)) {
        init = optional_var_decl_with_delimiter(self, TOKEN_OP_SEMICOLON);
    } else {
        init = optional_expression_with_delimiter(self, TOKEN_OP_SEMICOLON);
    }

    AstNode *cond =
        optional_expression_with_delimiter(self, TOKEN_OP_SEMICOLON);
    AstNode *iter = optional_expression_with_delimiter(self, TOKEN_OP_RPAREN);
    AstNode *body = statement(self);

    if (body && body->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
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

    if (expr->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_RPAREN_KEEP);
    }

    expect(self, TOKEN_OP_RPAREN);

    AstNode *stmt = astnode_new(AST_NODE_PRINT);
    stmt->kw_print.expr = expr;

    return stmt;
}

static AstNode *println_statement(Parser *self) {
    advance(self); /* consume println keyword */

    expect(self, TOKEN_OP_LPAREN);
    AstNode *expr = expression(self, PREC_NONE);

    if (expr->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_RPAREN_KEEP);
    }

    expect(self, TOKEN_OP_RPAREN);

    AstNode *stmt = astnode_new(AST_NODE_PRINTLN);
    stmt->kw_print.expr = expr;

    return stmt;
}

static AstNode *exit_statement(Parser *self) {
    advance(self); /* consume exit keyword */

    expect(self, TOKEN_OP_LPAREN);
    AstNode *expr = expression(self, PREC_NONE);

    if (expr->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_RPAREN_KEEP);
    }

    expect(self, TOKEN_OP_RPAREN);

    AstNode *stmt = astnode_new(AST_NODE_EXIT);
    stmt->kw_exit.expr = expr;

    return stmt;
}

static AstNode *return_statement(Parser *self) {
    advance(self); /* consume the return keyword */

    AstNode *expr = NULL;

    if (self->curr->kind != TOKEN_OP_SEMICOLON) {
        expr = expression(self, PREC_NONE);
    }

    if (expr && expr->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
    }

    AstNode *node = astnode_new(AST_NODE_RETURN);
    node->kw_return.expr = expr;

    return node;
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
    case TOKEN_KW_EXIT:
        stmt = exit_statement(self);

        break;
    case TOKEN_OP_LBRACE:
        advance(self);
        stmt = block(self);

        break;
    case TOKEN_KW_INT:
    case TOKEN_KW_STRING:
    case TOKEN_KW_VOID:
    case TOKEN_OP_LBRACKET:
        stmt = declaration(self);

        break;
    case TOKEN_KW_RETURN:
        stmt = return_statement(self);

        break;
    case TOKEN_KW_BREAK:
        advance(self);

        stmt = astnode_new(AST_NODE_BREAK);

        break;
    case TOKEN_KW_CONTINUE:
        advance(self);

        stmt = astnode_new(AST_NODE_CONTINUE);

        break;
    default:
        stmt = expression(self, PREC_NONE);

        break;
    }

    if (stmt->kind != AST_NODE_ERROR && stmt->kind != AST_NODE_FN_DECL &&
        !match_prev(self, TOKEN_OP_RBRACE) && !match(self, TOKEN_EOF) &&
        !expect(self, TOKEN_OP_SEMICOLON)) {
        astnode_destroy(stmt);

        return astnode_new(AST_NODE_ERROR);
    }

    return stmt;
}

static AstNode *parse_type(Parser *self) {
    AstNodeKind type_id;

    switch (self->curr->kind) {
    case TOKEN_KW_INT:
        type_id = AST_NODE_INT_TYPE;

        break;
    case TOKEN_KW_STRING:
        type_id = AST_NODE_STRING_TYPE;

        break;
    case TOKEN_KW_VOID:
        type_id = AST_NODE_VOID_TYPE;

        break;
    case TOKEN_OP_LBRACKET:
        type_id = AST_NODE_ARRAY_TYPE;

        break;
    default:
        type_id = AST_NODE_ERROR;

        break;
    }

    advance(self);

    AstNode *node = NULL;

    if (type_id == AST_NODE_ERROR) {
        error(self, "expected type specifier: int, string, void or array");

        return astnode_new(AST_NODE_ERROR);
    } else if (type_id == AST_NODE_ARRAY_TYPE) {
        AstNode *type = parse_type(self);

        if (type->kind == AST_NODE_ERROR) {
            sync(self, SYNC_TO_COMMA_KEEP);
        }

        AstNode *size = NULL;

        if (!expect(self, TOKEN_OP_COMMA)) {
            size = astnode_new(AST_NODE_ERROR);

            sync(self, SYNC_TO_RBRACKET_KEEP);
        } else {
            size = expression(self, PREC_NONE);
        }

        if (!expect(self, TOKEN_OP_RBRACKET)) {
            sync(self, SYNC_TO_RBRACKET_SKIP);
        }

        node = astnode_new(type_id);
        node->array_type.type = type;
        node->array_type.size = size;
    } else {
        node = astnode_new(type_id);
    }

    while (match(self, TOKEN_OP_QUEST)) {
        advance(self);

        AstNode *parent_node = astnode_new(AST_NODE_OPTION_TYPE);
        parent_node->opt_type.type = node;

        node = parent_node;
    }

    return node;
}

static AstNode *param_decl(Parser *self) {
    AstNode *type = parse_type(self);

    if (type->kind == AST_NODE_ERROR) {
        return type;
    }

    if (!match(self, TOKEN_IDENTIFIER)) {
        error(self, "expected identifier");
        astnode_destroy(type);

        return astnode_new(AST_NODE_ERROR);
    }

    AstNode *name = identifier(self);

    AstNode *node = astnode_new(AST_NODE_PARAM_DECL);
    node->param_decl.type = type;
    node->param_decl.name = name;

    return node;
}

/* TODO: union value_list() and fn_decl_param_list() */
static bool fn_decl_param_list(Parser *self, Vector *params) {
    if (match(self, TOKEN_OP_RPAREN)) {
        advance(self);

        return true;
    }

    AstNode *param = NULL;

    while (!match(self, TOKEN_EOF) && !match(self, TOKEN_OP_RPAREN)) {
        param = param_decl(self);
        vec_push(params, &param);

        if (param->kind == AST_NODE_ERROR) {
            sync(self, SYNC_TO_COMMA_KEEP | SYNC_TO_RPAREN_KEEP);

            if (match(self, TOKEN_OP_COMMA)) {
                advance(self);
            }
        } else {
            if (match(self, TOKEN_OP_COMMA)) {
                advance(self);
            } else if (!match(self, TOKEN_OP_RPAREN)) {
                AstNode *error_node = astnode_new(AST_NODE_ERROR);
                vec_push(params, &error_node);

                error(self, "expected , or )");

                sync(self, SYNC_TO_COMMA_KEEP | SYNC_TO_RPAREN_KEEP);

                if (match(self, TOKEN_OP_COMMA)) {
                    advance(self);
                }
            }
        }
    }

    if (param->kind != AST_NODE_ERROR && !expect(self, TOKEN_OP_RPAREN)) {
        return false;
    }

    return true;
}

static AstNode *declaration(Parser *self) {
    AstNode *type = parse_type(self);

    if (!match(self, TOKEN_IDENTIFIER)) {
        error(self, "expected identifier");
        sync(self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
        astnode_destroy(type);

        return astnode_new(AST_NODE_ERROR);
    }

    AstNode *name = identifier(self);
    AstNode *rvalue = NULL;

    if (match(self, TOKEN_OP_ASSIGN)) {
        advance(self);

        rvalue = expression(self, PREC_NONE);
    } else if (match(self, TOKEN_OP_LPAREN)) {
        advance(self);

        Vector params;
        vec_init(&params, sizeof(AstNode *));

        if (!fn_decl_param_list(self, &params)) {
            sync(
                self, SYNC_TO_SEMICOLON_KEEP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT
            );
        }

        AstNode *body = statement(self);

        if (body && body->kind == AST_NODE_ERROR) {
            sync(
                self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT
            );
        }

        AstNode *node = astnode_new(AST_NODE_FN_DECL);

        node->fn_decl.type = type;
        node->fn_decl.name = name;
        node->fn_decl.params = params;
        node->fn_decl.body = body;

        return node;
    } else if (!match(self, TOKEN_OP_SEMICOLON)) {
        error(self, "expected assignment, function parameter list or ;");

        rvalue = astnode_new(AST_NODE_ERROR);
    }

    if (rvalue && rvalue->kind == AST_NODE_ERROR) {
        sync(self, SYNC_TO_SEMICOLON_KEEP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT);
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
            sync(
                self, SYNC_TO_SEMICOLON_SKIP | SYNC_TO_BLOCK | SYNC_TO_STATEMENT
            );
        }
    }

    return ast;
}
