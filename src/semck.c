#include <monolog/semck.h>
#include <monolog/type.h>
#include <monolog/utils.h>

#include <stdlib.h>
#include <string.h>

void error(SemChecker *self, const DiagnosticMessage *dmsg) {
    self->error_state = true;

    vec_push(&self->dmsgs, dmsg);
}

static Type *check_expr(SemChecker *self, const AstNode *node, bool assigning);

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
        return node->unary.op == TOKEN_OP_MUL
                   ? expr_is_mutable(self, node->unary.right)
                   : false;
    case AST_NODE_ARRAY_SUBSCRIPT:
        return true;
    default:
        return false;
    }
}

Type *assign_expr(SemChecker *self, const AstNode *node, Type *type) {
    switch (node->kind) {
    case AST_NODE_IDENT: {
        Variable *var = env_find_var(&self->env, node->ident.str.data);

        var->defined = true;

        return var->type;
    }
    case AST_NODE_GROUPING:
        return assign_expr(self, node->grouping.expr, type);
    default:
        return self->types->error_type;
    }
}

static Type *check_binary(SemChecker *self, const AstNode *node) {
    TokenKind op = node->binary.op;
    Type *t1 = check_expr(self, node->binary.left, op == TOKEN_OP_ASSIGN);
    Type *t2 = check_expr(self, node->binary.right, false);

    if (t1->id == TYPE_ERROR || t2->id == TYPE_ERROR) {
        return self->types->error_type;
    }

    switch (op) {
    case TOKEN_OP_PLUS:
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
    case TOKEN_OP_EQUAL:
    case TOKEN_OP_NOT_EQUAL:
        if ((t1->id == TYPE_OPTION && t2->id == TYPE_NIL) ||
            (t1->id == TYPE_NIL && t2->id == TYPE_OPTION)) {
            return self->types->builtin_int;
        } else if (t1->id == TYPE_STRING && t2->id == TYPE_STRING) {
            return self->types->builtin_int;
        }

        /* Fallthrough */
    case TOKEN_OP_MINUS:
    case TOKEN_OP_MUL:
    case TOKEN_OP_DIV:
    case TOKEN_OP_MOD:
    case TOKEN_OP_LESS:
    case TOKEN_OP_GREATER:
    case TOKEN_OP_LESS_EQUAL:
    case TOKEN_OP_GREATER_EQUAL:
    case TOKEN_OP_AND:
    case TOKEN_OP_OR:
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
    case TOKEN_OP_ASSIGN:
        if (!expr_is_mutable(self, node->binary.left)) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_EXPR_NOT_MUTABLE, {0}};

            error(self, &dmsg);

            return self->types->error_type;
        }

        if (!type_can_implicitly_convert(t2, t1)) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_MISMATCHED_TYPES, {0}};
            dmsg.type_mismatch.expected = t1;
            dmsg.type_mismatch.found = t2;

            error(self, &dmsg);

            return self->types->error_type;
        }

        return assign_expr(self, node->binary.left, t2);
    default: {
        DiagnosticMessage dmsg = {DIAGNOSTIC_INTERNAL_ERROR, {0}};
        error(self, &dmsg);

        return self->types->error_type;
    }
    }
}

static Type *check_unary(SemChecker *self, const AstNode *node) {
    TokenKind op = node->unary.op;
    Type *type = check_expr(self, node->unary.right, false);

    if (type->id == TYPE_ERROR) {
        return self->types->error_type;
    }

    switch (op) {
    case TOKEN_OP_PLUS:
    case TOKEN_OP_MINUS:
    case TOKEN_OP_EXCL:
        if (type_equal(type, self->types->builtin_int)) {
            return self->types->builtin_int;
        } else {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION, {0}};
            dmsg.unary_op_comb.op = op;
            dmsg.unary_op_comb.type = type;
            error(self, &dmsg);

            return self->types->error_type;
        }
    case TOKEN_OP_INC:
    case TOKEN_OP_DEC:
        if (!expr_is_mutable(self, node->unary.right)) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_EXPR_NOT_MUTABLE, {0}};

            error(self, &dmsg);

            return self->types->error_type;
        }

        if (type->id != TYPE_INT) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION, {0}};
            dmsg.unary_op_comb.op = op;
            dmsg.unary_op_comb.type = type;
            error(self, &dmsg);

            return self->types->error_type;
        }

        return type;
    case TOKEN_OP_HASHTAG:
        if (type->id != TYPE_ARRAY && type->id != TYPE_STRING) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION, {0}};
            dmsg.unary_op_comb.op = op;
            dmsg.unary_op_comb.type = type;
            error(self, &dmsg);

            return self->types->error_type;
        }

        return self->types->builtin_int;
    case TOKEN_OP_DOLAR:
        if (type->id != TYPE_INT) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION, {0}};
            dmsg.unary_op_comb.op = op;
            dmsg.unary_op_comb.type = type;
            error(self, &dmsg);

            return self->types->error_type;
        }

        return self->types->builtin_string;
    case TOKEN_OP_MUL:
        if (type->id != TYPE_OPTION) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION, {0}};
            dmsg.unary_op_comb.op = op;
            dmsg.unary_op_comb.type = type;
            error(self, &dmsg);

            return self->types->error_type;
        }

        return type->opt_type.type;
    default: {
        DiagnosticMessage dmsg = {DIAGNOSTIC_INTERNAL_ERROR, {0}};
        error(self, &dmsg);

        return self->types->error_type;
    }
    }
}

static Type *check_suffix(SemChecker *self, const AstNode *node) {
    TokenKind op = node->suffix.op;
    Type *type = check_expr(self, node->suffix.left, false);

    if (type->id == TYPE_ERROR) {
        return self->types->error_type;
    }

    switch (op) {
    case TOKEN_OP_INC:
    case TOKEN_OP_DEC:
        if (!expr_is_mutable(self, node->suffix.left)) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_EXPR_NOT_MUTABLE, {0}};

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

static bool
check_var(SemChecker *self, const Variable *var, char *name, bool assigning) {
    if (!var) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_UNDECLARED_VARIABLE, {0}};
        dmsg.undef_sym.name = name;

        error(self, &dmsg);

        return false;
    } else if (!assigning && (var->type->id != TYPE_ARRAY && !var->defined) &&
               !var->is_param) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_UNDEFINED_VARIABLE, {0}};
        dmsg.undef_sym.name = name;

        error(self, &dmsg);

        return false;
    }

    return true;
}

static Type *
check_ident(SemChecker *self, const AstNode *node, bool assigning) {
    Variable *var = env_find_var(&self->env, node->ident.str.data);

    if (!check_var(self, var, node->ident.str.data, assigning)) {
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

    const Variable *params = fn->params.data;
    const AstNode **values = values_vec->data;

    for (size_t i = 0; i < values_vec->len; ++i) {
        const AstNode *value = values[i];
        const Variable *param = &params[i];

        Type *value_type = check_expr(self, value, false);

        if (value_type->id == TYPE_ERROR) {
            continue;
        }

        if (!type_can_implicitly_convert(value_type, param->type)) {
            DiagnosticMessage dmsg;
            dmsg.kind = DIAGNOSTIC_BAD_ARG_TYPE;
            dmsg.bad_arg_type.expected = param->type;
            dmsg.bad_arg_type.found = value_type;

            error(self, &dmsg);
        }
    }

    return type;
}

static Type *check_array_sub(SemChecker *self, const AstNode *node) {
    const AstNode *expr = node->array_sub.expr;
    const AstNode *left = node->array_sub.left;

    Type *left_type = check_expr(self, left, false);

    if (left_type->id == TYPE_ERROR) {
        return self->types->error_type;
    } else if (left_type->id != TYPE_ARRAY && left_type->id != TYPE_STRING) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_EXPR_NOT_INDEXABLE, {0}};

        error(self, &dmsg);

        return self->types->error_type;
    }

    Type *expr_type = check_expr(self, expr, false);

    if (expr_type->id != TYPE_ERROR && expr_type->id != TYPE_INT) {
        DiagnosticMessage dmsg;
        dmsg.kind = DIAGNOSTIC_BAD_INDEX_TYPE;
        dmsg.bad_index_type.found = expr_type;

        error(self, &dmsg);
    }

    if (left_type->id == TYPE_ARRAY) {
        return left_type->array_type.type;
    } else {
        return self->types->builtin_int;
    }
}

static Type *check_expr(SemChecker *self, const AstNode *node, bool assigning) {
    switch (node->kind) {
    case AST_NODE_INTEGER:
        return self->types->builtin_int;
    case AST_NODE_STRING:
        return self->types->builtin_string;
    case AST_NODE_IDENT:
        return check_ident(self, node, assigning);
    case AST_NODE_NIL:
        return self->types->nil_type;
    case AST_NODE_BINARY:
        return check_binary(self, node);
    case AST_NODE_UNARY:
        return check_unary(self, node);
    case AST_NODE_SUFFIX:
        return check_suffix(self, node);
    case AST_NODE_GROUPING:
        return check_expr(self, node->grouping.expr, assigning);
    case AST_NODE_FN_CALL:
        return check_fn_call(self, node);
    case AST_NODE_ARRAY_SUBSCRIPT:
        return check_array_sub(self, node);
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
    case AST_NODE_ARRAY_TYPE: {
        Type *contained_type = parse_type(self, node->array_type.type);
        Type array = {TYPE_ARRAY, NULL, {0}};
        array.array_type.type = contained_type;

        return type_system_register(self->types, &array);
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
    const char *name = node->var_decl.name->ident.str.data;
    const AstNode *rvalue = node->var_decl.rvalue;

    if (rvalue) {
        Type *value_type = check_expr(self, node->var_decl.rvalue, false);

        if (value_type->id != TYPE_ERROR &&
            !type_can_implicitly_convert(value_type, type)) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_MISMATCHED_TYPES, {0}};
            dmsg.type_mismatch.expected = type;
            dmsg.type_mismatch.found = value_type;

            error(self, &dmsg);
        }
    }

    Variable *var = malloc(sizeof(*var));
    var->type = type;
    var->name = cstr_dup(name);
    var->defined = rvalue != NULL;
    var->is_param = false;

    hashmap_add(&self->env.curr_scope->vars, var->name, var);
}

static void check_node(SemChecker *self, const AstNode *node);

static void check_fn_decl(SemChecker *self, const AstNode *node) {
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

    Function *fn = malloc(sizeof(*fn));
    fn->type = type;
    fn->name = name;
    fn->body = body;

    vec_init(&fn->params, sizeof(Variable));

    const Vector *params_vec = &node->fn_decl.params;
    const AstNode **params = params_vec->data;

    for (size_t i = 0; i < params_vec->len; ++i) {
        const AstNode *param_node = params[i];

        type = parse_type(self, param_node->param_decl.type);
        name = param_node->param_decl.name->ident.str.data;

        for (size_t j = 0; j < fn->params.len; ++j) {
            const Variable *fn_params = fn->params.data;

            if (strcmp(name, fn_params[j].name) == 0) {
                DiagnosticMessage dmsg;
                dmsg.kind = DIAGNOSTIC_PARAM_REDECLARATION;
                dmsg.param_redecl.name = name;

                error(self, &dmsg);
            }
        }

        Variable param;
        param.type = type;
        param.name = name;
        param.defined = false;
        param.is_param = true;

        vec_push(&fn->params, &param);
    }

    hashmap_add(&self->env.funcs, fn->name, fn);

    self->env.curr_fn = fn;

    if (body) {
        check_node(self, body);
    }

    self->env.curr_fn = NULL;
}

static void check_block(SemChecker *self, const AstNode *node) {
    Scope new_scope;
    scope_init(&new_scope);
    vec_push(&self->env.scopes, &new_scope);

    self->env.curr_scope = &VEC_LAST(&self->env.scopes, Scope);

    const AstNode **nodes = node->block.nodes.data;

    for (size_t i = 0; i < node->block.nodes.len; ++i) {
        check_node(self, nodes[i]);
    }

    scope_deinit(self->env.curr_scope);
    vec_pop(&self->env.scopes);
    self->env.curr_scope = &VEC_LAST(&self->env.scopes, Scope);
}

static void check_if(SemChecker *self, const AstNode *node) {
    const AstNode *cond = node->kw_if.cond;
    const AstNode *body = node->kw_if.body;
    const AstNode *else_body = node->kw_if.else_body;

    Type *cond_type = check_expr(self, cond, false);

    if (cond_type->id != TYPE_ERROR && cond_type->id != TYPE_INT) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_MISMATCHED_TYPES, {0}};
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

    Type *cond_type = check_expr(self, cond, false);

    if (cond_type->id != TYPE_ERROR && cond_type->id != TYPE_INT) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_MISMATCHED_TYPES, {0}};
        dmsg.type_mismatch.expected = self->types->builtin_int;
        dmsg.type_mismatch.found = cond_type;

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
        Type *cond_type = check_expr(self, cond, false);

        if (cond_type->id != TYPE_ERROR && cond_type->id != TYPE_INT) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_MISMATCHED_TYPES, {0}};
            dmsg.type_mismatch.expected = self->types->builtin_int;
            dmsg.type_mismatch.found = cond_type;

            error(self, &dmsg);
        }
    }

    if (iter) {
        check_expr(self, iter, false);
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
        DiagnosticMessage dmsg = {DIAGNOSTIC_RETURN_OUTSIDE_FUNCTION, {0}};

        error(self, &dmsg);

        return;
    }

    Type *ret_type =
        expr ? check_expr(self, expr, false) : self->types->builtin_void;
    Type *expected = self->env.curr_fn->type;

    if (ret_type->id == TYPE_ERROR) {
        return;
    }

    if (ret_type != self->types->builtin_void &&
        expected == self->types->builtin_void) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_VOID_RETURN, {0}};

        error(self, &dmsg);
    } else if (expected != ret_type) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_MISMATCHED_TYPES, {0}};
        dmsg.type_mismatch.expected = self->env.curr_fn->type;
        dmsg.type_mismatch.found = ret_type;

        error(self, &dmsg);
    }
}

static void check_print(SemChecker *self, const AstNode *node) {
    Type *expr_type = check_expr(self, node->kw_print.expr, false);

    if (expr_type->id != TYPE_ERROR && expr_type->id != TYPE_STRING) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_MISMATCHED_TYPES, {0}};
        dmsg.type_mismatch.expected = self->types->builtin_string;
        dmsg.type_mismatch.found = expr_type;

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
    case AST_NODE_PRINT:
    case AST_NODE_PRINTLN:
        check_print(self, node);

        break;
    case AST_NODE_BREAK:
        if (self->loop_depth <= 0) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BREAK_OUTSIDE_LOOP, {0}};

            error(self, &dmsg);
        }

        break;
    case AST_NODE_CONTINUE:
        if (self->loop_depth <= 0) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_CONTINUE_OUTSIDE_LOOP, {0}};

            error(self, &dmsg);
        }

        break;
    case AST_NODE_RETURN:
        check_return(self, node);

        break;
    default:
        check_expr(self, node, false);

        break;
    }
}

void semck_init(SemChecker *self, TypeSystem *types) {
    self->types = types;
    env_init(&self->env);

    vec_init(&self->dmsgs, sizeof(DiagnosticMessage));
    self->error_state = false;
    self->loop_depth = 0;
}

void semck_deinit(SemChecker *self) {
    env_deinit(&self->env);

    vec_deinit(&self->dmsgs);
    self->error_state = false;
}

// Value value_clone(const Value *self) {
//     Value val = *self;
//
//     val.type = self->type;
//
//     if (val.type->id == TYPE_STRING) {
//         str_dup_n(&val.s, self->s.data, self->s.len);
//     }
//
//     return val;
// }

Variable *var_clone(const Variable *var) {
    Variable *var_copy = malloc(sizeof(*var));
    var_copy->type = var->type;
    var_copy->name = cstr_dup(var->name);
    // var_copy->val = value_clone(&var->val);
    var_copy->defined = var->defined;
    var_copy->is_param = var->is_param;

    return var_copy;
}

void clone_fn_params(
    Vector *self, const Vector *params_vec
) {
    const Variable *params = params_vec->data;

    for (size_t i = 0; i < params_vec->len; ++i) {
        Variable *var_copy = var_clone(&params[i]);
        vec_push(self, var_copy);
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

            hashmap_add(
                &self->env.global_scope->vars, var_copy->name, var_copy
            );
        }
    }

    if (funcs) {
        for (HashMapIter it = hashmap_iter(funcs); it.bucket != NULL;
             hashmap_iter_next(&it)) {
            Function *func = it.bucket->value;

            Function *func_copy = malloc(sizeof(*func_copy));

            func_copy->type = type_system_register(self->types, func->type);
            func_copy->name = cstr_dup(func->name);
            clone_fn_params(&func_copy->params, &func->params);
            func_copy->body = func->body;

            hashmap_add(&self->env.funcs, func_copy->name, func_copy);
        }
    }

    const AstNode **nodes = ast->nodes.data;

    for (size_t i = 0; i < ast->nodes.len; ++i) {
        check_node(self, nodes[i]);
    }

    return !self->error_state;
}

void semck_reset(SemChecker *self) {
    env_reset(&self->env);
}
