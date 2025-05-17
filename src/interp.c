/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/ast.h>
#include <monolog/builtin_funcs.h>
#include <monolog/expr_result.h>
#include <monolog/interp.h>
#include <monolog/stmt_result.h>
#include <monolog/utils.h>

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CASE_BINARY_INT(_op, _v1, _v2)                                         \
    val.i = (_v1)->i _op(_v2)->i;                                              \
    break

#define CASE_UNARY_INT(_op, _v1)                                               \
    val.i = _op((_v1)->i);                                                     \
    break

#define EXPR_RETURN_ON_HALT(_node)                                             \
    if (self->had_error && self->halt) {                                       \
        ExprResult __expr_res = {EXPR_ERROR, .node = _node, {0}};              \
                                                                               \
        return __expr_res;                                                     \
    } else if (self->halt) {                                                   \
        ExprResult __expr_res = {EXPR_HALT, .node = _node, {0}};               \
                                                                               \
        return __expr_res;                                                     \
    }

#define STMT_RETURN_ON_HALT(_node)                                             \
    if (self->had_error && self->halt) {                                       \
        StmtResult __stmt_res = {STMT_ERROR, .node = _node, {0}};              \
                                                                               \
        return __stmt_res;                                                     \
    } else if (self->halt) {                                                   \
        StmtResult __stmt_res = {STMT_HALT, .node = _node, {0}};               \
                                                                               \
        return __stmt_res;                                                     \
    }

#define UNREACHABLE()                                                          \
    fprintf(                                                                   \
        stderr,                                                                \
        "reached unreachable code (must be a bug in interpreter's code): "     \
        "%s:%d\n",                                                             \
        __FILE__, __LINE__                                                     \
    );                                                                         \
    abort()

#define INPUT_BUFSIZE 1024

static void clear_stdin(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

static void
error(Interpreter *self, SourceInfo src_info, const char *fmt, ...) {
    self->had_error = true;
    self->halt = true;

    if (self->log_errors) {
        fprintf(stderr, "%d:%d: runtime error: ", src_info.line, src_info.col);

        va_list vargs;
        va_start(vargs, fmt);
        vfprintf(stderr, fmt, vargs);
        va_end(vargs);

        fputc('\n', stderr);
    }
}

static Value expr_get_value(const Interpreter *self, ExprResult *expr) {
    Value err_val = {self->types->error_type, NULL, {0}};

    switch (expr->kind) {
    case EXPR_ERROR:
    case EXPR_HALT:
        return err_val;
    case EXPR_VALUE:
        return expr->val;
    case EXPR_VAR:
        return expr->var->val;
    case EXPR_REF:
        return *expr->ref;
    case EXPR_CHAR_REF: {
        Value val = {self->types->builtin_int, self->env.curr_scope, {0}};
        val.i = *expr->char_ref;

        return val;
    }
    }

    /* satisfy gcc */
    return err_val;
}

static void expr_set_value(ExprResult *expr, const Value *val) {
    switch (expr->kind) {
    case EXPR_ERROR:
    case EXPR_HALT:
        break;
    case EXPR_VALUE:
        expr->val = *val;

        break;
    case EXPR_VAR:
        expr->var->val = *val;

        break;
    case EXPR_REF:
        *expr->ref = *val;

        break;
    case EXPR_CHAR_REF:
        assert(val->type->id == TYPE_INT);

        *expr->char_ref = (char) val->i;

        break;
    }
}

static void clone_value(
    Interpreter *self, Value *dest, Type *dest_type, const Value *src,
    Scope *scope
);

static void make_opt(
    Interpreter *self, Value *dest, Type *dest_type, const Value *val,
    Scope *scope
) {
    assert(type_convertable(val->type, dest_type));

    dest->type = dest_type;
    dest->scope = scope;
    dest->opt.val = NULL;

    if (!val || val->type->id == TYPE_NIL) {
        return;
    }

    Type *inner_type = dest->type->opt_type.type;
    Value *inner = scope_new_value(scope, inner_type);

    if (val->type->id == TYPE_OPTION && type_equal(val->type, dest_type)) {
        if (val->opt.val) {
            assert(type_equal(val->opt.val->type, inner_type));

            clone_value(self, inner, inner_type, val->opt.val, scope);
        } else {
            inner = NULL;
        }
    } else {
        assert(type_equal(val->type, inner_type));

        clone_value(self, inner, inner_type, val, scope);
    }

    dest->opt.val = inner;
}

static void implicitly_clone_value(
    Interpreter *self, Value *dest, const Value *src, Scope *scope
) {
    if (dest->type->id == TYPE_OPTION) {
        make_opt(self, dest, dest->type, src, scope);
    } else {
        clone_value(self, dest, dest->type, src, scope);
    }
}

static Value shallowly_clone_value(Interpreter *self, const Value *val) {
    UNUSED(self);

    Value arg = {val->type, val->scope, {0}};

    switch (val->type->id) {
    case TYPE_INT:
        arg.i = val->i;

        break;
    case TYPE_STRING:
        arg.s = val->s;

        break;
    case TYPE_OPTION:
        arg.opt.val = val->opt.val;

        break;
    case TYPE_LIST:
        arg.list.values = val->list.values;

        break;
    case TYPE_NIL:
    case TYPE_VOID:
        break;
    case TYPE_ERROR:
        UNREACHABLE();
    }

    return arg;
}

static void shallowly_implicitly_clone_value(
    Interpreter *self, Value *dest, const Value *src, Scope *scope
) {
    if (dest->type->id == TYPE_OPTION && src->type->id != TYPE_OPTION &&
        type_convertable(src->type, dest->type)) {
        make_opt(self, dest, dest->type, src, scope);
    } else {
        *dest = shallowly_clone_value(self, src);
    }
}

static bool
clone_list(Interpreter *self, Value *dest, const Value *src, Scope *scope) {
    assert(dest->type);
    assert(type_convertable(dest->type, src->type));

    Vector *values = scope_new_list(scope);

    if (dest->list.values) {
        vec_deinit(dest->list.values);
    }

    dest->list.values = values;

    const Value *src_values = src->list.values->data;

    for (size_t i = 0; i < src->list.values->len; ++i) {
        Value *elem = vec_emplace(values);
        elem->type = src_values[i].type;

        implicitly_clone_value(self, elem, &src_values[i], scope);
    }

    return true;
}

static void clone_value(
    Interpreter *self, Value *dest, Type *dest_type, const Value *src,
    Scope *scope
) {
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
        dest->s = scope_new_string(scope);
        str_dup_n(dest->s, src->s->data, src->s->len);

        break;
    case TYPE_VOID:
        dest->type = self->types->builtin_void;

        break;
    case TYPE_LIST:
        clone_list(self, dest, src, scope);

        break;
    case TYPE_OPTION:
        make_opt(self, dest, dest_type, src, scope);

        break;
    case TYPE_NIL:
        *dest = *src;

        break;
    }
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

static void new_value(Interpreter *self, Value *val, Type *type, Scope *scope) {
    UNUSED(self);

    memset(val, 0, sizeof(*val));
    val->type = type;
    val->scope = scope;

    if (type->id == TYPE_LIST) {
        Vector *values = scope_new_list(scope);
        val->list.values = values;
    } else if (type->id == TYPE_STRING) {
        StrBuf *str = scope_new_string(scope);
        str_init_n(str, 0);

        val->s = str;
    }
}

static Variable *new_var_shallow(
    Interpreter *self, Type *type, const char *name, const Value *val
) {
    UNUSED(self);

    Variable *var = mem_alloc(sizeof(*var));

    var->type = type;
    var->name = cstr_dup(name);
    var->val = *val;
    var->scope = self->env.curr_scope;

    return var;
}

static Value
exec_binary_int(Interpreter *self, Token op, Value *v1, const Value *v2) {
    Value val = {self->types->builtin_int, self->env.curr_scope, {0}};

    switch (op.kind) {
    case TOKEN_PLUS:
        CASE_BINARY_INT(+, v1, v2);
    case TOKEN_MINUS:
        CASE_BINARY_INT(-, v1, v2);
    case TOKEN_MUL:
        CASE_BINARY_INT(*, v1, v2);
    case TOKEN_DIV:
        if (v2->i == 0) {
            error(self, op.src_info, "division by zero");
            val.type = self->types->error_type;

            return val;
        }

        CASE_BINARY_INT(/, v1, v2);
    case TOKEN_MOD:
        if (v2->i == 0) {
            error(self, op.src_info, "division by zero");
            val.type = self->types->error_type;

            return val;
        }

        CASE_BINARY_INT(%, v1, v2);
    case TOKEN_EQUAL:
        CASE_BINARY_INT(==, v1, v2);
    case TOKEN_NOT_EQUAL:
        CASE_BINARY_INT(!=, v1, v2);
    case TOKEN_LESS:
        CASE_BINARY_INT(<, v1, v2);
    case TOKEN_GREATER:
        CASE_BINARY_INT(>, v1, v2);
    case TOKEN_LESS_EQUAL:
        CASE_BINARY_INT(<=, v1, v2);
    case TOKEN_GREATER_EQUAL:
        CASE_BINARY_INT(>=, v1, v2);
    case TOKEN_AND:
        CASE_BINARY_INT(&&, v1, v2);
    case TOKEN_OR:
        CASE_BINARY_INT(||, v1, v2);
    default:
        UNREACHABLE();
    }

    return val;
}

static Value exec_binary_string(
    Interpreter *self, TokenKind op, Value *v1, const Value *v2
) {
    Value val = *v1;

    switch (op) {
    case TOKEN_PLUS:
        val.type = self->types->builtin_string;
        str_cat(val.s, v2->s);

        break;
    case TOKEN_EQUAL:
        val.type = self->types->builtin_int;
        val.i = str_equal(v1->s, v2->s);

        break;
    case TOKEN_NOT_EQUAL:
        val.type = self->types->builtin_int;
        val.i = !str_equal(v1->s, v2->s);

        break;
    default:
        UNREACHABLE();
    }

    return val;
}

static Value exec_binary_option(
    Interpreter *self, TokenKind op, const Value *v1, const Value *v2
) {
    Value val = {0};

    switch (op) {
    case TOKEN_EQUAL:
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
    case TOKEN_NOT_EQUAL:
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
        UNREACHABLE();
    }

    return val;
}

static Value exec_binary_nil(
    Interpreter *self, TokenKind op, const Value *v1, const Value *v2
) {
    UNUSED(v1);
    Value val = {0};

    switch (op) {
    case TOKEN_EQUAL:
        val.type = self->types->builtin_int;

        if (v2->type->id == TYPE_NIL) {
            val.i = 1;
        } else if (v2->type->id == TYPE_OPTION) {
            val.i = v2->opt.val == NULL;
        }

        break;
    case TOKEN_NOT_EQUAL:
        val.type = self->types->builtin_int;

        if (v2->type->id == TYPE_NIL) {
            val.i = 0;
        } else if (v2->type->id == TYPE_OPTION) {
            val.i = v2->opt.val != NULL;
        }

        break;
    default:
        UNREACHABLE();
    }

    return val;
}

static Value
exec_binary_list(Interpreter *self, Token op, Value *v1, const Value *v2) {
    Value val = *v1;
    Type *inner_type = v1->type->list_type.type;
    val.type = inner_type;

    Vector *values = v1->list.values;

    switch (op.kind) {
    case TOKEN_ADD_ASSIGN: {
        assert(type_equal(inner_type, v2->type));

        Value *elem = vec_emplace(values);
        elem->type = inner_type;

        implicitly_clone_value(self, elem, v2, v1->scope);

        break;
    }
    case TOKEN_SUB_ASSIGN:
        assert(type_equal(self->types->builtin_int, v2->type));

        if (v2->i < 0) {
            error(self, op.src_info, "the right side cannot be negative");
            val.type = self->types->error_type;

            break;
        }

        if (values->len == 0) {
            error(self, op.src_info, "cannot pop more on empty list");
            val.type = self->types->error_type;

            break;
        }

        for (Int i = 0; i < v2->i; ++i) {
            vec_pop(values);
        }

        break;
    case TOKEN_HASHTAG_ASSIGN: {
        assert(type_equal(self->types->builtin_int, v2->type));

        if (v2->i < 0) {
            error(self, op.src_info, "the right side cannot be negative");
            val.type = self->types->error_type;

            break;
        }

        size_t size = (size_t) v2->i;

        if (size > values->len) {
            while (values->len != size) {
                Value *elem = vec_emplace(val.list.values);
                new_value(
                    self, elem, inner_type, self->env.curr_scope
                );
            }
        } else if (size < values->len) {
            while (values->len != size) {
                vec_pop(values);
            }
        }

        break;
    }
    default:
        UNREACHABLE();
    }

    return val;
}

static ExprResult
exec_expr(Interpreter *self, const AstNode *node, bool assigning);

static Value exec_unary_int(Interpreter *self, TokenKind op, const Value *v1) {
    Value val = {self->types->builtin_int, self->env.curr_scope, {0}};

    switch (op) {
    case TOKEN_INC:
        val.i = v1->i + 1;

        break;
    case TOKEN_DEC:
        val.i = v1->i - 1;

        break;
    case TOKEN_PLUS:
        CASE_UNARY_INT(+, v1);
    case TOKEN_MINUS:
        CASE_UNARY_INT(-, v1);
    case TOKEN_EXCL:
        val.i = !v1->i;

        break;
    case TOKEN_DOLAR: {
        StrBuf *str = scope_new_string(self->env.curr_scope);

        size_t len = (size_t) snprintf(NULL, 0, "%" PRId64, v1->i);
        str_init_n(str, len);
        snprintf(str->data, len + 1, "%" PRId64, v1->i);

        val.type = self->types->builtin_string;
        val.s = str;

        break;
    }
    default:
        UNREACHABLE();
    }

    return val;
}

static Value
exec_unary_string(Interpreter *self, TokenKind op, const Value *v1) {
    Value val = {self->types->builtin_string, self->env.curr_scope, {0}};

    switch (op) {
    case TOKEN_HASHTAG:
        val.type = self->types->builtin_int;
        val.i = (Int) v1->s->len;

        break;
    default:
        UNREACHABLE();
    }

    return val;
}

static Value exec_unary_list(Interpreter *self, TokenKind op, const Value *v1) {
    Value val = {self->types->error_type, self->env.curr_scope, {0}};

    switch (op) {
    case TOKEN_HASHTAG:
        val.type = self->types->builtin_int;
        val.i = (Int) v1->list.values->len;

        break;
    default:
        UNREACHABLE();
    }

    return val;
}

static Value *exec_unary_option(Interpreter *self, Token op, const Value *v1) {
    Value *val = NULL;

    switch (op.kind) {
    case TOKEN_MUL:
        if (v1->opt.val) {
            val = v1->opt.val;
        } else {
            val = NULL;
            error(self, op.src_info, "tried to dereference an empty option");
        }

        break;
    default:
        UNREACHABLE();
    }

    return val;
}

static ExprResult exec_unary(Interpreter *self, const AstNode *node) {
    ExprResult expr = exec_expr(self, node->unary.right, false);
    EXPR_RETURN_ON_HALT(expr.node);

    Value expr_val = expr_get_value(self, &expr);
    ExprResult expr_res = {EXPR_VALUE, .node = node, {0}};
    Value new_val = {0};

    switch (expr_val.type->id) {
    case TYPE_INT:
        new_val = exec_unary_int(self, node->unary.op.kind, &expr_val);
        expr_set_value(&expr_res, &new_val);

        break;
    case TYPE_STRING:
        new_val = exec_unary_string(self, node->unary.op.kind, &expr_val);
        expr_set_value(&expr_res, &new_val);

        break;
    case TYPE_LIST:
        new_val = exec_unary_list(self, node->unary.op.kind, &expr_val);
        expr_set_value(&expr_res, &new_val);

        break;
    case TYPE_OPTION:
    case TYPE_NIL: {
        Value *inner_val = exec_unary_option(self, node->unary.op, &expr_val);

        if (!inner_val) {
            expr_res.kind = EXPR_ERROR;

            return expr_res;
        } else {
            expr_res.kind = EXPR_REF;
            expr_res.ref = inner_val;
        }

        return expr_res;
    }
    default:
        break;
    }

    if (expr_res.val.type->id == TYPE_ERROR) {
        expr_res.kind = EXPR_ERROR;
    } else if (expr.kind == EXPR_VAR && (node->unary.op.kind == TOKEN_INC ||
                                         node->unary.op.kind == TOKEN_DEC)) {
        Value temp = expr_res.val;

        expr_res.kind = EXPR_VAR;
        expr_res.var = expr.var;
        expr_res.var->val = temp;
    } else {
        expr_res.kind = EXPR_VALUE;
    }

    return expr_res;
}

static ExprResult
assign_var(Interpreter *self, Variable *var, ExprResult expr) {
    UNUSED(self);

    Value rval = expr_get_value(self, &expr);
    ExprResult expr_res = {0};

    Value *val = &var->val;
    bool in_different_scope = var->scope != self->env.curr_scope;

    if (in_different_scope) {
        val = scope_new_value(var->scope, var->type);
    }

    switch (expr.kind) {
    case EXPR_HALT:
        expr_res.kind = EXPR_HALT;

        return expr_res;
    case EXPR_ERROR:
        expr_res.kind = EXPR_ERROR;

        return expr_res;
    case EXPR_VALUE:
        if (in_different_scope) {
            implicitly_clone_value(self, val, &rval, var->scope);
        } else {
            shallowly_implicitly_clone_value(self, val, &rval, var->scope);
        }

        break;
    case EXPR_VAR:
    case EXPR_REF:
    case EXPR_CHAR_REF:
        implicitly_clone_value(self, val, &rval, var->scope);

        break;
    }

    if (in_different_scope) {
        var->val = *val;
    }

    expr_res.kind = EXPR_VALUE;
    expr_res.val = var->val;

    return expr_res;
}

static ExprResult assign_ref(Interpreter *self, Value *val, ExprResult expr) {
    ExprResult expr_res = {0};
    Value expr_val = expr_get_value(self, &expr);

    implicitly_clone_value(self, val, &expr_val, val->scope);

    expr_res.kind = EXPR_REF;
    expr_res.ref = val;

    return expr_res;
}

static ExprResult
assign_char_ref(Interpreter *self, char *dest, ExprResult expr) {
    ExprResult expr_res = {0};
    Value expr_val = expr_get_value(self, &expr);

    /* clang-format off */
    assert(type_convertable(expr_val.type, self->types->builtin_int));
    /* clang-format on */

    *dest = (char) expr_val.i;

    expr_res.kind = EXPR_CHAR_REF;
    expr_res.char_ref = dest;

    return expr_res;
}

static ExprResult
assign_expr(Interpreter *self, ExprResult expr1, ExprResult expr2) {
    switch (expr1.kind) {
    case EXPR_VAR:
        return assign_var(self, expr1.var, expr2);
    case EXPR_REF:
        return assign_ref(self, expr1.ref, expr2);
    case EXPR_CHAR_REF:
        return assign_char_ref(self, expr1.char_ref, expr2);
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
    case EXPR_HALT:
        return false;
    case EXPR_VAR:
    case EXPR_REF:
    case EXPR_CHAR_REF:
        return true;
    }

    /* satisfy gcc */
    return false;
}

static ExprResult exec_binary(Interpreter *self, const AstNode *node) {
    bool assigning = node->binary.op.kind == TOKEN_ASSIGN;

    ExprResult expr1 = exec_expr(self, node->binary.left, assigning);
    EXPR_RETURN_ON_HALT(expr1.node);

    ExprResult expr2 = exec_expr(self, node->binary.right, false);
    EXPR_RETURN_ON_HALT(expr2.node);

    if (expr_assignable(expr1) && assigning) {
        return assign_expr(self, expr1, expr2);
    }

    Value v1 = expr_get_value(self, &expr1);
    const Value v2 = expr_get_value(self, &expr2);

    ExprResult expr;

    switch (v1.type->id) {
    case TYPE_INT:
        expr.kind = EXPR_VALUE;
        expr.val = exec_binary_int(self, node->binary.op, &v1, &v2);

        break;
    case TYPE_STRING:
        expr.kind = EXPR_VALUE;
        expr.val = exec_binary_string(self, node->binary.op.kind, &v1, &v2);

        break;
    case TYPE_NIL:
        expr.kind = EXPR_VALUE;
        expr.val = exec_binary_nil(self, node->binary.op.kind, &v1, &v2);

        break;
    case TYPE_OPTION:
        expr.kind = EXPR_VALUE;
        expr.val = exec_binary_option(self, node->binary.op.kind, &v1, &v2);

        break;
    case TYPE_LIST:
        expr.kind = EXPR_VALUE;
        expr.val = exec_binary_list(self, node->binary.op, &v1, &v2);

        break;
    default:
        UNREACHABLE();
    }

    return expr;
}

static ExprResult
exec_suffix_int(Interpreter *self, Variable *var, const AstNode *node) {
    Value val = {self->types->builtin_int, self->env.curr_scope, {0}};
    ExprResult expr_res = {EXPR_VALUE, .node = node, {0}};

    switch (node->unary.op.kind) {
    case TOKEN_INC: {
        Int old = var->val.i;
        ++var->val.i;

        val.i = old;

        break;
    }
    case TOKEN_DEC: {
        Int old = var->val.i;
        --var->val.i;

        val.i = old;

        break;
    }
    default:
        UNREACHABLE();
    }

    expr_res.val = val;

    return expr_res;
}

static ExprResult exec_suffix(Interpreter *self, const AstNode *node) {
    ExprResult expr = exec_expr(self, node->suffix.left, false);
    EXPR_RETURN_ON_HALT(expr.node);

    assert(expr.kind == EXPR_VAR);
    Variable *var = expr.var;

    switch (var->type->id) {
    case TYPE_INT:
        return exec_suffix_int(self, var, node);
    default:
        UNREACHABLE();

        break;
    }

    return expr;
}

static ExprResult exec_string_literal(Interpreter *self, const AstNode *node) {
    ExprResult expr_res;

    expr_res.kind = EXPR_VALUE;

    StrBuf *str = scope_new_string(self->env.curr_scope);
    str_dup_n(str, node->literal.str.data, node->literal.str.len);

    expr_res.val.type = self->types->builtin_string;
    expr_res.val.scope = self->env.curr_scope;
    expr_res.val.s = str;

    return expr_res;
}

static ExprResult
exec_identifier(Interpreter *self, const AstNode *node, SourceInfo src_info) {
    ExprResult expr_res;

    Variable *var = env_find_var(&self->env, node->ident.str.data);

    if (!var) {
        expr_res.kind = EXPR_ERROR;
        error(self, src_info, "undeclared variable %s", node->ident.str.data);

        return expr_res;
    }

    expr_res.kind = EXPR_VAR;
    expr_res.var = var;

    return expr_res;
}

static ExprResult exec_string_subscript(
    Interpreter *self, const StrBuf *str, Int idx, SourceInfo src_info,
    bool assigning
) {
    ExprResult expr_res = {0};

    if ((size_t) idx >= str->len) {
        expr_res.kind = EXPR_ERROR;
        error(
            self, src_info,
            "string index out of range: %" PRId64
            " but the string length is %zu",
            idx, str->len
        );

        return expr_res;
    }

    if (assigning) {
        expr_res.kind = EXPR_CHAR_REF;
        expr_res.char_ref = &str->data[idx];
    } else {
        expr_res.kind = EXPR_VALUE;
        expr_res.val.type = self->types->builtin_int;
        expr_res.val.scope = self->env.curr_scope;
        expr_res.val.i = str->data[idx];
    }

    return expr_res;
}

static ExprResult exec_list_subscript(
    Interpreter *self, const Value *val, Int idx, SourceInfo src_info
) {
    ExprResult expr_res = {0};

    if ((size_t) idx >= val->list.values->len) {
        expr_res.kind = EXPR_ERROR;
        error(
            self, src_info,
            "list index out of range: %" PRId64 " but the list size is %zu",
            idx, val->list.values->len
        );

        return expr_res;
    }

    Value *values = val->list.values->data;

    expr_res.kind = EXPR_REF;
    expr_res.ref = &values[idx];

    return expr_res;
}

static ExprResult
exec_subscript(Interpreter *self, const AstNode *node, bool assigning) {
    ExprResult left_expr = exec_expr(self, node->subscript.left, false);
    EXPR_RETURN_ON_HALT(left_expr.node);

    ExprResult idx_expr = exec_expr(self, node->subscript.expr, false);
    EXPR_RETURN_ON_HALT(idx_expr.node);

    ExprResult expr_res = {0};
    Value left_val = expr_get_value(self, &left_expr);
    Value idx = expr_get_value(self, &idx_expr);

    switch (left_val.type->id) {
    case TYPE_STRING:
        return exec_string_subscript(
            self, left_val.s, idx.i, node->tok.src_info, assigning
        );
    case TYPE_LIST:
        return exec_list_subscript(self, &left_val, idx.i, node->tok.src_info);
    default:
        UNREACHABLE();

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
        } else if (expr.kind == EXPR_HALT) {
            break;
        }

        Value arg_val = expr_get_value(self, &expr);
        Value val = {param->type, self->env.curr_scope, {0}};

        shallowly_implicitly_clone_value(
            self, &val, &arg_val, self->env.curr_scope
        );

        Variable *arg = new_var_shallow(self, param->type, param->name, &val);
        arg->is_param = true;

        env_add_local_var(&self->env, arg);
    }

    return true;
}

static bool
pass_args_builtin(Interpreter *self, Function *fn, const AstNode **args) {
    vec_clear(&self->builtin_fn_args);

    const FnParam *params = fn->params.data;

    for (size_t i = 0; i < fn->params.len; ++i) {
        const AstNode *arg_node = args[i];
        const FnParam *param = &params[i];

        ExprResult expr = exec_expr(self, arg_node, false);

        if (expr.kind == EXPR_ERROR) {
            return false;
        } else if (expr.kind == EXPR_HALT) {
            break;
        }

        Value *arg = vec_emplace(&self->builtin_fn_args);
        Value arg_val = expr_get_value(self, &expr);
        arg->type = param->type;

        if (param->type->id == TYPE_OPTION && arg_val.type->id != TYPE_OPTION &&
            type_convertable(arg_val.type, param->type)) {
            make_opt(self, arg, param->type, &arg_val, self->env.curr_scope);
        } else {
            *arg = arg_val;
        }
    }

    return true;
}

static ExprResult
exec_builtin(Interpreter *self, Function *fn, const AstNode *node) {
    ExprResult expr_res = {0};
    Scope *saved_scope = self->env.curr_scope;
    Scope *saved_caller = self->env.caller_scope;
    Function *saved_fn = self->env.curr_fn;

    env_enter_fn(&self->env, fn);

    const AstNode **arg_nodes = node->fn_call.values.data;

    if (!pass_args_builtin(self, fn, arg_nodes)) {
        expr_res.kind = EXPR_ERROR;

        goto finish;
    }

    if (self->halt) {
        goto finish;
    }

    Value *args = self->builtin_fn_args.data;

    self->env.caller_scope = saved_scope;
    expr_res = fn->builtin(self, args, node);

finish:
    vec_clear(&self->builtin_fn_args);
    env_leave_fn(&self->env);
    self->env.curr_scope = saved_scope;
    self->env.caller_scope = saved_caller;
    self->env.curr_fn = saved_fn;

    return expr_res;
}

static ExprResult
exec_fn_body(Interpreter *self, Function *fn, const AstNode *node) {
    ExprResult expr_res = {0};

    Scope *saved_scope = self->env.curr_scope;
    Scope *saved_caller = self->env.caller_scope;
    Function *saved_fn = self->env.curr_fn;

    env_enter_fn(&self->env, fn);

    const AstNode **args = node->fn_call.values.data;

    if (!fill_fn_params_values(self, args)) {
        expr_res.kind = EXPR_ERROR;

        goto finish;
    }

    self->env.caller_scope = saved_scope;
    StmtResult body_res = exec_node(self, fn->body);

    if (self->had_error) {
        expr_res.kind = EXPR_ERROR;
    }

    if (self->halt) {
        goto finish;
    }

    switch (body_res.kind) {
    case STMT_ERROR:
        expr_res.kind = EXPR_ERROR;

        break;
    case STMT_RETURN: {
        /* The returned value is allocated in the same scope as the caller */
        expr_res.kind = EXPR_VALUE;
        expr_res.val = body_res.ret_val;

        break;
    }
    case STMT_VOID:
        if (fn->type->id != TYPE_VOID) {
            expr_res.kind = EXPR_ERROR;

            error(
                self, expr_res.node->tok.src_info,
                "function %s was expected to return %s, but it didn't",
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
    self->env.curr_fn = saved_fn;

    return expr_res;
}

static ExprResult exec_fn_call(Interpreter *self, const AstNode *node) {
    const char *name = node->fn_call.name->ident.str.data;
    Function *fn = env_find_fn(&self->env, name);
    ExprResult expr_res = {0};
    expr_res.node = node;

    if (!fn) {
        expr_res.kind = EXPR_ERROR;

        return expr_res;
    } else if (fn->is_builtin) {
        return exec_builtin(self, fn, node);
    } else if (!fn->body) {
        expr_res.kind = EXPR_ERROR;
        error(
            self, expr_res.node->tok.src_info,
            "function %s has no body to execute", name
        );

        return expr_res;
    }

    return exec_fn_body(self, fn, node);
}

static ExprResult exec_nil(const Interpreter *self) {
    ExprResult expr_res = {0};
    expr_res.kind = EXPR_VALUE;
    expr_res.val.type = self->types->nil_type;

    return expr_res;
}

static ExprResult
exec_expr(Interpreter *self, const AstNode *node, bool assigning) {
    ExprResult expr_res = {0};
    expr_res.node = node;

    switch (node->kind) {
    case AST_NODE_INTEGER:
        expr_res.kind = EXPR_VALUE;
        expr_res.val.type = self->types->builtin_int;
        expr_res.val.scope = self->env.curr_scope;
        expr_res.val.i = node->literal.i;

        return expr_res;
    case AST_NODE_STRING:
        return exec_string_literal(self, node);
    case AST_NODE_IDENT:
        return exec_identifier(self, node, node->tok.src_info);
    case AST_NODE_NIL:
        return exec_nil(self);
    case AST_NODE_UNARY:
        return exec_unary(self, node);
    case AST_NODE_BINARY:
        return exec_binary(self, node);
    case AST_NODE_SUFFIX:
        return exec_suffix(self, node);
    case AST_NODE_SUBSCRIPT:
        return exec_subscript(self, node, assigning);
    case AST_NODE_GROUPING:
        return exec_expr(self, node->grouping.expr, assigning);
    case AST_NODE_FN_CALL:
        return exec_fn_call(self, node);
    default:
        expr_res.kind = EXPR_ERROR;
        error(self, node->tok.src_info, "invalid expression");

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

        Type list = {TYPE_LIST, NULL, {0}};
        list.list_type.type = contained_type;

        return type_system_register(self->types, &list);
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
    StmtResult stmt_res = {STMT_VOID, .node = node, {0}};

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
    }

    env_leave_scope(&self->env);

    return stmt_res;
}

static ExprResult exec_list_size(Interpreter *self, const AstNode *node) {
    ExprResult expr_res = {0};
    ExprResult size_res = exec_expr(self, node, false);
    EXPR_RETURN_ON_HALT(node);

    Value val = expr_get_value(self, &size_res);
    assert(val.type == self->types->builtin_int);

    if (val.i < 0) {
        error(self, node->tok.src_info, "list size cannot be negative");
        expr_res.kind = EXPR_ERROR;

        return expr_res;
    }

    expr_res.kind = EXPR_VALUE;
    expr_res.node = node;
    new_value(
        self, &expr_res.val, self->types->builtin_int, self->env.curr_scope
    );
    expr_res.val.i = val.i;

    return expr_res;
}

static StmtResult exec_var_decl(Interpreter *self, const AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, .node = node, {0}};
    Type *type = process_type(self, node->var_decl.type);
    Value list_size = {0};

    const AstNode *size_node = node->var_decl.type->list_type.size;
    if (type->id == TYPE_LIST && size_node) {
        ExprResult expr = exec_list_size(self, size_node);
        STMT_RETURN_ON_HALT(size_node);

        list_size = expr_get_value(self, &expr);
    }

    Value val = {type, self->env.curr_scope, {0}};

    if (node->var_decl.rvalue) {
        ExprResult expr = exec_expr(self, node->var_decl.rvalue, false);
        STMT_RETURN_ON_HALT(expr.node);

        Value rval = expr_get_value(self, &expr);
        assert(type_convertable(rval.type, type));

        switch (expr.kind) {
        case EXPR_HALT:
            stmt_res.kind = STMT_HALT;

            return stmt_res;
        case EXPR_ERROR:
            stmt_res.kind = STMT_ERROR;

            return stmt_res;
        case EXPR_VALUE:
            shallowly_implicitly_clone_value(
                self, &val, &rval, self->env.curr_scope
            );

            break;
        case EXPR_VAR:
        case EXPR_REF:
        case EXPR_CHAR_REF:
            implicitly_clone_value(self, &val, &rval, self->env.curr_scope);

            break;
        }
    } else {
        new_value(self, &val, type, self->env.curr_scope);
    }

    /* Initialize list with empty values, if the size was given at declaration
     */
    if (type->id == TYPE_LIST && list_size.i > 0) {
        for (Int i = 0; i < list_size.i; ++i) {
            Value *elem = vec_emplace(val.list.values);
            new_value(self, elem, type->list_type.type, self->env.curr_scope);
        }
    }

    Variable *var = mem_alloc(sizeof(*var));
    var->name = cstr_dup(node->var_decl.name->ident.str.data);
    var->type = type;
    var->val = val;
    var->is_param = false;
    var->scope = self->env.curr_scope;

    env_add_local_var(&self->env, var);

    return stmt_res;
}

static void
create_param_list(Interpreter *self, Function *fn, const AstNode *node) {
    const AstNode **params_nodes = node->fn_decl.params.data;
    for (size_t i = 0; i < node->fn_decl.params.len; ++i) {
        const AstNode *param_node = params_nodes[i];
        Type *param_type = process_type(self, param_node->param_decl.type);

        FnParam *param = vec_emplace(&fn->params);

        param->type = param_type;
        param->name = cstr_dup(param_node->param_decl.name->ident.str.data);
    }
}

static StmtResult exec_fn_decl(Interpreter *self, AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, node, {0}};
    Type *type = process_type(self, node->fn_decl.type);
    AstNode *body = node->fn_decl.body;

    if (body) {
        /* steal ownership, so interpreter can later use the body and execute
         * it. REPL destroys AST every time and cloning is too expensive.
         */
        node->fn_decl.body = NULL;
    }

    Function *fn = mem_alloc(sizeof(*fn));

    fn->type = type;
    fn->name = cstr_dup(node->fn_decl.name->ident.str.data);
    fn->body = body;
    fn->free_body = true;
    fn->is_builtin = false;

    vec_init(&fn->params, sizeof(FnParam));
    create_param_list(self, fn, node);
    env_add_fn(&self->env, fn);

    return stmt_res;
}

static StmtResult exec_if(Interpreter *self, const AstNode *node) {
    ExprResult cond = exec_expr(self, node->kw_if.cond, false);
    STMT_RETURN_ON_HALT(cond.node);

    Value cond_val = expr_get_value(self, &cond);

    if (cond_val.i) {
        return exec_node(self, node->kw_if.body);
    } else if (node->kw_if.else_body) {
        return exec_node(self, node->kw_if.else_body);
    }

    StmtResult stmt_res = {STMT_VOID, .node = node, {0}};

    return stmt_res;
}

static StmtResult exec_while(Interpreter *self, const AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, .node = node, {0}};
    ExprResult cond = exec_expr(self, node->kw_while.cond, false);
    STMT_RETURN_ON_HALT(cond.node);

    Value cond_val = expr_get_value(self, &cond);

    while (cond_val.i) {
        StmtResult res = exec_node(self, node->kw_while.body);

        if (self->halt) {
            return stmt_res;
        }

        switch (res.kind) {
        case STMT_VOID:
        case STMT_CONTINUE:
            break;
        case STMT_HALT:
        case STMT_ERROR:
        case STMT_RETURN:
            return res;
        case STMT_BREAK:
            return stmt_res;
        }

        cond = exec_expr(self, node->kw_while.cond, false);
        STMT_RETURN_ON_HALT(cond.node);

        cond_val = expr_get_value(self, &cond);
    }

    return stmt_res;
}

static StmtResult exec_for(Interpreter *self, const AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, .node = node, {0}};

    AstNode *init = node->kw_for.init;
    const AstNode *cond = node->kw_for.cond;
    const AstNode *iter = node->kw_for.iter;
    AstNode *body = node->kw_for.body;

    env_enter_scope(&self->env);

    if (init) {
        StmtResult stmt = exec_node(self, init);
        STMT_RETURN_ON_HALT(stmt.node);
    }

    for (;;) {
        if (cond) {
            ExprResult expr = exec_expr(self, cond, false);
            STMT_RETURN_ON_HALT(expr.node);

            Value val = expr_get_value(self, &expr);

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
            case STMT_HALT:
                return res;
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
            STMT_RETURN_ON_HALT(expr.node);
        }
    }

    env_leave_scope(&self->env);

    return stmt_res;
}

static StmtResult exec_return(Interpreter *self, const AstNode *node) {
    StmtResult res = {0};
    res.kind = STMT_RETURN;

    if (node->kw_return.expr) {
        ExprResult expr = exec_expr(self, node->kw_return.expr, false);
        STMT_RETURN_ON_HALT(expr.node);

        Value val = expr_get_value(self, &expr);

        res.ret_val.type = self->env.curr_fn->type;
        res.ret_val.scope = self->env.caller_scope;

        implicitly_clone_value(
            self, &res.ret_val, &val, self->env.caller_scope
        );
    } else {
        res.ret_val.type = self->types->builtin_void;
    }

    return res;
}

static StmtResult exec_node(Interpreter *self, AstNode *node) {
    StmtResult stmt_res = {STMT_VOID, .node = node, {0}};

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
    default: {
        ExprResult expr = exec_expr(self, node, false);
        STMT_RETURN_ON_HALT(expr.node);

        break;
    }
    }

    return stmt_res;
}

void interp_init(Interpreter *self, Ast *ast, TypeSystem *types) {
    self->types = types;

    env_init(&self->env, types);
    vec_init(&self->builtin_fn_args, sizeof(Value));

    self->ast = ast;
    self->exit_code = 0;
    self->halt = false;
    self->had_error = false;
    self->log_errors = false;

    srand((unsigned) time(NULL));
}

void interp_deinit(Interpreter *self) {
    env_deinit(&self->env);
    vec_deinit(&self->builtin_fn_args);
}

int interp_walk(Interpreter *self) {
    AstNode **nodes = self->ast->nodes.data;

    for (size_t i = 0; i < self->ast->nodes.len; ++i) {
        exec_node(self, nodes[i]);

        if (self->had_error) {
            self->exit_code = -1;
        }

        if (self->halt) {
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
        exec_expr(self, ((const AstNode **) self->ast->nodes.data)[0], false);

    Value expr_val = expr_get_value(self, &expr);
    val.type = expr_val.type;
    val = expr_val;

    if (self->had_error) {
        val.type = self->types->error_type;
        self->exit_code = -1;
    }

    return val;
}

ExprResult builtin_print(Interpreter *self, Value *args, const AstNode *node) {
    ExprResult expr_res = {EXPR_VALUE, .node = node, {0}};
    expr_res.val.type = self->types->builtin_void;

    fputs(args[0].s->data, stdout);

    return expr_res;
}

ExprResult
builtin_println(Interpreter *self, Value *args, const AstNode *node) {
    ExprResult expr_res = {EXPR_VALUE, .node = node, {0}};
    expr_res.val.type = self->types->builtin_void;

    fputs(args[0].s->data, stdout);
    fputc('\n', stdout);

    return expr_res;
}

ExprResult builtin_exit(Interpreter *self, Value *args, const AstNode *node) {
    ExprResult expr_res = {EXPR_VALUE, .node = node, {0}};
    expr_res.val.type = self->types->builtin_void;

    self->exit_code = (int) args[0].i;
    self->halt = true;

    return expr_res;
}

static bool fgets_wrapper(char *buf, size_t size, FILE *file) {
    if (fgets(buf, (int) size, file)) {
        size_t newline_idx = strcspn(buf, "\n");

        if (buf[newline_idx] == '\n') {
            buf[newline_idx] = '\0';
        } else {
            clear_stdin();
        }

        return true;
    }

    return false;
}

ExprResult
builtin_input_int(Interpreter *self, Value *args, const AstNode *node) {
    UNUSED(args);

    Type *opt_int = type_system_get(self->types, "option<int>");

    /* Type has to be registered by env_init() */
    assert(opt_int != NULL);

    ExprResult expr_res = {EXPR_VALUE, .node = node, {0}};
    expr_res.val.type = opt_int;

    Value val;
    new_value(self, &val, self->types->builtin_int, self->env.caller_scope);

    static char temp_buf[INPUT_BUFSIZE];

    if (fgets_wrapper(temp_buf, sizeof(temp_buf), stdin) &&
        str_to_i64(temp_buf, &val.i)) {
        make_opt(self, &expr_res.val, opt_int, &val, self->env.caller_scope);
    } else {
        expr_res.val.opt.val = NULL;
    }

    return expr_res;
}

ExprResult
builtin_input_string(Interpreter *self, Value *args, const AstNode *node) {
    UNUSED(args);

    Type *opt_string = type_system_get(self->types, "option<string>");

    /* Type has to be registered by env_init() */
    assert(opt_string != NULL);

    ExprResult expr_res = {EXPR_VALUE, .node = node, {0}};
    expr_res.val.type = opt_string;
    expr_res.val.scope = self->env.caller_scope;
    expr_res.val.opt.val = NULL;

    Value temp_val;

    new_value(
        self, &temp_val, self->types->builtin_string, self->env.caller_scope
    );

    static char temp_buf[INPUT_BUFSIZE];

    if (fgets_wrapper(temp_buf, sizeof(temp_buf), stdin)) {
        str_set_cstr(temp_val.s, temp_buf);
        make_opt(
            self, &expr_res.val, opt_string, &temp_val, self->env.caller_scope
        );
    }

    return expr_res;
}

ExprResult builtin_random(Interpreter *self, Value *args, const AstNode *node) {
    UNUSED(args);

    ExprResult expr_res = {EXPR_VALUE, .node = node, {0}};
    expr_res.val.type = self->types->builtin_int;
    expr_res.val.scope = self->env.caller_scope;

    expr_res.val.i = rand();

    return expr_res;
}

ExprResult
builtin_random_range(Interpreter *self, Value *args, const AstNode *node) {
    ExprResult expr_res = {EXPR_VALUE, .node = node, {0}};
    expr_res.val.type = self->types->builtin_int;
    expr_res.val.scope = self->env.caller_scope;

    Value *min = &args[0];
    Value *max = &args[1];

    expr_res.val.i = rand() % ((int) max->i + 1 - (int) min->i) + (int) min->i;

    return expr_res;
}
