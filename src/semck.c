#include <monolog/semck.h>
#include <monolog/type.h>

#include <stdlib.h>
#include <string.h>

void error(SemChecker *self, const DiagnosticMessage *dmsg) {
    self->error_state = true;

    vec_push(&self->dmsgs, dmsg);
}

static Type *check_expr(SemChecker *self, const AstNode *node, bool assigning);
static SemVariable *find_var(SemChecker *self, const char *name);

static bool expr_is_mutable(SemChecker *self, const AstNode *node) {
    switch (node->kind) {
    case AST_NODE_IDENT:
        if (find_var(self, node->ident.str.data)) {
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
        SemVariable *var = find_var(self, node->ident.str.data);

        var->defined = true;

        return var->type;
    }
    case AST_NODE_GROUPING:
        return assign_expr(self, node->grouping.expr, type);
    default:
        return self->error_type;
    }
}

static Type *check_binary(SemChecker *self, const AstNode *node) {
    TokenKind op = node->binary.op;
    Type *t1 = check_expr(self, node->binary.left, op == TOKEN_OP_ASSIGN);
    Type *t2 = check_expr(self, node->binary.right, false);

    if (t1->id == TYPE_ERROR || t2->id == TYPE_ERROR) {
        return self->error_type;
    }

    switch (op) {
    case TOKEN_OP_PLUS:
        if (type_equal(t1, self->builtin_int) &&
            type_equal(t2, self->builtin_int)) {
            return self->builtin_int;
        } else if (type_equal(t1, self->builtin_string) &&
                   type_equal(t2, self->builtin_string)) {
            return self->builtin_string;
        } else {
            DiagnosticMessage dmsg;
            dmsg.kind = DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION;
            dmsg.binary_op_comb.op = op;
            dmsg.binary_op_comb.t1 = t1;
            dmsg.binary_op_comb.t2 = t2;
            error(self, &dmsg);

            return self->error_type;
        }
    case TOKEN_OP_EQUAL:
    case TOKEN_OP_NOT_EQUAL:
        if ((t1->id == TYPE_OPTION && t2->id == TYPE_NIL) ||
            (t1->id == TYPE_NIL && t2->id == TYPE_OPTION)) {
            return self->builtin_int;
        } else if (t1->id == TYPE_STRING && t2->id == TYPE_STRING) {
            return self->builtin_int;
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
        if (type_equal(t1, self->builtin_int) &&
            type_equal(t2, self->builtin_int)) {
            return self->builtin_int;
        } else {
            DiagnosticMessage dmsg;
            dmsg.kind = DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION;
            dmsg.binary_op_comb.op = op;
            dmsg.binary_op_comb.t1 = t1;
            dmsg.binary_op_comb.t2 = t2;

            error(self, &dmsg);

            return self->error_type;
        }
    case TOKEN_OP_ASSIGN:
        if (!expr_is_mutable(self, node->binary.left)) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_EXPR_NOT_MUTABLE};

            error(self, &dmsg);

            return self->error_type;
        }

        if (!type_can_implicitly_convert(t2, t1)) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_MISMATCHED_TYPES};
            dmsg.type_mismatch.expected = t1;
            dmsg.type_mismatch.found = t2;

            error(self, &dmsg);

            return self->error_type;
        }

        return assign_expr(self, node->binary.left, t2);
    default: {
        DiagnosticMessage dmsg = {DIAGNOSTIC_INTERNAL_ERROR};
        error(self, &dmsg);

        return self->error_type;
    }
    }
}

static Type *check_unary(SemChecker *self, const AstNode *node) {
    TokenKind op = node->unary.op;
    Type *type = check_expr(self, node->unary.right, false);

    if (type->id == TYPE_ERROR) {
        return self->error_type;
    }

    switch (op) {
    case TOKEN_OP_PLUS:
    case TOKEN_OP_MINUS:
    case TOKEN_OP_EXCL:
        if (type_equal(type, self->builtin_int)) {
            return self->builtin_int;
        } else {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION};
            dmsg.unary_op_comb.op = op;
            dmsg.unary_op_comb.type = type;
            error(self, &dmsg);

            return self->error_type;
        }
    case TOKEN_OP_INC:
    case TOKEN_OP_DEC:
        if (!expr_is_mutable(self, node->unary.right)) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_EXPR_NOT_MUTABLE};

            error(self, &dmsg);

            return self->error_type;
        }

        if (type->id != TYPE_INT) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION};
            dmsg.unary_op_comb.op = op;
            dmsg.unary_op_comb.type = type;
            error(self, &dmsg);

            return self->error_type;
        }

        return type;
    case TOKEN_OP_HASHTAG:
        if (type->id != TYPE_ARRAY && type->id != TYPE_STRING) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION};
            dmsg.unary_op_comb.op = op;
            dmsg.unary_op_comb.type = type;
            error(self, &dmsg);

            return self->error_type;
        }

        return self->builtin_int;
    case TOKEN_OP_DOLAR:
        if (type->id != TYPE_INT) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION};
            dmsg.unary_op_comb.op = op;
            dmsg.unary_op_comb.type = type;
            error(self, &dmsg);

            return self->error_type;
        }

        return self->builtin_int;
    case TOKEN_OP_MUL:
        if (type->id != TYPE_OPTION) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION};
            dmsg.unary_op_comb.op = op;
            dmsg.unary_op_comb.type = type;
            error(self, &dmsg);

            return self->error_type;
        }

        return type->opt_type.type;
    default: {
        DiagnosticMessage dmsg = {DIAGNOSTIC_INTERNAL_ERROR};
        error(self, &dmsg);

        return self->error_type;
    }
    }
}

static Type *check_suffix(SemChecker *self, const AstNode *node) {
    TokenKind op = node->suffix.op;
    Type *type = check_expr(self, node->suffix.left, false);

    if (type->id == TYPE_ERROR) {
        return self->error_type;
    }

    switch (op) {
    case TOKEN_OP_INC:
    case TOKEN_OP_DEC:
        if (!expr_is_mutable(self, node->suffix.left)) {
            DiagnosticMessage dmsg = {DIAGNOSTIC_EXPR_NOT_MUTABLE};

            error(self, &dmsg);

            return self->error_type;
        }

        if (type->id != TYPE_INT) {
            DiagnosticMessage dmsg;
            dmsg.kind = DIAGNOSTIC_BAD_SUFFIX_OPERAND_COMBINATION;
            dmsg.suffix_op_comb.op = op;
            dmsg.suffix_op_comb.type = type;
            error(self, &dmsg);

            return self->error_type;
        }

        return type;
    default:
        return self->error_type;
    }
}

static SemVariable *find_var(SemChecker *self, const char *name) {
    SemVariable *var = NULL;

    for (int i = self->scopes.len; i >= 0; --i) {
        const SemScope *scopes = self->scopes.data;
        const SemScope *scope = &scopes[i];

        var = hashmap_get(&scope->vars, name);

        if (var) {
            break;
        }
    }

    return var;
}

static bool check_var(
    SemChecker *self, const SemVariable *var, char *name, bool assigning
) {
    if (!var) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_UNDECLARED_VARIABLE};
        dmsg.undef_sym.name = name;

        error(self, &dmsg);

        return false;
    } else if (!assigning && (var->type->id != TYPE_ARRAY && !var->defined) &&
               !var->is_param) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_UNDEFINED_VARIABLE};
        dmsg.undef_sym.name = name;

        error(self, &dmsg);

        return false;
    }

    return true;
}

static Type *
check_ident(SemChecker *self, const AstNode *node, bool assigning) {
    SemVariable *var = find_var(self, node->ident.str.data);

    if (!check_var(self, var, node->ident.str.data, assigning)) {
        return self->error_type;
    }

    return var->type;
}

static Type *check_fn_call(SemChecker *self, const AstNode *node) {
    char *name = node->fn_call.name->ident.str.data;

    SemFunction *fn = hashmap_get(&self->funcs, name);

    if (!fn) {
        DiagnosticMessage dmsg;
        dmsg.kind = DIAGNOSTIC_UNDECLARED_FUNCTION;
        dmsg.undef_sym.name = name;

        error(self, &dmsg);

        return self->error_type;
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

    const SemVariable *params = fn->params.data;
    const AstNode **values = values_vec->data;

    for (size_t i = 0; i < values_vec->len; ++i) {
        const AstNode *value = values[i];
        const SemVariable *param = &params[i];

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
        return self->error_type;
    } else if (left_type->id != TYPE_ARRAY && left_type->id != TYPE_STRING) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_EXPR_NOT_INDEXABLE};

        error(self, &dmsg);

        return self->error_type;
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
        return self->builtin_int;
    }
}

static Type *check_expr(SemChecker *self, const AstNode *node, bool assigning) {
    switch (node->kind) {
    case AST_NODE_INTEGER:
        return self->builtin_int;
    case AST_NODE_STRING:
        return self->builtin_string;
    case AST_NODE_IDENT:
        return check_ident(self, node, assigning);
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
        return self->error_type;
    }
}

static Type *parse_type(SemChecker *self, const AstNode *node) {
    switch (node->kind) {
    case AST_NODE_INT_TYPE:
        return self->builtin_int;
    case AST_NODE_STRING_TYPE:
        return self->builtin_string;
    case AST_NODE_VOID_TYPE:
        return self->builtin_void;
    case AST_NODE_ARRAY_TYPE: {
        Type *contained_type = parse_type(self, node->array_type.type);
        Type array = {TYPE_ARRAY};
        array.array_type.type = contained_type;

        return type_system_register(&self->types, &array);
    }
    case AST_NODE_OPTION_TYPE: {
        Type *contained_type = parse_type(self, node->opt_type.type);
        Type option = {TYPE_OPTION};
        option.opt_type.type = contained_type;

        return type_system_register(&self->types, &option);
    }
    case AST_NODE_NIL:
        return self->nil_type;
    default:
        return self->error_type;
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
            DiagnosticMessage dmsg = {DIAGNOSTIC_MISMATCHED_TYPES};
            dmsg.type_mismatch.expected = type;
            dmsg.type_mismatch.found = value_type;

            error(self, &dmsg);
        }
    }

    SemVariable *var = malloc(sizeof(*var));
    var->type = type;
    var->name = name;
    var->defined = rvalue != NULL;
    var->is_param = false;

    hashmap_add(&self->curr_scope->vars, name, var);
}

static void check_fn_decl(SemChecker *self, const AstNode *node) {
    Type *type = parse_type(self, node->fn_decl.type);
    char *name = node->fn_decl.name->ident.str.data;
    const AstNode *body = node->fn_decl.body;

    if (hashmap_get(&self->funcs, name)) {
        DiagnosticMessage dmsg;
        dmsg.kind = DIAGNOSTIC_FN_REDEFINITION;
        dmsg.fn_redef.name = name;

        error(self, &dmsg);

        return;
    }

    SemFunction *fn = malloc(sizeof(*fn));
    fn->type = type;
    fn->name = name;
    fn->defined = body != NULL;

    vec_init(&fn->params, sizeof(SemVariable));

    const Vector *params_vec = &node->fn_decl.params;
    const AstNode **params = params_vec->data;

    for (size_t i = 0; i < params_vec->len; ++i) {
        const AstNode *param_node = params[i];

        type = parse_type(self, param_node->param_decl.type);
        name = param_node->param_decl.name->ident.str.data;

        for (size_t i = 0; i < fn->params.len; ++i) {
            const SemVariable *fn_params = fn->params.data;

            if (strcmp(name, fn_params[i].name) == 0) {
                DiagnosticMessage dmsg;
                dmsg.kind = DIAGNOSTIC_PARAM_REDECLARATION;
                dmsg.param_redecl.name = name;

                error(self, &dmsg);
            }
        }

        SemVariable param;
        param.type = type;
        param.name = name;
        param.defined = false;
        param.is_param = true;

        vec_push(&fn->params, &param);
    }

    hashmap_add(&self->funcs, fn->name, fn);
}

static void check_node(SemChecker *self, const AstNode *node);

static void check_block(SemChecker *self, const AstNode *node) {
    SemScope new_scope;
    semscope_init(&new_scope);
    vec_push(&self->scopes, &new_scope);

    self->curr_scope = &VEC_LAST(&self->scopes, SemScope);

    const AstNode **nodes = node->block.nodes.data;

    for (size_t i = 0; i < node->block.nodes.len; ++i) {
        check_node(self, nodes[i]);
    }

    vec_pop(&self->scopes);
    self->curr_scope = &VEC_LAST(&self->scopes, SemScope);
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
    default:
        check_expr(self, node, false);

        break;
    }
}

void semscope_init(SemScope *self) { hashmap_init(&self->vars); }

void semscope_deinit(SemScope *self) {
    hashmap_deinit(&self->vars);

    for (HashMapIter it = hashmap_iter(&self->vars); it.bucket != NULL;
         hashmap_iter_next(&it)) {
        free(it.bucket->value);
    }
}

void semck_init(SemChecker *self) {
    type_system_init(&self->types);

    Type builtin_int = {TYPE_INT};
    self->builtin_int = type_system_register(&self->types, &builtin_int);

    Type builtin_string = {TYPE_STRING};
    self->builtin_string = type_system_register(&self->types, &builtin_string);

    Type builtin_void = {TYPE_VOID};
    self->builtin_void = type_system_register(&self->types, &builtin_void);

    Type error_type = {TYPE_ERROR};
    self->error_type = type_system_register(&self->types, &error_type);

    Type nil_type = {TYPE_NIL};
    self->nil_type = type_system_register(&self->types, &nil_type);

    vec_init(&self->scopes, sizeof(SemScope));
    vec_init(&self->dmsgs, sizeof(DiagnosticMessage));
    self->error_state = false;

    SemScope global_scope;
    semscope_init(&global_scope);
    vec_push(&self->scopes, &global_scope);
    self->curr_scope = &VEC_LAST(&self->scopes, SemScope);

    hashmap_init(&self->funcs);
}

void semck_deinit(SemChecker *self) {
    SemScope *scopes = self->scopes.data;

    for (size_t i = 0; i < self->scopes.len; ++i) {
        SemScope *scope = &scopes[i];

        for (HashMapIter it = hashmap_iter(&scope->vars); it.bucket != NULL;
             hashmap_iter_next(&it)) {
            free(it.bucket->value);
        }

        semscope_deinit(&scopes[i]);
    }

    for (HashMapIter it = hashmap_iter(&self->funcs); it.bucket != NULL;
         hashmap_iter_next(&it)) {
        free(it.bucket->value);
    }

    vec_deinit(&self->scopes);
    hashmap_deinit(&self->funcs);

    type_system_deinit(&self->types);

    vec_deinit(&self->dmsgs);
    self->error_state = false;
}

bool semck_check(SemChecker *self, const Ast *ast) {
    const AstNode **nodes = ast->nodes.data;

    for (size_t i = 0; i < ast->nodes.len; ++i) {
        check_node(self, nodes[i]);
    }

    return !self->error_state;
}
