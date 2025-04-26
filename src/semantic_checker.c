#include <monolog/semantic_checker.h>
#include <monolog/type.h>

#include <stdlib.h>

static Type g_builtin_types[] = {{TYPE_ERROR}, {TYPE_INT},   {TYPE_STRING},
                                 {TYPE_VOID},  {TYPE_ARRAY}, {TYPE_OPTION}};

#define BUILTIN(_type) (&g_builtin_types[_type])

static bool type_equal(const Type *t1, const Type *t2) {
    if (t1->id != t2->id) {
        return false;
    }

    switch (t1->id) {
    case TYPE_INT:
    case TYPE_STRING:
    case TYPE_VOID:
        return true;
    case TYPE_OPTION:
        return type_equal(t1->opt_type.type, t2->opt_type.type);
    case TYPE_ARRAY:
        return type_equal(t1->array_type.type, t2->array_type.type);
    default:
        return false;
    }
}

void error(SemChecker *self, const DiagnosticMessage *dmsg) {
    self->error_state = true;

    vec_push(&self->dmsgs, dmsg);
}

static Type *check_expr(SemChecker *self, const AstNode *node);

static Type *check_binary(SemChecker *self, const AstNode *node) {
    TokenKind op = node->binary.op;
    Type *t1 = check_expr(self, node->binary.left);
    Type *t2 = check_expr(self, node->binary.right);

    if (t1->id == TYPE_ERROR || t2->id == TYPE_ERROR) {
        return BUILTIN(TYPE_ERROR);
    }

    switch (op) {
    case TOKEN_OP_PLUS:
        if (type_equal(t1, BUILTIN(TYPE_INT)) &&
            type_equal(t2, BUILTIN(TYPE_INT))) {
            return BUILTIN(TYPE_INT);
        } else if (type_equal(t1, BUILTIN(TYPE_STRING)) &&
                   type_equal(t2, BUILTIN(TYPE_STRING))) {
            return BUILTIN(TYPE_STRING);
        } else {
            DiagnosticMessage dmsg;
            dmsg.kind = DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION;
            dmsg.binary_op_comb.op = op;
            dmsg.binary_op_comb.t1 = t1;
            dmsg.binary_op_comb.t2 = t2;
            error(self, &dmsg);

            return BUILTIN(TYPE_ERROR);
        }
    case TOKEN_OP_MINUS:
    case TOKEN_OP_MUL:
    case TOKEN_OP_DIV:
    case TOKEN_OP_MOD:
    case TOKEN_OP_LESS:
    case TOKEN_OP_GREATER:
    case TOKEN_OP_EQUAL:
    case TOKEN_OP_NOT_EQUAL:
    case TOKEN_OP_LESS_EQUAL:
    case TOKEN_OP_GREATER_EQUAL:
    case TOKEN_OP_AND:
    case TOKEN_OP_OR:
        if (type_equal(t1, BUILTIN(TYPE_INT)) &&
            type_equal(t2, BUILTIN(TYPE_INT))) {
            return BUILTIN(TYPE_INT);
        } else {
            DiagnosticMessage dmsg;
            dmsg.kind = DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION;
            dmsg.binary_op_comb.op = op;
            dmsg.binary_op_comb.t1 = t1;
            dmsg.binary_op_comb.t2 = t2;
            error(self, &dmsg);

            return BUILTIN(TYPE_ERROR);
        }
    default: {
        DiagnosticMessage dmsg = {DIAGNOSTIC_INTERNAL_ERROR};
        error(self, &dmsg);

        return BUILTIN(TYPE_ERROR);
    }
    }
}

static Type *check_unary(SemChecker *self, const AstNode *node) {
    TokenKind op = node->unary.op;
    Type *type = check_expr(self, node->unary.right);

    if (type->id == TYPE_ERROR) {
        return BUILTIN(TYPE_ERROR);
    }

    switch (op) {
    case TOKEN_OP_PLUS:
    case TOKEN_OP_MINUS:
    case TOKEN_OP_EXCL:
        if (type_equal(type, BUILTIN(TYPE_INT))) {
            return BUILTIN(TYPE_INT);
        } else {
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_UNARY_OPERAND_COMBINATION};
            dmsg.unary_op_comb.op = op;
            dmsg.unary_op_comb.type = type;
            error(self, &dmsg);

            return BUILTIN(TYPE_ERROR);
        }
    default: {
        DiagnosticMessage dmsg = {DIAGNOSTIC_INTERNAL_ERROR};
        error(self, &dmsg);

        return BUILTIN(TYPE_ERROR);
    }
    }
}

static Type *check_ident(SemChecker *self, const AstNode *node) {
    SemVariable *var = NULL;

    for (int i = self->scopes.len; i >= 0; --i) {
        const SemScope *scopes = self->scopes.data;
        const SemScope *scope = &scopes[i];

        var = hashmap_get(&scope->variables, node->ident.str.data);

        if (var) {
            break;
        }
    }

    if (!var) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_UNDECLARED_SYMBOL};
        dmsg.undef_sym.name = node->ident.str.data;

        error(self, &dmsg);

        return BUILTIN(TYPE_ERROR);
    } else if (!var->defined) {
        DiagnosticMessage dmsg = {DIAGNOSTIC_UNDEFINED_VARIABLE};
        dmsg.undef_sym.name = node->ident.str.data;

        error(self, &dmsg);

        return BUILTIN(TYPE_ERROR);
    }

    return var->type;
}

static Type *check_expr(SemChecker *self, const AstNode *node) {
    switch (node->kind) {
    case AST_NODE_INTEGER:
        return BUILTIN(TYPE_INT);
    case AST_NODE_STRING:
        return BUILTIN(TYPE_STRING);
    case AST_NODE_IDENT:
        return check_ident(self, node);
    case AST_NODE_BINARY:
        return check_binary(self, node);
    case AST_NODE_UNARY:
        return check_unary(self, node);
    case AST_NODE_GROUPING:
        return check_expr(self, node->grouping.expr);
    default:
        return BUILTIN(TYPE_ERROR);
    }
}

static Type *kw_to_type(AstNodeKind kind) {
    switch (kind) {
    case AST_NODE_INT_TYPE:
        return BUILTIN(TYPE_INT);
    case AST_NODE_STRING_TYPE:
        return BUILTIN(TYPE_STRING);
    case AST_NODE_VOID_TYPE:
        return BUILTIN(TYPE_VOID);
    case AST_NODE_ARRAY_TYPE:
        return BUILTIN(TYPE_ARRAY);
    case AST_NODE_OPTION_TYPE:
        return BUILTIN(TYPE_OPTION);
    default:
        return BUILTIN(TYPE_ERROR);
    }
}

static void check_var_decl(SemChecker *self, const AstNode *node) {
    const AstNode *rvalue = node->var_decl.rvalue;
    Type *type = kw_to_type(node->var_decl.type->kind);
    const char *name = node->var_decl.name->ident.str.data;

    if (rvalue) {
        Type *value_type = check_expr(self, node->var_decl.rvalue);

        if (value_type->id != TYPE_ERROR && !type_equal(type, value_type)) {
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

    hashmap_add(&self->curr_scope->variables, name, var);
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
    case AST_NODE_BLOCK:
        check_block(self, node);

        break;
    default:
        check_expr(self, node);

        break;
    }
}

void semscope_init(SemScope *self) { hashmap_init(&self->variables); }

void semscope_deinit(SemScope *self) { hashmap_deinit(&self->variables); }

void semck_init(SemChecker *self) {
    vec_init(&self->scopes, sizeof(SemScope));
    vec_init(&self->dmsgs, sizeof(DiagnosticMessage));
    self->error_state = false;

    SemScope global_scope;
    semscope_init(&global_scope);
    vec_push(&self->scopes, &global_scope);

    self->curr_scope = &VEC_LAST(&self->scopes, SemScope);
}

void semck_deinit(SemChecker *self) {
    SemScope *scopes = self->scopes.data;

    for (size_t i = 0; i < self->scopes.len; ++i) {
        SemScope *scope = &scopes[i];

        for (HashMapIter it = hashmap_iter(&scope->variables);
             it.bucket != NULL; hashmap_iter_next(&it)) {
            free(it.bucket->value);
        }

        semscope_deinit(&scopes[i]);
    }

    vec_deinit(&self->scopes);

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
