#include <monolog/interp.h>

#include <inttypes.h>
#include <stdio.h>

#define CASE_BINARY_INT(_op, _v1, _v2)                                         \
    val.i = (_v1)->i _op(_v2)->i;                                              \
    break

#define CASE_UNARY_INT(_op, _v1)                                               \
    val.i = _op((_v1)->i);                                                     \
    break

#define RETURN_IF_ERROR(_v)                                                    \
    if ((_v).type == TYPE_ERROR)                                               \
        return _v;
#define RETURN_VOID_IF_ERROR(_v)                                               \
    if ((_v).type == TYPE_ERROR)                                               \
        return;

static Value g_err_value = {TYPE_ERROR};

static void error(Interpreter *self, const char *msg) {
    self->had_error = true;
    fputs(msg, stderr);
    putchar('\n');
}

static Value
exec_binary_int(Interpreter *self, TokenKind op, Value *v1, Value *v2) {
    Value val = {TYPE_INT};

    switch (op) {
    case TOKEN_OP_PLUS:
        CASE_BINARY_INT(+, v1, v2);
    case TOKEN_OP_MINUS:
        CASE_BINARY_INT(-, v1, v2);
    case TOKEN_OP_MUL:
        CASE_BINARY_INT(*, v1, v2);
    case TOKEN_OP_DIV:
        if (v2->i == 0) {
            error(self, "division by zero");

            return g_err_value;
        }

        CASE_BINARY_INT(/, v1, v2);
    case TOKEN_OP_MOD:
        if (v2->i == 0) {
            error(self, "division by zero");

            return g_err_value;
        }

        CASE_BINARY_INT(%, v1, v2);
    case TOKEN_OP_EQUAL:
        CASE_BINARY_INT(==, v1, v2);
    case TOKEN_OP_NOT_EQUAL:
        CASE_BINARY_INT(!=, v1, v2);
    case TOKEN_OP_LESS:
        CASE_BINARY_INT(<, v1, v2);
    case TOKEN_OP_GREATER:
        CASE_BINARY_INT(>, v1, v2);
    case TOKEN_OP_LESS_EQUAL:
        CASE_BINARY_INT(<=, v1, v2);
    case TOKEN_OP_GREATER_EQUAL:
        CASE_BINARY_INT(>=, v1, v2);
    case TOKEN_OP_AND:
        CASE_BINARY_INT(&&, v1, v2);
    case TOKEN_OP_OR:
        CASE_BINARY_INT(||, v1, v2);
    default:
        break;
    }

    return val;
}

static Value
exec_binary_string(Interpreter *self, TokenKind op, Value *v1, Value *v2) {
    Value val = *v1;
    val.type = TYPE_STRING;

    switch (op) {
    case TOKEN_OP_PLUS:
        str_cat(&val.s, &v2->s);

        break;
    default:
        break;
    }

    return val;
}

static Value exec_expr(Interpreter *self, const AstNode *node);

static Value exec_unary_int(Interpreter *self, TokenKind op, Value *v1) {
    Value val = {TYPE_INT};

    switch (op) {
    case TOKEN_OP_PLUS:
        CASE_UNARY_INT(+, v1);
    case TOKEN_OP_MINUS:
        CASE_UNARY_INT(-, v1);
    case TOKEN_OP_EXCL:
        val.i = !(v1->i != 0);

        break;
    case TOKEN_OP_DOLAR: {
        int len = snprintf(NULL, 0, "%" PRId64, v1->i);

        str_init_n(&val.s, len);
        snprintf(val.s.data, len + 1, "%" PRId64, v1->i);

        val.type = TYPE_STRING;

        break;
    }
    default:
        break;
    }

    return val;
}

static Value exec_unary_string(Interpreter *self, TokenKind op, Value *v1) {
    Value val = {TYPE_STRING};

    switch (op) {
    case TOKEN_OP_HASHTAG:
        val.i = v1->s.len;

        break;
    default:
        break;
    }

    return val;
}

static Value exec_unary(Interpreter *self, const AstNode *node) {
    Value val;

    Value v1 = exec_expr(self, node->unary.right);
    RETURN_IF_ERROR(v1);

    switch (v1.type) {
    case TYPE_INT:
        return exec_unary_int(self, node->unary.op, &v1);
    case TYPE_STRING:
        return exec_unary_string(self, node->unary.op, &v1);
    default:
        break;
    }

    return val;
}

static Value exec_binary(Interpreter *self, const AstNode *node) {
    Value val;

    Value v1 = exec_expr(self, node->binary.left);
    RETURN_IF_ERROR(v1);

    Value v2 = exec_expr(self, node->binary.right);
    RETURN_IF_ERROR(v2);

    switch (v1.type) {
    case TYPE_INT:
        return exec_binary_int(self, node->binary.op, &v1, &v2);
    case TYPE_STRING:
        return exec_binary_string(self, node->binary.op, &v1, &v2);
    default:
        break;
    }

    return val;
}

static Value exec_expr(Interpreter *self, const AstNode *node) {
    Value val;

    switch (node->kind) {
    case AST_NODE_INTEGER:
        val.type = TYPE_INT;
        val.i = node->literal.i;

        break;
    case AST_NODE_STRING:
        val.type = TYPE_STRING;
        str_dup_n(&val.s, node->literal.str.data, node->literal.str.len);

        break;
    case AST_NODE_UNARY:
        val = exec_unary(self, node);

        break;
    case AST_NODE_BINARY:
        val = exec_binary(self, node);

        break;
    case AST_NODE_GROUPING:
        return exec_expr(self, node->grouping.expr);
    default:
        break;
    }

    return val;
}

static void builtin_print(Interpreter *self, const char *str) {
    fputs(str, stdout);
}

static void builtin_println(Interpreter *self, const char *str) {
    fputs(str, stdout);
    putchar('\n');
}

static void exec_node(Interpreter *self, const AstNode *node) {
    switch (node->kind) {
    case AST_NODE_BLOCK: {
        AstNode **nodes = node->block.nodes.data;

        for (size_t i = 0; i < node->block.nodes.len; ++i) {
            exec_node(self, nodes[i]);

            if (self->had_error || self->halt) {
                break;
            }
        }

        break;
    }
    case AST_NODE_PRINT: {
        Value v = exec_expr(self, node->kw_print.expr);
        RETURN_VOID_IF_ERROR(v);

        builtin_print(self, v.s.data);

        break;
    }
    case AST_NODE_PRINTLN: {
        Value v = exec_expr(self, node->kw_print.expr);
        RETURN_VOID_IF_ERROR(v);

        builtin_println(self, v.s.data);

        break;
    case AST_NODE_EXIT: {
        Value v = exec_expr(self, node->kw_print.expr);
        RETURN_VOID_IF_ERROR(v);

        self->exit_code = v.i;
        self->halt = true;

    } break;
    case AST_NODE_IF: {
        Value cond = exec_expr(self, node->kw_if.cond);
        RETURN_VOID_IF_ERROR(cond);

        if (cond.i) {
            exec_node(self, node->kw_if.body);
        } else if (node->kw_if.else_body) {
            exec_node(self, node->kw_if.else_body);
        }

        break;
    }
    case AST_NODE_WHILE: {
        Value cond = exec_expr(self, node->kw_while.cond);
        RETURN_VOID_IF_ERROR(cond);

        while (cond.i) {
            exec_node(self, node->kw_while.body);

            cond = exec_expr(self, node->kw_while.cond);
            RETURN_VOID_IF_ERROR(cond);
        }

        break;
    }
    }
    default:
        exec_expr(self, node);
        break;
    }
}

void interp_init(Interpreter *self, const Ast *ast) {
    self->ast = ast;
    self->exit_code = 0;
    self->had_error = false;
}

void interp_deinit(Interpreter *self) { (void)self; }

int interp_run(Interpreter *self) {
    const AstNode **nodes = self->ast->nodes.data;

    for (size_t i = 0; i < self->ast->nodes.len; ++i) {
        exec_node(self, nodes[i]);

        if (self->had_error) {
            self->exit_code = -1;
            break;
        } else if (self->halt) {
            break;
        }
    }

    return self->exit_code;
}
