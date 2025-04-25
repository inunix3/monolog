#include <monolog/semantic_checker.h>
#include <monolog/type.h>

static Type g_builtin_types[] = {
    {TYPE_ERROR}, {TYPE_INT}, {TYPE_STRING}, {TYPE_VOID}
};

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
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION};
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
            DiagnosticMessage dmsg = {DIAGNOSTIC_BAD_BINARY_OPERAND_COMBINATION};
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

static Type *check_expr(SemChecker *self, const AstNode *node) {
    switch (node->kind) {
    case AST_NODE_INTEGER:
        return BUILTIN(TYPE_INT);
    case AST_NODE_STRING:
        return BUILTIN(TYPE_STRING);
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

static void check_node(SemChecker *self, const AstNode *node) {
    switch (node->kind) {
    default:
        check_expr(self, node);

        break;
    }
}

void semck_init(SemChecker *self) {
    vec_init(&self->dmsgs, sizeof(DiagnosticMessage));
    self->error_state = false;
}

void semck_deinit(SemChecker *self) {
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
