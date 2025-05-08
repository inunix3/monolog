#include <monolog/ast.h>
#include <monolog/expr_result.h>
#include <monolog/stmt_result.h>
#include <monolog/interp.h>
#include <monolog/utils.h>

#include <assert.h>
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

#define STMT_RETURN_IF_INVALID_TYPE(_t)                                        \
    if (!(_t)) {                                                               \
        StmtResult __stmt_res = {STMT_ERROR, {0}};                             \
                                                                               \
        return __stmt_res;                                                     \
    }

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

static Value *expr_get_value(ExprResult *expr) {
    switch (expr->kind) {
    case EXPR_ERROR:
        return NULL;
    case EXPR_VALUE:
        return &expr->val;
    case EXPR_VAR:
        return &expr->var->val;
    case EXPR_REF:
        return expr->ref;
    }
}

static bool clone_value(Interpreter *self, Value *dest, const Value *src);

static bool make_opt(Interpreter *self, Value *dest, const Value *val) {
    dest->opt.val = NULL;

    if (val && val->type->id != TYPE_NIL) {
        Value *owned_val = vec_emplace(&self->opts);

        if (!owned_val) {
            return false;
        }

        dest->opt.val = owned_val;

        if (val->type->id == TYPE_OPTION && type_equal(val->type, dest->type)) {
            if (!val->opt.val) {
                vec_pop(&self->opts);
                dest->opt.val = NULL;
            } else {
                owned_val->type = val->opt.val->type;
                return clone_value(self, dest->opt.val, val->opt.val);
            }
        } else {
            owned_val->type = val->type;

            return clone_value(self, dest->opt.val, val);
        }
    }

    return true;
}

static bool clone_value(Interpreter *self, Value *dest, const Value *src);

static bool clone_list(Interpreter *self, Value *dest, const Value *src) {
    assert(dest->type);
    assert(type_equal(dest->type, src->type));

    Vector *values = vec_emplace(&self->lists);
    vec_init(values, sizeof(Value));

    dest->list.values = values;

    const Value *src_values = src->list.values->data;

    for (size_t i = 0; i < src->list.values->len; ++i) {
        Value *elem = vec_emplace(values);

        if (!clone_value(self, elem, &src_values[i])) {
            return false;
        }
    }

    return true;
}

static bool clone_value(Interpreter *self, Value *dest, const Value *src) {
    switch (src->type->id) {
    case TYPE_ERROR:
        dest->type = self->types->error_type;

        break;
    case TYPE_INT:
        dest->type = self->types->builtin_int;
        dest->i = src->i;

        break;
    case TYPE_STRING:
        dest->type = self->types->builtin_string;
        dest->s = vec_emplace(&self->strings);

        return str_dup_n(dest->s, src->s->data, src->s->len);
    case TYPE_VOID:
        dest->type = self->types->builtin_void;

        break;
    case TYPE_LIST:
        return clone_list(self, dest, src);
    case TYPE_OPTION:
        return make_opt(self, dest, src);
    case TYPE_NIL:
        *dest = *src;

        break;
    }

    return true;
}

static bool value_equal(const Value *v1, const Value *v2) {
    if (!type_equal(v1->type, v2->type)) {
        return false;
    }

    switch (v1->type->id) {
    case TYPE_ERROR:
    case TYPE_VOID:
    case TYPE_NIL:
        break;
    case TYPE_INT:
        return v1->i == v2->i;
    case TYPE_STRING:
        return str_equal(v1->s, v2->s);
    case TYPE_LIST:
        /* lists cannot be compared */

        return false;
    case TYPE_OPTION:
        if (v1->opt.val && v2->opt.val) {
            return value_equal(v1->opt.val, v2->opt.val);
        } else if (!v1->opt.val && !v2->opt.val) {
            return true;
        } else {
            return false;
        }
    }

    return true;
}

static Value
exec_binary_int(Interpreter *self, TokenKind op, Value *v1, const Value *v2) {
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

static Value exec_binary_string(
    Interpreter *self, TokenKind op, Value *v1, const Value *v2
) {
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

static Value exec_binary_option(
    Interpreter *self, TokenKind op, const Value *v1, const Value *v2
) {
    Value val = {0};

    switch (op) {
    case TOKEN_OP_EQUAL:
        val.type = self->types->builtin_int;

        if (v2->type->id == TYPE_NIL) {
            val.i = v1->opt.val == NULL;
        } else if (type_equal(v1->type, v2->type)) {
            if (v1->opt.val && v2->opt.val) {
                val.i = value_equal(v1->opt.val, v2->opt.val);
            } else if (!v1->opt.val && !v2->opt.val) {
                val.i = 1;
            } else {
                val.i = 0;
            }
        }

        break;
    case TOKEN_OP_NOT_EQUAL:
        val.type = self->types->builtin_int;

        if (v2->type->id == TYPE_NIL) {
            val.i = v1->opt.val != NULL;
        } else if (type_equal(v1->type, v2->type)) {
            if (v1->opt.val && v2->opt.val) {
                val.i = value_equal(v1->opt.val, v2->opt.val);
            } else if (!v1->opt.val && !v2->opt.val) {
                val.i = 0;
            } else {
                val.i = 1;
            }
        }

        break;
    default:
        break;
    }

    return val;
}

static Value exec_binary_nil(
    Interpreter *self, TokenKind op, const Value *v1, const Value *v2
) {
    (void)v1;
    Value val = {0};

    switch (op) {
    case TOKEN_OP_EQUAL:
        val.type = self->types->builtin_int;

        if (v2->type->id == TYPE_NIL) {
            val.i = 1;
        } else if (v2->type->id == TYPE_OPTION) {
            val.i = v2->opt.val == NULL;
        }

        break;
    case TOKEN_OP_NOT_EQUAL:
        val.type = self->types->builtin_int;

        if (v2->type->id == TYPE_NIL) {
            val.i = 0;
        } else if (v2->type->id == TYPE_OPTION) {
            val.i = v2->opt.val != NULL;
        }

        break;
    default:
        break;
    }

    return val;
}

static ExprResult
exec_expr(Interpreter *self, const AstNode *node, bool assigning);

static Value exec_unary_int(Interpreter *self, TokenKind op, const Value *v1) {
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

        StrBuf *str = vec_emplace(&self->strings);
        str_init_n(str, len);
        snprintf(str->data, len + 1, "%" PRId64, v1->i);

        val.type = self->types->builtin_string;
        val.s = str;

        break;
    }
    default:
        break;
    }

    return val;
}

static Value
exec_unary_string(Interpreter *self, TokenKind op, const Value *v1) {
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

static Value
exec_unary_list(Interpreter *self, TokenKind op, const Value *v1) {
    Value val = {self->types->error_type, {0}};

    switch (op) {
    case TOKEN_OP_HASHTAG:
        val.type = self->types->builtin_int;
        val.i = (Int)v1->list.values->len;

        break;
    default:
        break;
    }

    return val;
}

static Value *
exec_unary_option(Interpreter *self, TokenKind op, const Value *v1) {
    Value *val = NULL;

    switch (op) {
    case TOKEN_OP_MUL:
        if (v1->opt.val) {
            val = v1->opt.val;
        } else {
            val = NULL;
            error(self, "tried to dereference an empty option");
        }

        break;
    default:
        break;
    }

    return val;
}

static ExprResult exec_unary(Interpreter *self, const AstNode *node) {
    ExprResult expr = exec_expr(self, node->unary.right, false);
    EXPR_RETURN_IF_ERROR(expr, EXPR_ERROR);

    const Value *expr_val = expr_get_value(&expr);
    ExprResult expr_res;

    switch (expr_val->type->id) {
    case TYPE_INT:
        expr_res.val = exec_unary_int(self, node->unary.op, expr_val);

        break;
    case TYPE_STRING:
        expr_res.val = exec_unary_string(self, node->unary.op, expr_val);

        break;
    case TYPE_LIST:
        expr_res.val = exec_unary_list(self, node->unary.op, expr_val);

        break;
    case TYPE_OPTION:
    case TYPE_NIL: {
        Value *val = exec_unary_option(self, node->unary.op, expr_val);

        if (!val) {
            expr_res.kind = EXPR_ERROR;

            return expr_res;
        } else {
            expr_res.kind = EXPR_REF;
            expr_res.ref = val;
        }

        return expr_res;
    }
    default:
        break;
    }

    if (expr_res.val.type->id == TYPE_ERROR) {
        expr_res.kind = EXPR_ERROR;
    } else if (expr.kind == EXPR_VAR && (node->unary.op == TOKEN_OP_INC ||
                                         node->unary.op == TOKEN_OP_DEC)) {
        Value temp = expr_res.val;

        expr_res.kind = EXPR_VAR;
        expr_res.var = expr.var;
        expr_res.var->val = temp;
    } else {
        expr_res.kind = EXPR_VALUE;
    }

    return expr_res;
}

static bool
implicitly_clone_value(Interpreter *self, Value *dest, const Value *src) {
    if (dest->type->id == TYPE_OPTION) {
        return make_opt(self, dest, src);
    }

    return clone_value(self, dest, src);
}

static ExprResult
assign_var(Interpreter *self, Variable *var, ExprResult expr) {
    (void)self;

    Value *expr_val = expr_get_value(&expr);
    ExprResult expr_res = {0};

    if (!implicitly_clone_value(self, &var->val, expr_val)) {
        expr_res.kind = EXPR_ERROR;
    } else {
        expr_res.kind = EXPR_VALUE;
        expr_res.val = var->val;
    }

    return expr_res;
}

static ExprResult assign_ref(Interpreter *self, Value *val, ExprResult expr) {
    (void)self;

    Value *expr_val = expr_get_value(&expr);

    implicitly_clone_value(self, val, expr_val);

    ExprResult expr_res;
    expr_res.kind = EXPR_REF;
    expr_res.ref = val;

    return expr_res;
}

static ExprResult
assign_expr(Interpreter *self, ExprResult expr1, ExprResult expr2) {
    switch (expr1.kind) {
    case EXPR_VAR:
        return assign_var(self, expr1.var, expr2);
    case EXPR_REF:
        return assign_ref(self, expr1.ref, expr2);
    default:
        break;
    }

    ExprResult expr_res;
    expr_res.kind = EXPR_ERROR;

    return expr_res;
}

static bool expr_assignable(ExprResult expr) {
    switch (expr.kind) {
    case EXPR_ERROR:
    case EXPR_VALUE:
        return false;
    case EXPR_VAR:
    case EXPR_REF:
        return true;
    }
}

static ExprResult exec_binary(Interpreter *self, const AstNode *node) {
    bool assigning = node->binary.op == TOKEN_OP_ASSIGN;

    ExprResult expr1 = exec_expr(self, node->binary.left, assigning);
    EXPR_RETURN_IF_ERROR(expr1, EXPR_ERROR);

    ExprResult expr2 = exec_expr(self, node->binary.right, false);
    EXPR_RETURN_IF_ERROR(expr2, EXPR_ERROR);

    if (expr_assignable(expr1) && assigning) {
        return assign_expr(self, expr1, expr2);
    }

    Value *v1 = expr_get_value(&expr1);
    const Value *v2 = expr_get_value(&expr2);

    ExprResult expr;

    switch (v1->type->id) {
    case TYPE_INT:
        expr.kind = EXPR_VALUE;
        expr.val = exec_binary_int(self, node->binary.op, v1, v2);

        break;
    case TYPE_STRING:
        expr.kind = EXPR_VALUE;
        expr.val = exec_binary_string(self, node->binary.op, v1, v2);

        break;
    case TYPE_NIL:
        expr.kind = EXPR_VALUE;
        expr.val = exec_binary_nil(self, node->binary.op, v1, v2);

        break;
    case TYPE_OPTION:
        expr.kind = EXPR_VALUE;
        expr.val = exec_binary_option(self, node->binary.op, v1, v2);

        break;
    default:
        expr.kind = EXPR_ERROR;
        error(
            self, "unsupported binary operation for %s and %s", v1->type->name,
            v2->type->name
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

    StrBuf *str = vec_emplace(&self->strings);
    str_dup_n(str, node->literal.str.data, node->literal.str.len);

    expr_res.val.type = self->types->builtin_string;
    expr_res.val.s = str;

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

    if (!assigning && !var->defined && !var->is_param &&
        var->type->id != TYPE_LIST) {
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

static ExprResult
exec_list_subscript(Interpreter *self, const Value *val, Int idx) {
    ExprResult expr_res = {0};

    if ((size_t)idx >= val->list.values->len) {
        expr_res.kind = EXPR_ERROR;
        error(
            self,
            "list index out of range: %" PRId64 " but the list size is %zu",
            idx, val->list.values->len
        );

        return expr_res;
    }

    Value *values = val->list.values->data;

    expr_res.kind = EXPR_REF;
    expr_res.val.type = val->type->list_type.type;
    expr_res.ref = &values[idx];

    return expr_res;
}

static ExprResult exec_subscript(Interpreter *self, const AstNode *node) {
    ExprResult left_expr = exec_expr(self, node->subscript.left, false);
    EXPR_RETURN_IF_ERROR(left_expr, EXPR_ERROR);

    ExprResult idx_expr = exec_expr(self, node->subscript.expr, false);
    EXPR_RETURN_IF_ERROR(idx_expr, EXPR_ERROR);

    ExprResult expr_res = {0};
    Value *left_val = expr_get_value(&left_expr);
    Value *idx = expr_get_value(&idx_expr);

    switch (left_val->type->id) {
    case TYPE_STRING:
        return exec_string_subscript(self, left_val->s, idx->i);
    case TYPE_LIST:
        return exec_list_subscript(self, left_val, idx->i);
    default:
        expr_res.kind = EXPR_ERROR;
        error(self, "unsupported type %s for indexing", left_val->type->name);

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

        arg->val.type = param->type;
        if (!implicitly_clone_value(self, &arg->val, expr_get_value(&expr))) {
            return false;
        }

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

    Scope *saved_scope = self->env.curr_scope;
    Scope *saved_caller = self->env.caller_scope;

    env_enter_fn(&self->env, fn);

    const AstNode **args = node->fn_call.values.data;

    if (!fill_fn_params_values(self, args)) {
        expr_res.kind = EXPR_ERROR;

        goto finish;
    }

    self->env.caller_scope = saved_scope;

    StmtResult body_res = exec_node(self, fn->body);

    switch (body_res.kind) {
    case STMT_ERROR:
        expr_res.kind = EXPR_ERROR;

        break;
    case STMT_RETURN:
        expr_res.kind = EXPR_VALUE;
        expr_res.val.type = fn->type;

        if (!implicitly_clone_value(self, &expr_res.val, &body_res.ret_value)) {
            expr_res.kind = EXPR_ERROR;

            goto finish;
        }

        break;
    case STMT_VOID:
        if (fn->type->id != TYPE_VOID) {
            expr_res.kind = EXPR_ERROR;

            error(
                self, "function %s was expected to return %s, but it didn't",
                fn->name, fn->type->name
            );
        } else {
            expr_res.kind = EXPR_VALUE;
            expr_res.val.type = self->types->builtin_void;
        }

        break;
    default:
        break;
    }

finish:
    env_leave_fn(&self->env);
    self->env.curr_scope = saved_scope;
    self->env.caller_scope = saved_caller;

    return expr_res;
}

static ExprResult exec_nil(const Interpreter *self) {
    ExprResult expr_res = {0};
    expr_res.kind = EXPR_VALUE;
    expr_res.val.type = self->types->nil_type;

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
    case AST_NODE_NIL:
        return exec_nil(self);
    case AST_NODE_UNARY:
        return exec_unary(self, node);
    case AST_NODE_BINARY:
        return exec_binary(self, node);
    case AST_NODE_SUFFIX:
        return exec_suffix(self, node);
    case AST_NODE_SUBSCRIPT:
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
    case AST_NODE_LIST_TYPE: {
        Type *contained_type = process_type(self, node->list_type.type);

        if (!contained_type) {
            return NULL;
        }

        Type list = {TYPE_LIST, NULL, {0}};
        list.list_type.type = contained_type;

        return type_system_register(self->types, &list);
    }
    case AST_NODE_OPTION_TYPE: {
        Type *contained_type = process_type(self, node->opt_type.type);

        if (!contained_type) {
            return NULL;
        }

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

static bool new_value(Interpreter *self, Value *val, Type *type) {
    memset(val, 0, sizeof(*val));
    val->type = type;

    if (type->id == TYPE_LIST) {
        Vector *values = vec_emplace(&self->lists);
        vec_init(values, sizeof(Value));
        val->list.values = values;
    }

    return true;
}

static StmtResult exec_var_decl(Interpreter *self, const AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, {0}};

    Type *type = process_type(self, node->var_decl.type);
    STMT_RETURN_IF_INVALID_TYPE(type);

    Value val = {type, {0}};
    bool defined = false;

    if (node->var_decl.rvalue) {
        ExprResult expr = exec_expr(self, node->var_decl.rvalue, false);
        STMT_RETURN_IF_ERROR(expr, EXPR_ERROR);

        Value *expr_val = expr_get_value(&expr);

        if (!type_can_implicitly_convert(expr_val->type, type)) {
            stmt_res.kind = STMT_ERROR;
            error(
                self, "type mismatch: expected %s, found %s", type->name,
                expr_val->type->name
            );

            return stmt_res;
        }

        if (!implicitly_clone_value(self, &val, expr_val)) {
            stmt_res.kind = STMT_ERROR;

            return stmt_res;
        }

        defined = true;
    } else if (!new_value(self, &val, type)) {
        stmt_res.kind = STMT_ERROR;

        return stmt_res;
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
    STMT_RETURN_IF_INVALID_TYPE(type);

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
        Type *param_type = process_type(self, param_node->param_decl.type);
        STMT_RETURN_IF_INVALID_TYPE(type);

        FnParam *param = vec_emplace(&fn->params);
        param->type = param_type;
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

    Value *cond_val = expr_get_value(&cond);

    if (cond_val->i) {
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

    Value *cond_val = expr_get_value(&cond);

    while (cond_val->i) {
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

        if (self->halt) {
            return stmt_res;
        }

        cond = exec_expr(self, node->kw_while.cond, false);
        STMT_RETURN_IF_ERROR(cond, EXPR_ERROR);

        cond_val = expr_get_value(&cond);
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

            Value *val = expr_get_value(&expr);

            if (!val->i) {
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

            if (self->halt) {
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
    StmtResult res = {0};
    res.kind = STMT_RETURN;

    if (node->kw_return.expr) {
        ExprResult expr = exec_expr(self, node->kw_return.expr, false);
        STMT_RETURN_IF_ERROR(expr, EXPR_ERROR);

        res.ret_value = *expr_get_value(&expr);
    } else {
        res.ret_value.type = self->types->builtin_void;
    }

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
    vec_init(&self->lists, sizeof(Vector));
    vec_init(&self->opts, sizeof(Value));

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

    Vector *lists = self->lists.data;

    for (size_t i = 0; i < lists->len; ++i) {
        vec_deinit(&lists[i]);
    }

    vec_deinit(&self->lists);
    vec_deinit(&self->strings);
    vec_deinit(&self->opts);
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
    Value val = {0};

    if (self->ast->nodes.len == 0) {
        val.type = self->types->builtin_void;

        return val;
    }

    ExprResult expr =
        exec_expr(self, ((const AstNode **)self->ast->nodes.data)[0], false);
    Value *expr_val = expr_get_value(&expr);

    if (!expr_val) {
        val.type = self->types->error_type;
    } else {
        val = *expr_val;
    }

    if (self->had_error) {
        val.type = self->types->error_type;
        self->exit_code = -1;
    }

    return val;
}
