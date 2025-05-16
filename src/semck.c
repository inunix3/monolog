/*
 * Copyright (c) 2025-present inunix3
 *
 * This file is licensed under the MIT License (Expat)
 * (see LICENSE.md in the root of project).
 */

#include <monolog/semck.h>
#include <monolog/type.h>
#include <monolog/utils.h>

#include <stdlib.h>
#include <string.h>

void error(SemChecker *self, const DiagnosticMessage *dmsg) {
    self->had_error = true;

    vec_push(&self->dmsgs, dmsg);
}

static Type *check_expr(SemChecker *self, const AstNode *node);

static bool expr_is_mutable(SemChecker *self, const AstNode *node) {
    switch (node->kind) {
    case AST_NODE_IDENT:
        if (env_find_var(&self->env, node->ident.str.data)) {
            return true;
        }

        return false;
    case AST_NODE_GROUPING:
        return expr_is_mutable(self, node->grouping.expr);
    case AST_NODE_UNARY:
        return node->unary.op.kind == TOKEN_MUL
                   ? expr_is_mutable(self, node->unary.right)
                   : false;
    case AST_NODE_SUBSCRIPT:
        return true;
    default:
        return false;
    }
}

static Type *check_binary(SemChecker *self, const AstNode *node) {
    TokenKind op = node->binary.op.kind;
    Type *t1 = check_expr(self, node->binary.left);
    Type *t2 = check_expr(self, node->binary.right);

    if (t1->id == TYPE_ERROR || t2->id == TYPE_ERROR) {
        return self->types->error_type;
    }

    switch (op) {
    case TOKEN_PLUS:
        if (type_equal(t1, self->types->builtin_int) &&
            type_equal(t2, self->types->builtin_int)) {
            return self->types->builtin_int;
        } else if (type_equal(t1, self->types->builtin_string) &&
                   type_equal(t2, self->types->builtin_string)) {
            return self->types->builtin_string;
        } else {
            DiagnosticMessage dmsg;
            dmsg.kind = DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION;
            dmsg.binary_op_comb.op = op;
            dmsg.binary_op_comb.t1 = t1;
            dmsg.binary_op_comb.t2 = t2;
            error(self, &dmsg);

            return self->types->error_type;
        }
    case TOKEN_EQUAL:
    case TOKEN_NOT_EQUAL:
        if (t1->id == TYPE_OPTION && t2->id == TYPE_OPTION) {
            /* opt1 == opt2 */
            return self->types->builtin_int;
        } else if ((t1->id == TYPE_OPTION && t2->id == TYPE_NIL) ||
                   (t1->id == TYPE_NIL && t2->id == TYPE_OPTION)) {
            /* opt == nil, nil == opt */
            return self->types->builtin_int;
        } else if (t1->id == TYPE_NIL && t2->id == TYPE_NIL) {
            /* nil == nil */
            return self->types->builtin_int;
        } else if (t1->id == TYPE_STRING && t2->id == TYPE_STRING) {
            /* string == string */
            return self->types->builtin_int;
        }

        /* Fallthrough */
    case TOKEN_MINUS:
    case TOKEN_MUL:
    case TOKEN_DIV:
    case TOKEN_MOD:
    case TOKEN_LESS:
    case TOKEN_GREATER:
    case TOKEN_LESS_EQUAL:
    case TOKEN_GREATER_EQUAL:
    case TOKEN_AND:
    case TOKEN_OR:
        if (type_equal(t1, self->types->builtin_int) &&
            type_equal(t2, self->types->builtin_int)) {
            return self->types->builtin_int;
        } else {
            DiagnosticMessage dmsg;
            dmsg.kind = DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION;
            dmsg.binary_op_comb.op = op;
            dmsg.binary_op_comb.t1 = t1;
            dmsg.binary_op_comb.t2 = t2;

            error(self, &dmsg);

            return self->types->error_type;
        }
    case TOKEN_ASSIGN:
        if (!expr_is_mutable(self, node->binary.left)) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_EXPR_NOT_MUTABLE, node->binary.op.src_info, {0}
            };

            error(self, &dmsg);

            return self->types->error_type;
        }

        if (!type_convertable(t2, t1)) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_MISMATCHED_TYPES, node->binary.op.src_info, {0}
            };
            dmsg.type_mismatch.expected = t1;
            dmsg.type_mismatch.found = t2;

            error(self, &dmsg);

            return self->types->error_type;
        }

        return t1;
    case TOKEN_ADD_ASSIGN:
        if (!expr_is_mutable(self, node->binary.left)) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_EXPR_NOT_MUTABLE, node->binary.op.src_info, {0}
            };

            error(self, &dmsg);

            return self->types->error_type;
        }

        if (t1->id != TYPE_LIST) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_EXPECTED_LIST, node->binary.left->tok.src_info, {0}
            };
            dmsg.type_mismatch.found = t2;

            error(self, &dmsg);

            return self->types->error_type;
        }

        if (!type_convertable(t2, t1->list_type.type)) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_MISMATCHED_TYPES,
                node->binary.right->tok.src_info,
                {0}
            };
            dmsg.type_mismatch.expected = t1;
            dmsg.type_mismatch.found = self->types->builtin_int;

            error(self, &dmsg);

            return self->types->error_type;
        }

        return self->types->builtin_void;
    case TOKEN_SUB_ASSIGN:
        if (!expr_is_mutable(self, node->binary.left)) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_EXPR_NOT_MUTABLE,
                node->binary.left->tok.src_info,
                {0}
            };

            error(self, &dmsg);

            return self->types->error_type;
        }

        if (t1->id != TYPE_LIST) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_EXPECTED_LIST, node->binary.left->tok.src_info, {0}
            };
            dmsg.type_mismatch.found = t2;

            error(self, &dmsg);

            return self->types->error_type;
        }

        if (!type_convertable(t2, self->types->builtin_int)) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_MISMATCHED_TYPES,
                node->binary.left->tok.src_info,
                {0}
            };
            dmsg.type_mismatch.expected = t1;
            dmsg.type_mismatch.found = self->types->builtin_int;

            error(self, &dmsg);

            return self->types->error_type;
        }

        return self->types->builtin_void;
    default: {
        DiagnosticMessage dmsg = {
            DIAGNOSTIC_INTERNAL_ERROR, node->binary.op.src_info, {0}
        };
        error(self, &dmsg);

        return self->types->error_type;
    }
    }
}

static Type *check_unary(SemChecker *self, const AstNode *node) {
    TokenKind op = node->unary.op.kind;
    Type *type = check_expr(self, node->unary.right);

    if (type->id == TYPE_ERROR) {
        return self->types->error_type;
    }

    switch (op) {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_EXCL:
        if (type_equal(type, self->types->builtin_int)) {
            return self->types->builtin_int;
        } else {
            goto bad_unary_operand;
        }
    case TOKEN_INC:
    case TOKEN_DEC:
        if (!expr_is_mutable(self, node->unary.right)) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_EXPR_NOT_MUTABLE, node->binary.op.src_info, {0}
            };

            error(self, &dmsg);

            return self->types->error_type;
        }

        if (type->id != TYPE_INT) {
            goto bad_unary_operand;
        }

        return type;
    case TOKEN_HASHTAG:
        if (type->id != TYPE_LIST && type->id != TYPE_STRING) {
            goto bad_unary_operand;
        }

        return self->types->builtin_int;
    case TOKEN_DOLAR:
        if (type->id != TYPE_INT) {
            goto bad_unary_operand;
        }

        return self->types->builtin_string;
    case TOKEN_MUL:
        if (type->id != TYPE_OPTION) {
            goto bad_unary_operand;
        }

        return type->opt_type.type;
    default: {
        DiagnosticMessage dmsg = {
            DIAGNOSTIC_INTERNAL_ERROR, node->unary.op.src_info, {0}
        };
        error(self, &dmsg);

        return self->types->error_type;
    }
    }

bad_unary_operand: {
    DiagnosticMessage dmsg = {
        .kind = DIAGNOSTIC_BAD_UNARY_OPERAND,
        .src_info = node->unary.right->tok.src_info,
        {
            .unary_op_comb =
                {
                    .op = op,
                    .type = type,
                },
        },
    };
    error(self, &dmsg);

    return self->types->error_type;
}
}

static Type *check_suffix(SemChecker *self, const AstNode *node) {
    TokenKind op = node->suffix.op.kind;
    Type *type = check_expr(self, node->suffix.left);

    if (type->id == TYPE_ERROR) {
        return self->types->error_type;
    }

    switch (op) {
    case TOKEN_INC:
    case TOKEN_DEC:
        if (!expr_is_mutable(self, node->suffix.left)) {
            DiagnosticMessage dmsg = {
                .kind = DIAGNOSTIC_EXPR_NOT_MUTABLE,
                .src_info = node->suffix.left->tok.src_info,
                {0}
            };

            error(self, &dmsg);

            return self->types->error_type;
        }

        if (type->id != TYPE_INT) {
            DiagnosticMessage dmsg;
            dmsg.kind = DIAGNOSTIC_BAD_SUFFIX_OPERAND_COMBINATION;
            dmsg.suffix_op_comb.op = op;
            dmsg.suffix_op_comb.type = type;
            error(self, &dmsg);

            return self->types->error_type;
        }

        return type;
    default:
        return self->types->error_type;
    }
}

static bool check_var(SemChecker *self, const Variable *var, char *name) {
    if (!var) {
        DiagnosticMessage dmsg = {
            DIAGNOSTIC_UNDECLARED_VARIABLE, .src_info = {0}
        };
        dmsg.undef_sym.name = name;

        error(self, &dmsg);

        return false;
    }

    return true;
}

static Type *check_ident(SemChecker *self, const AstNode *node) {
    Variable *var = env_find_var(&self->env, node->ident.str.data);

    if (!check_var(self, var, node->ident.str.data)) {
        return self->types->error_type;
    }

    return var->type;
}

static Type *check_fn_call(SemChecker *self, const AstNode *node) {
    char *name = node->fn_call.name->ident.str.data;
    Function *fn = hashmap_get(&self->env.funcs, name);

    if (!fn) {
        DiagnosticMessage dmsg;
        dmsg.kind = DIAGNOSTIC_UNDECLARED_FUNCTION;
        dmsg.undef_sym.name = name;

        error(self, &dmsg);

        return self->types->error_type;
    }

    Type *type = fn->type;
    const Vector *values_vec = &node->fn_call.values;

    if (values_vec->len < fn->params.len) {
        DiagnosticMessage dmsg;
        dmsg.kind = DIAGNOSTIC_TOO_FEW_ARGS;
        dmsg.bad_arg_count.expected = fn->params.len;
        dmsg.bad_arg_count.supplied = values_vec->len;

        error(self, &dmsg);

        return type;
    } else if (values_vec->len > fn->params.len) {
        DiagnosticMessage dmsg;
        dmsg.kind = DIAGNOSTIC_TOO_MANY_ARGS;
        dmsg.bad_arg_count.expected = fn->params.len;
        dmsg.bad_arg_count.supplied = values_vec->len;

        error(self, &dmsg);

        return type;
    }

    const FnParam *params = fn->params.data;
    const AstNode **values = values_vec->data;

    for (size_t i = 0; i < values_vec->len; ++i) {
        const AstNode *value = values[i];
        const FnParam *param = &params[i];

        Type *value_type = check_expr(self, value);

        if (value_type->id == TYPE_ERROR) {
            continue;
        }

        if (!type_convertable(value_type, param->type)) {
            DiagnosticMessage dmsg;
            dmsg.kind = DIAGNOSTIC_BAD_ARG_TYPE;
            dmsg.bad_arg_type.expected = param->type;
            dmsg.bad_arg_type.found = value_type;

            error(self, &dmsg);
        }
    }

    return type;
}

static Type *check_subscript(SemChecker *self, const AstNode *node) {
    const AstNode *expr = node->subscript.expr;
    const AstNode *left = node->subscript.left;

    Type *left_type = check_expr(self, left);

    if (left_type->id == TYPE_ERROR) {
        return self->types->error_type;
    } else if (left_type->id != TYPE_LIST && left_type->id != TYPE_STRING) {
        DiagnosticMessage dmsg = {
            DIAGNOSTIC_EXPR_NOT_INDEXABLE, left->tok.src_info, {0}
        };

        error(self, &dmsg);

        return self->types->error_type;
    }

    Type *expr_type = check_expr(self, expr);

    if (expr_type->id != TYPE_ERROR && expr_type->id != TYPE_INT) {
        DiagnosticMessage dmsg;
        dmsg.kind = DIAGNOSTIC_BAD_INDEX_TYPE;
        dmsg.bad_index_type.found = expr_type;

        error(self, &dmsg);
    }

    if (left_type->id == TYPE_LIST) {
        return left_type->list_type.type;
    } else {
        return self->types->builtin_int;
    }
}

static Type *check_expr(SemChecker *self, const AstNode *node) {
    switch (node->kind) {
    case AST_NODE_INTEGER:
        return self->types->builtin_int;
    case AST_NODE_STRING:
        return self->types->builtin_string;
    case AST_NODE_IDENT:
        return check_ident(self, node);
    case AST_NODE_NIL:
        return self->types->nil_type;
    case AST_NODE_BINARY:
        return check_binary(self, node);
    case AST_NODE_UNARY:
        return check_unary(self, node);
    case AST_NODE_SUFFIX:
        return check_suffix(self, node);
    case AST_NODE_GROUPING:
        return check_expr(self, node->grouping.expr);
    case AST_NODE_FN_CALL:
        return check_fn_call(self, node);
    case AST_NODE_SUBSCRIPT:
        return check_subscript(self, node);
    default:
        return self->types->error_type;
    }
}

static Type *parse_type(SemChecker *self, const AstNode *node) {
    switch (node->kind) {
    case AST_NODE_INT_TYPE:
        return self->types->builtin_int;
    case AST_NODE_STRING_TYPE:
        return self->types->builtin_string;
    case AST_NODE_VOID_TYPE:
        return self->types->builtin_void;
    case AST_NODE_LIST_TYPE: {
        Type *contained_type = parse_type(self, node->list_type.type);
        Type list = {TYPE_LIST, NULL, {0}};
        list.list_type.type = contained_type;

        return type_system_register(self->types, &list);
    }
    case AST_NODE_OPTION_TYPE: {
        Type *contained_type = parse_type(self, node->opt_type.type);
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

static void check_var_decl(SemChecker *self, const AstNode *node) {
    Type *type = parse_type(self, node->var_decl.type);
    AstNode *list_size_node = node->var_decl.type->list_type.size;

    if (type->id == TYPE_LIST && list_size_node) {
        Type *size_type = check_expr(self, list_size_node);

        if (size_type->id != TYPE_INT) {
            DiagnosticMessage dmsg = {
                .kind = DIAGNOSTIC_MISMATCHED_TYPES,
                .src_info = list_size_node->tok.src_info,
                {
                    .type_mismatch.expected = self->types->builtin_int,
                    .type_mismatch.found = size_type,
                },
            };

            error(self, &dmsg);
        }
    }

    const char *name = node->var_decl.name->ident.str.data;
    const AstNode *rvalue = node->var_decl.rvalue;

    if (rvalue) {
        Type *value_type = check_expr(self, rvalue);

        if (value_type->id != TYPE_ERROR &&
            !type_convertable(value_type, type)) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_MISMATCHED_TYPES, rvalue->tok.src_info, {0}
            };
            dmsg.type_mismatch.expected = type;
            dmsg.type_mismatch.found = value_type;

            error(self, &dmsg);
        }
    }

    Variable *var = mem_alloc(sizeof(*var));

    var->type = type;
    var->name = cstr_dup(name);
    var->is_param = false;

    env_add_local_var(&self->env, var);
}

static void check_node(SemChecker *self, const AstNode *node);

static void check_fn_decl(SemChecker *self, const AstNode *node) {
    if (self->env.curr_fn) {
        DiagnosticMessage dmsg;
        dmsg.kind = DIAGNOSTIC_FN_BAD_PLACE;

        error(self, &dmsg);

        return;
    }

    Type *type = parse_type(self, node->fn_decl.type);
    char *name = node->fn_decl.name->ident.str.data;
    AstNode *body = node->fn_decl.body;

    if (hashmap_get(&self->env.funcs, name)) {
        DiagnosticMessage dmsg;
        dmsg.kind = DIAGNOSTIC_FN_REDEFINITION;
        dmsg.fn_redef.name = name;

        error(self, &dmsg);

        return;
    }

    Function *fn = mem_alloc(sizeof(*fn));
    fn->type = type;
    fn->name = cstr_dup_n(name, node->fn_decl.name->ident.str.len);
    fn->body = body;
    fn->free_body = false;

    env_enter_scope(&self->env);

    vec_init(&fn->params, sizeof(FnParam));

    const Vector *params_vec = &node->fn_decl.params;
    const AstNode **params = params_vec->data;

    for (size_t i = 0; i < params_vec->len; ++i) {
        const AstNode *param_node = params[i];

        type = parse_type(self, param_node->param_decl.type);
        name = param_node->param_decl.name->ident.str.data;
        bool bad_param = false;

        for (size_t j = 0; j < fn->params.len; ++j) {
            const FnParam *param = fn->params.data;

            if (strcmp(name, param[j].name) == 0) {
                DiagnosticMessage dmsg;
                dmsg.kind = DIAGNOSTIC_PARAM_REDECLARATION;
                dmsg.param_redecl.name = name;

                error(self, &dmsg);

                bad_param = true;
            }
        }

        if (!bad_param) {
            FnParam *param = vec_emplace(&fn->params);
            param->type = type;
            param->name = cstr_dup(name);

            Variable *var = mem_alloc(sizeof(*var));
            var->type = type;
            var->name = cstr_dup(name);
            var->is_param = true;

            env_add_local_var(&self->env, var);
        } else {
            bad_param = false;
        }
    }

    env_add_fn(&self->env, fn);

    if (body) {
        self->env.curr_fn = fn;
        check_node(self, body);
        self->env.curr_fn = NULL;
    }

    env_leave_scope(&self->env);
}

static void check_block(SemChecker *self, const AstNode *node) {
    env_enter_scope(&self->env);
    const AstNode **nodes = node->block.nodes.data;

    for (size_t i = 0; i < node->block.nodes.len; ++i) {
        check_node(self, nodes[i]);
    }

    env_leave_scope(&self->env);
}

static void check_if(SemChecker *self, const AstNode *node) {
    const AstNode *cond = node->kw_if.cond;
    const AstNode *body = node->kw_if.body;
    const AstNode *else_body = node->kw_if.else_body;

    Type *cond_type = check_expr(self, cond);

    if (cond_type->id != TYPE_ERROR && cond_type->id != TYPE_INT) {
        DiagnosticMessage dmsg = {
            DIAGNOSTIC_MISMATCHED_TYPES, cond->tok.src_info, {0}
        };
        dmsg.type_mismatch.expected = self->types->builtin_int;
        dmsg.type_mismatch.found = cond_type;

        error(self, &dmsg);
    }

    if (body) {
        check_node(self, body);
    }

    if (else_body) {
        check_node(self, else_body);
    }
}

static void check_while(SemChecker *self, const AstNode *node) {
    const AstNode *cond = node->kw_while.cond;
    const AstNode *body = node->kw_while.body;

    Type *cond_type = check_expr(self, cond);

    if (cond_type->id != TYPE_ERROR && cond_type->id != TYPE_INT) {
        DiagnosticMessage dmsg = {
            .kind = DIAGNOSTIC_MISMATCHED_TYPES,
            .src_info = cond->tok.src_info,
            .type_mismatch =
                {
                    .expected = self->types->builtin_int,
                    .found = cond_type,
                },
        };

        error(self, &dmsg);
    }

    if (body) {
        ++self->loop_depth;
        check_node(self, body);
        --self->loop_depth;
    }
}

static void check_for(SemChecker *self, const AstNode *node) {
    const AstNode *init = node->kw_for.init;
    const AstNode *cond = node->kw_for.cond;
    const AstNode *iter = node->kw_for.iter;
    const AstNode *body = node->kw_for.body;

    if (init) {
        check_node(self, init);
    }

    if (cond) {
        Type *cond_type = check_expr(self, cond);

        if (cond_type->id != TYPE_ERROR && cond_type->id != TYPE_INT) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_MISMATCHED_TYPES, cond->tok.src_info, {0}
            };
            dmsg.type_mismatch.expected = self->types->builtin_int;
            dmsg.type_mismatch.found = cond_type;

            error(self, &dmsg);
        }
    }

    if (iter) {
        check_expr(self, iter);
    }

    if (body) {
        ++self->loop_depth;
        check_node(self, body);
        --self->loop_depth;
    }
}

static void check_return(SemChecker *self, const AstNode *node) {
    const AstNode *expr = node->kw_return.expr;

    if (!self->env.curr_fn) {
        DiagnosticMessage dmsg = {
            DIAGNOSTIC_RETURN_OUTSIDE_FUNCTION, node->tok.src_info, {0}
        };

        error(self, &dmsg);

        return;
    }

    Type *ret_type = expr ? check_expr(self, expr) : self->types->builtin_void;
    Type *expected = self->env.curr_fn->type;

    if (ret_type->id == TYPE_ERROR) {
        return;
    }

    if (!type_convertable(ret_type, self->types->builtin_void) &&
        expected == self->types->builtin_void) {
        DiagnosticMessage dmsg = {
            DIAGNOSTIC_VOID_RETURN, expr->tok.src_info, {0}
        };

        error(self, &dmsg);
    } else if (!type_convertable(ret_type, expected)) {
        DiagnosticMessage dmsg = {
            DIAGNOSTIC_MISMATCHED_TYPES, expr->tok.src_info, {0}
        };
        dmsg.type_mismatch.expected = self->env.curr_fn->type;
        dmsg.type_mismatch.found = ret_type;

        error(self, &dmsg);
    }
}

static void check_node(SemChecker *self, const AstNode *node) {
    switch (node->kind) {
    case AST_NODE_VAR_DECL:
        check_var_decl(self, node);

        break;
    case AST_NODE_FN_DECL:
        check_fn_decl(self, node);

        break;
    case AST_NODE_BLOCK:
        check_block(self, node);

        break;
    case AST_NODE_IF:
        check_if(self, node);

        break;
    case AST_NODE_WHILE:
        check_while(self, node);

        break;
    case AST_NODE_FOR:
        check_for(self, node);

        break;
    case AST_NODE_BREAK:
        if (self->loop_depth <= 0) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_BREAK_OUTSIDE_LOOP, node->tok.src_info, {0}
            };

            error(self, &dmsg);
        }

        break;
    case AST_NODE_CONTINUE:
        if (self->loop_depth <= 0) {
            DiagnosticMessage dmsg = {
                DIAGNOSTIC_CONTINUE_OUTSIDE_LOOP, node->tok.src_info, {0}
            };

            error(self, &dmsg);
        }

        break;
    case AST_NODE_RETURN:
        check_return(self, node);

        break;
    default:
        check_expr(self, node);

        break;
    }
}

void semck_init(SemChecker *self, TypeSystem *types) {
    self->types = types;
    env_init(&self->env, types);

    vec_init(&self->dmsgs, sizeof(DiagnosticMessage));
    self->had_error = false;
    self->loop_depth = 0;
}

void semck_deinit(SemChecker *self) {
    env_deinit(&self->env);

    vec_deinit(&self->dmsgs);
    self->had_error = false;
}

static Variable *var_clone(const Variable *var) {
    Variable *var_copy = mem_alloc(sizeof(*var));
    var_copy->type = var->type;
    var_copy->name = cstr_dup(var->name);
    var_copy->is_param = var->is_param;

    return var_copy;
}

static void clone_fn_params(Vector *self, const Vector *params_vec) {
    const FnParam *params = params_vec->data;

    for (size_t i = 0; i < params_vec->len; ++i) {
        FnParam *param = vec_emplace(self);

        param->type = params[i].type;

        /* only builtin functions don't have names for params */
        if (params[i].name) {
            param->name = cstr_dup(params[i].name);
        }
    }
}

bool semck_check(
    SemChecker *self, const Ast *ast, HashMap *vars, HashMap *funcs
) {
    if (vars) {
        for (HashMapIter it = hashmap_iter(vars); it.bucket != NULL;
             hashmap_iter_next(&it)) {
            Variable *var = it.bucket->value;
            Variable *var_copy = var_clone(var);

            env_add_global_var(&self->env, var_copy);
        }
    }

    if (funcs) {
        for (HashMapIter it = hashmap_iter(funcs); it.bucket != NULL;
             hashmap_iter_next(&it)) {
            Function *fn = it.bucket->value;

            Function *fn_copy = mem_alloc(sizeof(*fn_copy));
            vec_init(&fn_copy->params, sizeof(FnParam));

            fn_copy->type = fn->type;
            fn_copy->name = cstr_dup(fn->name);
            clone_fn_params(&fn_copy->params, &fn->params);
            fn_copy->body = fn->body;
            fn_copy->free_body = false;

            env_add_fn(&self->env, fn_copy);
        }
    }

    const AstNode **nodes = ast->nodes.data;

    for (size_t i = 0; i < ast->nodes.len; ++i) {
        check_node(self, nodes[i]);
    }

    return !self->had_error;
}

void semck_reset(SemChecker *self) {
    self->had_error = false;
    self->loop_depth = 0;
    vec_clear(&self->dmsgs);
    env_reset(&self->env);
}
