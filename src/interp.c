#include <monolog/ast.h>
#include <monolog/interp.h>
#include <monolog/utils.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CASE_BINARY_INT(_op, _v1, _v2)                                         \
    val.i = (_v1)->i _op(_v2)->i;                                              \
    break

#define CASE_UNARY_INT(_op, _v1)                                               \
    val.i = _op((_v1)->i);                                                     \
    break

#define EXPR_RETURN_IF_ERROR(_e, _err)                                         \
    if ((_e).kind == (_err)) {                                                 \
        ExprResult __expr_res = {EXPR_ERROR, {0}};                             \
                                                                               \
        return __expr_res;                                                     \
    }

#define STMT_RETURN_IF_ERROR(_e, _err)                                         \
    if ((_e).kind == (_err)) {                                                 \
        StmtResult __stmt_res = {STMT_ERROR, {0}};                             \
                                                                               \
        return __stmt_res;                                                     \
    }

#define EXPR_GET_VALUE(_expr)                                                  \
    ((_expr).kind == EXPR_VAR ? (_expr).var->val : (_expr).val)

static void error(Interpreter *self, const char *fmt, ...) {
    self->had_error = true;

    if (self->log_errors) {
        fputs("runtime error: ", stderr);

        va_list vargs;

        va_start(vargs, fmt);
        vfprintf(stderr, fmt, vargs);
        va_end(vargs);

        fputc('\n', stderr);
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

static ExprResult
exec_expr(Interpreter *self, const AstNode *node, bool assigning);

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
        val.i = !val.i;

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
    ExprResult expr = exec_expr(self, node->unary.right, false);
    EXPR_RETURN_IF_ERROR(expr, EXPR_ERROR);

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
    bool assigning = node->binary.op == TOKEN_OP_ASSIGN;

    ExprResult expr1 = exec_expr(self, node->binary.left, assigning);
    EXPR_RETURN_IF_ERROR(expr1, EXPR_ERROR);

    ExprResult expr2 = exec_expr(self, node->binary.right, false);
    EXPR_RETURN_IF_ERROR(expr2, EXPR_ERROR);

    if (expr1.kind == EXPR_VAR && assigning) {
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
        error(
            self, "unsupported binary operation for %s and %s", v1.type->name,
            v2.type->name
        );

        break;
    }

    return expr;
}

static ExprResult
exec_suffix_int(Interpreter *self, Variable *var, TokenKind op) {
    Value val = {self->types->builtin_int, {0}};
    ExprResult expr_res = {EXPR_VALUE, {0}};

    switch (op) {
    case TOKEN_OP_INC: {
        Int old = var->val.i;
        ++var->val.i;

        val.i = old;

        break;
    }
    case TOKEN_OP_DEC: {
        Int old = var->val.i;
        --var->val.i;

        val.i = old;

        break;
    }
    default:
        expr_res.kind = EXPR_ERROR;
        error(self, "bad suffix operator %s", token_kind_to_str(op));

        return expr_res;
    }

    expr_res.val = val;

    return expr_res;
}

static ExprResult exec_suffix(Interpreter *self, const AstNode *node) {
    ExprResult expr_res = exec_expr(self, node->suffix.left, false);

    if (expr_res.kind != EXPR_VAR) {
        return expr_res;
    }

    Variable *var = expr_res.var;

    switch (var->type->id) {
    case TYPE_INT:
        return exec_suffix_int(self, var, node->suffix.op);
    default:
        expr_res.kind = EXPR_ERROR;

        error(
            self, "unsupported type %s for suffix operator %s", var->type->name,
            node->suffix.op
        );

        break;
    }

    return expr_res;
}

static ExprResult exec_string_literal(Interpreter *self, const AstNode *node) {
    ExprResult expr_res;

    expr_res.kind = EXPR_VALUE;

    StrBuf str = {0};
    StrBuf *strp = vec_push(&self->strings, &str);

    str_dup_n(strp, node->literal.str.data, node->literal.str.len);

    expr_res.val.type = self->types->builtin_string;
    expr_res.val.s = strp;

    return expr_res;
}

static ExprResult
exec_identifier(Interpreter *self, const AstNode *node, bool assigning) {
    ExprResult expr_res;

    Variable *var = env_find_var(&self->env, node->ident.str.data);

    if (!var) {
        expr_res.kind = EXPR_ERROR;
        error(self, "undeclared variable %s", node->ident.str.data);

        return expr_res;
    }

    if (!assigning && !var->defined && !var->is_param) {
        expr_res.kind = EXPR_ERROR;
        error(
            self, "variable %s is declared, but no value was assigned to it",
            var->name
        );

        return expr_res;
    }

    expr_res.kind = EXPR_VAR;
    expr_res.var = var;

    return expr_res;
}

static ExprResult
exec_string_subscript(Interpreter *self, const StrBuf *str, Int idx) {
    ExprResult expr_res = {0};

    if ((size_t)idx >= str->len) {
        expr_res.kind = EXPR_ERROR;
        error(
            self,
            "string index out of range: %" PRId64
            " but the string length is %zu",
            idx, str->len
        );

        return expr_res;
    }

    expr_res.kind = EXPR_VALUE;
    expr_res.val.type = self->types->builtin_int;
    expr_res.val.i = str->data[idx];

    return expr_res;
}

static ExprResult exec_subscript(Interpreter *self, const AstNode *node) {
    ExprResult left_expr = exec_expr(self, node->array_sub.left, false);
    EXPR_RETURN_IF_ERROR(left_expr, EXPR_ERROR);

    ExprResult idx_expr = exec_expr(self, node->array_sub.expr, false);
    EXPR_RETURN_IF_ERROR(idx_expr, EXPR_ERROR);

    ExprResult expr_res = {0};
    Value left_val = EXPR_GET_VALUE(left_expr);
    Value idx = EXPR_GET_VALUE(idx_expr);

    switch (left_val.type->id) {
    case TYPE_STRING:
        return exec_string_subscript(self, left_val.s, idx.i);
    default:
        expr_res.kind = EXPR_ERROR;
        error(self, "unsupported type %s for indexing", left_val.type->name);

        break;
    }

    return expr_res;
}

static StmtResult exec_node(Interpreter *self, AstNode *node);

static bool fill_fn_params_values(Interpreter *self, const AstNode **args) {
    const FnParam *params = self->env.curr_fn->params.data;

    for (size_t i = 0; i < self->env.curr_fn->params.len; ++i) {
        const AstNode *arg_node = args[i];
        const FnParam *param = &params[i];

        ExprResult expr = exec_expr(self, arg_node, false);

        if (expr.kind == EXPR_ERROR) {
            return false;
        }

        Variable *arg = malloc(sizeof(*arg));
        arg->type = param->type;
        arg->name = cstr_dup(param->name);
        arg->val = EXPR_GET_VALUE(expr);
        arg->defined = true;
        arg->is_param = true;

        env_add_local_var(&self->env, arg);
    }

    return true;
}

static ExprResult exec_fn_call(Interpreter *self, const AstNode *node) {
    const char *name = node->fn_call.name->ident.str.data;
    Function *fn = env_find_fn(&self->env, name);
    ExprResult expr_res = {0};

    if (!fn) {
        expr_res.kind = EXPR_ERROR;

        return expr_res;
    } else if (!fn->body) {
        expr_res.kind = EXPR_ERROR;
        error(self, "function %s has no body to execute", name);

        return expr_res;
    }

    Scope *caller_scope = self->env.curr_scope;
    env_enter_fn(&self->env, fn);

    const AstNode **args = node->fn_call.values.data;

    if (!fill_fn_params_values(self, args)) {
        expr_res.kind = EXPR_ERROR;

        goto finish;
    }

    env_save_caller(&self->env, caller_scope);

    StmtResult body_res = exec_node(self, fn->body);

    switch (body_res.kind) {
    case STMT_ERROR:
        expr_res.kind = EXPR_ERROR;

        break;
    case STMT_RETURN:
        expr_res.kind = EXPR_VALUE;
        expr_res.val = body_res.ret_value;

        break;
    case STMT_VOID:
        if (fn->type->id != TYPE_VOID) {
            expr_res.kind = EXPR_ERROR;

            error(
                self, "function %s was expected to return %s, but it didn't",
                fn->name, fn->type->name
            );
        }

        break;
    default:
        break;
    }

finish:
    env_leave_fn(&self->env);
    env_restore_caller(&self->env);

    return expr_res;
}

static ExprResult
exec_expr(Interpreter *self, const AstNode *node, bool assigning) {
    ExprResult expr_res;

    switch (node->kind) {
    case AST_NODE_INTEGER:
        expr_res.kind = EXPR_VALUE;
        expr_res.val.type = self->types->builtin_int;
        expr_res.val.i = node->literal.i;

        return expr_res;
    case AST_NODE_STRING:
        return exec_string_literal(self, node);
    case AST_NODE_IDENT:
        return exec_identifier(self, node, assigning);
    case AST_NODE_UNARY:
        return exec_unary(self, node);
    case AST_NODE_BINARY:
        return exec_binary(self, node);
    case AST_NODE_SUFFIX:
        return exec_suffix(self, node);
    case AST_NODE_ARRAY_SUBSCRIPT:
        return exec_subscript(self, node);
    case AST_NODE_GROUPING:
        return exec_expr(self, node->grouping.expr, assigning);
    case AST_NODE_FN_CALL:
        return exec_fn_call(self, node);
    default:
        expr_res.kind = EXPR_ERROR;
        error(self, "invalid expression");

        return expr_res;
    }
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

static StmtResult exec_block(Interpreter *self, const AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, {0}};

    if (node->block.nodes.len == 0) {
        return stmt_res;
    }

    AstNode **nodes = node->block.nodes.data;

    env_enter_scope(&self->env);

    for (size_t i = 0; i < node->block.nodes.len; ++i) {
        StmtResult res = exec_node(self, nodes[i]);

        /* Support for break, continue and return statements */
        if (res.kind != STMT_VOID) {
            stmt_res = res;

            break;
        }

        if (self->halt) {
            break;
        }
    }

    env_leave_scope(&self->env);

    return stmt_res;
}

static StmtResult exec_var_decl(Interpreter *self, const AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, {0}};

    Type *type = process_type(self, node->var_decl.type);
    Value val = {self->types->error_type, {0}};
    bool defined = false;

    if (node->var_decl.rvalue) {
        ExprResult expr = exec_expr(self, node->var_decl.rvalue, false);
        STMT_RETURN_IF_ERROR(expr, EXPR_ERROR);

        val = expr.kind == EXPR_VAR ? expr.var->val : expr.val;

        if (!type_equal(val.type, type)) {
            stmt_res.kind = STMT_ERROR;
            error(
                self, "type mismatch: expected %s, found %s", type->name,
                val.type->name
            );

            return stmt_res;
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

    return stmt_res;
}

static StmtResult exec_fn_decl(Interpreter *self, AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, {0}};
    Type *type = process_type(self, node->fn_decl.type);
    AstNode *body = node->fn_decl.body;

    if (body) {
        /* steal ownership, so interpreter can later use the body and execute it
         * (the problem is that REPL destroys AST after every interpretation).
         */
        node->fn_decl.body = NULL;
    }

    Function *fn = malloc(sizeof(*fn));
    fn->type = type;
    fn->name = cstr_dup(node->fn_decl.name->ident.str.data);
    fn->body = body;
    fn->free_body = true;

    vec_init(&fn->params, sizeof(FnParam));

    const AstNode **params_nodes = node->fn_decl.params.data;
    for (size_t i = 0; i < node->fn_decl.params.len; ++i) {
        const AstNode *param_node = params_nodes[i];

        FnParam *param = vec_emplace(&fn->params);
        param->type = type;
        param->name = cstr_dup(param_node->param_decl.name->ident.str.data);
    }

    env_add_fn(&self->env, fn);

    return stmt_res;
}

static StmtResult exec_print(Interpreter *self, const AstNode *node) {
    ExprResult expr = exec_expr(self, node->kw_print.expr, false);
    STMT_RETURN_IF_ERROR(expr, EXPR_ERROR);

    Value val = expr.kind == EXPR_VAR ? expr.var->val : expr.val;

    fputs(val.s->data, stdout);

    StmtResult stmt_res = {STMT_VOID, {0}};

    return stmt_res;
}

static StmtResult exec_println(Interpreter *self, const AstNode *node) {
    ExprResult expr = exec_expr(self, node->kw_print.expr, false);
    STMT_RETURN_IF_ERROR(expr, EXPR_ERROR);

    Value val = expr.kind == EXPR_VAR ? expr.var->val : expr.val;

    fputs(val.s->data, stdout);
    fputc('\n', stdout);

    StmtResult stmt_res = {STMT_VOID, {0}};

    return stmt_res;
}

static StmtResult exec_exit(Interpreter *self, const AstNode *node) {
    ExprResult expr = exec_expr(self, node->kw_exit.expr, false);
    STMT_RETURN_IF_ERROR(expr, EXPR_ERROR);

    Value val = expr.kind == EXPR_VAR ? expr.var->val : expr.val;

    self->exit_code = (int)val.i;
    self->halt = true;

    StmtResult stmt_res = {STMT_VOID, {0}};

    return stmt_res;
}

static StmtResult exec_if(Interpreter *self, const AstNode *node) {
    ExprResult cond = exec_expr(self, node->kw_if.cond, false);
    STMT_RETURN_IF_ERROR(cond, EXPR_ERROR);

    Value cond_val = EXPR_GET_VALUE(cond);

    if (cond_val.i) {
        return exec_node(self, node->kw_if.body);
    } else if (node->kw_if.else_body) {
        return exec_node(self, node->kw_if.else_body);
    }

    StmtResult stmt_res = {STMT_VOID, {0}};

    return stmt_res;
}

static StmtResult exec_while(Interpreter *self, const AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, {0}};
    ExprResult cond = exec_expr(self, node->kw_while.cond, false);
    STMT_RETURN_IF_ERROR(cond, EXPR_ERROR);

    Value cond_val = EXPR_GET_VALUE(cond);

    while (cond_val.i) {
        StmtResult res = exec_node(self, node->kw_while.body);

        switch (res.kind) {
        case STMT_VOID:
        case STMT_CONTINUE:
            break;
        case STMT_ERROR:
        case STMT_RETURN:
            return res;
        case STMT_BREAK:
            return stmt_res;
        }

        cond = exec_expr(self, node->kw_while.cond, false);
        STMT_RETURN_IF_ERROR(cond, EXPR_ERROR);

        cond_val = EXPR_GET_VALUE(cond);
    }

    return stmt_res;
}

static StmtResult exec_for(Interpreter *self, const AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, {0}};

    AstNode *init = node->kw_for.init;
    const AstNode *cond = node->kw_for.cond;
    const AstNode *iter = node->kw_for.iter;
    AstNode *body = node->kw_for.body;

    env_enter_scope(&self->env);

    if (init) {
        exec_node(self, init);

        if (self->had_error) {
            goto failure;
        }
    }

    for (;;) {
        if (cond) {
            ExprResult expr = exec_expr(self, cond, false);

            if (expr.kind == EXPR_ERROR) {
                goto failure;
            }

            Value val = EXPR_GET_VALUE(expr);

            if (!val.i) {
                break;
            }
        }

        if (body) {
            StmtResult res = exec_node(self, body);

            switch (res.kind) {
            case STMT_VOID:
            case STMT_CONTINUE:
                break;
            case STMT_ERROR:
                goto failure;
            case STMT_RETURN:
                env_leave_scope(&self->env);

                return res;
            case STMT_BREAK:
                env_leave_scope(&self->env);

                return stmt_res;
            }
        }

        if (iter) {
            ExprResult expr = exec_expr(self, iter, false);

            if (expr.kind == EXPR_ERROR) {
                goto failure;
            }
        }
    }

    env_leave_scope(&self->env);

    return stmt_res;

failure:
    env_leave_scope(&self->env);

    stmt_res.kind = STMT_ERROR;

    return stmt_res;
}

static StmtResult exec_return(Interpreter *self, const AstNode *node) {
    ExprResult expr = exec_expr(self, node->kw_return.expr, false);
    STMT_RETURN_IF_ERROR(expr, EXPR_ERROR);

    StmtResult res;
    res.kind = STMT_RETURN;
    res.ret_value = EXPR_GET_VALUE(expr);

    return res;
}

static StmtResult exec_node(Interpreter *self, AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, {0}};

    switch (node->kind) {
    case AST_NODE_BLOCK:
        return exec_block(self, node);
    case AST_NODE_VAR_DECL:
        return exec_var_decl(self, node);
    case AST_NODE_FN_DECL:
        return exec_fn_decl(self, node);
    case AST_NODE_PRINT:
        return exec_print(self, node);
    case AST_NODE_PRINTLN:
        return exec_println(self, node);
    case AST_NODE_EXIT:
        return exec_exit(self, node);
    case AST_NODE_IF:
        return exec_if(self, node);
    case AST_NODE_WHILE:
        return exec_while(self, node);
    case AST_NODE_FOR:
        return exec_for(self, node);
    case AST_NODE_BREAK:
        stmt_res.kind = STMT_BREAK;

        break;
    case AST_NODE_CONTINUE:
        stmt_res.kind = STMT_CONTINUE;

        break;
    case AST_NODE_RETURN:
        return exec_return(self, node);
    default:
        exec_expr(self, node, false);

        break;
    }

    return stmt_res;
}

void interp_init(Interpreter *self, Ast *ast, TypeSystem *types) {
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
    AstNode **nodes = self->ast->nodes.data;

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
        exec_expr(self, ((const AstNode **)self->ast->nodes.data)[0], false);
    val = EXPR_GET_VALUE(expr);

    if (self->had_error) {
        val.type = self->types->error_type;
        self->exit_code = -1;
    }

    return val;
}
