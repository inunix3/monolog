#include <monolog/ast.h>
#include <monolog/interp.h>
#include <monolog/utils.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CASE_BINARY_INT(_op, _v1, _v2)                                         \
    val.i = (_v1)->i _op(_v2)->i;                                              \
    break

#define CASE_UNARY_INT(_op, _v1)                                               \
    val.i = _op((_v1)->i);                                                     \
    break

#define RETURN_IF_ERROR(_v)                                                    \
    if ((_v).kind == EXPR_ERROR)                                               \
        return _v;

#define RETURN_VOID_IF_ERROR(_v)                                               \
    if ((_v).kind == EXPR_ERROR)                                               \
        return;

#define EXPR_GET_VALUE(_expr)                                                  \
    ((_expr).kind == EXPR_VAR ? (_expr).var->val : (_expr).val)

static void error(Interpreter *self, const char *msg) {
    self->had_error = true;

    if (self->log_errors) {
        fprintf(stderr, "runtime error: %s\n", msg);
    }
}

static Value
exec_binary_int(Interpreter *self, TokenKind op, Value *v1, Value *v2) {
    Value val = {self->types->builtin_int, {0}};

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

            val.type = self->types->error_type;

            return val;
        }

        CASE_BINARY_INT(/, v1, v2);
    case TOKEN_OP_MOD:
        if (v2->i == 0) {
            error(self, "division by zero");

            val.type = self->types->error_type;

            return val;
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
    val.type = self->types->builtin_string;

    switch (op) {
    case TOKEN_OP_PLUS:
        str_cat(val.s, v2->s);

        break;
    default:
        break;
    }

    return val;
}

static ExprResult exec_expr(Interpreter *self, const AstNode *node);

static Value exec_unary_int(Interpreter *self, TokenKind op, Value *v1) {
    Value val = {self->types->builtin_int, {0}};

    switch (op) {
    case TOKEN_OP_INC:
        val.i = v1->i + 1;

        break;
    case TOKEN_OP_DEC:
        val.i = v1->i - 1;

        break;
    case TOKEN_OP_PLUS:
        CASE_UNARY_INT(+, v1);
    case TOKEN_OP_MINUS:
        CASE_UNARY_INT(-, v1);
    case TOKEN_OP_EXCL:
        val.i = !(v1->i != 0);

        break;
    case TOKEN_OP_DOLAR: {
        size_t len = (size_t)snprintf(NULL, 0, "%" PRId64, v1->i);

        StrBuf str;
        StrBuf *strp = vec_push(&self->strings, &str);

        str_init_n(strp, len);
        snprintf(strp->data, len + 1, "%" PRId64, v1->i);

        val.type = self->types->builtin_string;
        val.s = strp;

        break;
    }
    default:
        break;
    }

    return val;
}

static Value exec_unary_string(Interpreter *self, TokenKind op, Value *v1) {
    Value val = {self->types->builtin_string, {0}};

    switch (op) {
    case TOKEN_OP_HASHTAG:
        val.type = self->types->builtin_int;
        val.i = (Int)v1->s->len;

        break;
    default:
        break;
    }

    return val;
}

static ExprResult exec_unary(Interpreter *self, const AstNode *node) {
    ExprResult expr = exec_expr(self, node->unary.right);
    RETURN_IF_ERROR(expr);

    Value val = EXPR_GET_VALUE(expr);

    switch (val.type->id) {
    case TYPE_INT:
        val = exec_unary_int(self, node->unary.op, &val);

        break;
    case TYPE_STRING:
        val = exec_unary_string(self, node->unary.op, &val);

        break;
    default:
        break;
    }

    ExprResult expr_res;

    if (expr.kind == EXPR_VAR &&
        (node->unary.op == TOKEN_OP_INC || node->unary.op == TOKEN_OP_DEC)) {
        expr_res.kind = EXPR_VAR;
        expr_res.var = expr.var;
        expr_res.var->val = val;
    } else {
        expr_res.kind = EXPR_VALUE;
        expr_res.val = val;
    }

    return expr_res;
}

static ExprResult
assign_var(Interpreter *self, Variable *var, ExprResult expr) {
    (void)self;

    var->val = EXPR_GET_VALUE(expr);

    ExprResult expr_res;
    expr_res.kind = EXPR_VALUE;
    expr_res.val = var->val;

    return expr_res;
}

static ExprResult exec_binary(Interpreter *self, const AstNode *node) {
    ExprResult expr1 = exec_expr(self, node->binary.left);
    RETURN_IF_ERROR(expr1);

    ExprResult expr2 = exec_expr(self, node->binary.right);
    RETURN_IF_ERROR(expr2);

    if (expr1.kind == EXPR_VAR && node->binary.op == TOKEN_OP_ASSIGN) {
        return assign_var(self, expr1.var, expr2);
    }

    Value v1 = EXPR_GET_VALUE(expr1);
    Value v2 = EXPR_GET_VALUE(expr2);

    ExprResult expr;

    switch (v1.type->id) {
    case TYPE_INT:
        expr.kind = EXPR_VALUE;
        expr.val = exec_binary_int(self, node->binary.op, &v1, &v2);

        break;
    case TYPE_STRING:
        expr.kind = EXPR_VALUE;
        expr.val = exec_binary_string(self, node->binary.op, &v1, &v2);

        break;
    default:
        expr.kind = EXPR_ERROR;
        error(self, "unsupported binary operation for 2 types");

        break;
    }

    return expr;
}

static ExprResult exec_expr(Interpreter *self, const AstNode *node) {
    ExprResult expr_res;

    switch (node->kind) {
    case AST_NODE_INTEGER:
        expr_res.kind = EXPR_VALUE;
        expr_res.val.type = self->types->builtin_int;
        expr_res.val.i = node->literal.i;

        return expr_res;
    case AST_NODE_STRING:
        expr_res.kind = EXPR_VALUE;

        StrBuf str = {0};
        StrBuf *strp = vec_push(&self->strings, &str);

        str_dup_n(strp, node->literal.str.data, node->literal.str.len);

        expr_res.val.type = self->types->builtin_string;
        expr_res.val.s = strp;

        return expr_res;
    case AST_NODE_IDENT: {
        Variable *var = env_find_var(&self->env, node->ident.str.data);

        if (!var) {
            expr_res.kind = EXPR_ERROR;
            error(self, "cannot find variable");

            return expr_res;
        }

        expr_res.kind = EXPR_VAR;
        expr_res.var = var;

        return expr_res;
    }
    case AST_NODE_UNARY:
        return exec_unary(self, node);
    case AST_NODE_BINARY:
        return exec_binary(self, node);
    case AST_NODE_GROUPING:
        return exec_expr(self, node->grouping.expr);
    default:
        expr_res.kind = EXPR_ERROR;
        error(self, "invalid expression");

        return expr_res;
    }
}

static void builtin_print(Interpreter *self, const char *str) {
    (void)self;

    fputs(str, stdout);
}

static void builtin_println(Interpreter *self, const char *str) {
    (void)self;

    fputs(str, stdout);
    fputc('\n', stdout);
}

static Type *process_type(Interpreter *self, const AstNode *node) {
    switch (node->kind) {
    case AST_NODE_INT_TYPE:
        return self->types->builtin_int;
    case AST_NODE_STRING_TYPE:
        return self->types->builtin_string;
    case AST_NODE_VOID_TYPE:
        return self->types->builtin_void;
    case AST_NODE_ARRAY_TYPE: {
        Type *contained_type = process_type(self, node->array_type.type);
        Type array = {TYPE_ARRAY, NULL, {0}};
        array.array_type.type = contained_type;

        return type_system_register(self->types, &array);
    }
    case AST_NODE_OPTION_TYPE: {
        Type *contained_type = process_type(self, node->opt_type.type);
        Type option = {TYPE_OPTION, NULL, {0}};
        option.opt_type.type = contained_type;

        return type_system_register(self->types, &option);
    }
    case AST_NODE_NIL:
        return self->types->nil_type;
    default:
        return self->types->error_type;
    }
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
    case AST_NODE_VAR_DECL: {
        Type *type = process_type(self, node->var_decl.type);

        Value val = {self->types->error_type, {0}};
        bool defined = false;

        if (node->var_decl.rvalue) {
            ExprResult expr = exec_expr(self, node->var_decl.rvalue);
            RETURN_VOID_IF_ERROR(expr);

            val = expr.kind == EXPR_VAR ? expr.var->val : expr.val;

            if (!type_equal(val.type, type)) {
                error(self, "type mismatch");

                return;
            }

            defined = true;
        }

        Variable *var = malloc(sizeof(*var));
        var->name = cstr_dup(node->var_decl.name->ident.str.data);
        var->type = type;
        var->val = val;
        var->defined = defined;
        var->is_param = false;

        hashmap_add(&self->env.curr_scope->vars, var->name, var);

        break;
    }
    case AST_NODE_PRINT: {
        ExprResult expr = exec_expr(self, node->kw_print.expr);
        RETURN_VOID_IF_ERROR(expr);

        Value val = expr.kind == EXPR_VAR ? expr.var->val : expr.val;

        builtin_print(self, val.s->data);

        break;
    }
    case AST_NODE_PRINTLN: {
        ExprResult expr = exec_expr(self, node->kw_print.expr);
        RETURN_VOID_IF_ERROR(expr);

        Value val = expr.kind == EXPR_VAR ? expr.var->val : expr.val;

        builtin_println(self, val.s->data);

        break;
    }
    case AST_NODE_EXIT: {
        ExprResult expr = exec_expr(self, node->kw_exit.expr);
        RETURN_VOID_IF_ERROR(expr);

        Value val = expr.kind == EXPR_VAR ? expr.var->val : expr.val;

        self->exit_code = (int)val.i;
        self->halt = true;

        break;
    }
    case AST_NODE_IF: {
        ExprResult cond = exec_expr(self, node->kw_if.cond);
        RETURN_VOID_IF_ERROR(cond);

        Value cond_val = EXPR_GET_VALUE(cond);

        if (cond_val.i) {
            exec_node(self, node->kw_if.body);
        } else if (node->kw_if.else_body) {
            exec_node(self, node->kw_if.else_body);
        }

        break;
    }
    case AST_NODE_WHILE: {
        ExprResult cond = exec_expr(self, node->kw_while.cond);
        RETURN_VOID_IF_ERROR(cond);

        Value cond_val = EXPR_GET_VALUE(cond);

        while (cond_val.i) {
            exec_node(self, node->kw_while.body);

            cond = exec_expr(self, node->kw_while.cond);
            RETURN_VOID_IF_ERROR(cond);

            cond_val = EXPR_GET_VALUE(cond);
        }

        break;
    }
    default:
        exec_expr(self, node);

        break;
    }
}

void interp_init(Interpreter *self, const Ast *ast, TypeSystem *types) {
    self->types = types;
    env_init(&self->env);
    vec_init(&self->strings, sizeof(StrBuf));

    self->ast = ast;
    self->exit_code = 0;
    self->halt = false;
    self->had_error = false;
    self->log_errors = false;
}

void interp_deinit(Interpreter *self) {
    env_deinit(&self->env);

    StrBuf *strings = self->strings.data;

    for (size_t i = 0; i < self->strings.len; ++i) {
        str_deinit(&strings[i]);
    }

    vec_deinit(&self->strings);
}

int interp_walk(Interpreter *self) {
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

Value interp_eval(Interpreter *self) {
    Value val;

    if (self->ast->nodes.len == 0) {
        val.type = self->types->builtin_void;

        return val;
    }

    ExprResult expr =
        exec_expr(self, ((const AstNode **)self->ast->nodes.data)[0]);
    val = EXPR_GET_VALUE(expr);

    if (self->had_error) {
        val.type = self->types->error_type;
        self->exit_code = -1;
    }

    return val;
}
