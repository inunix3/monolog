#include <monolog/ast.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

AstNode *astnode_new(AstNodeKind kind) {
    AstNode *node = calloc(1, sizeof(*node));

    if (!node) {
        return NULL;
    }

    node->kind = kind;

    return node;
}

void astnode_destroy(AstNode *self) {
    switch (self->kind) {
    case AST_NODE_ERROR:
    case AST_NODE_INTEGER:
        break;
    case AST_NODE_UNARY:
        astnode_destroy(self->unary.right);
        self->unary.right = NULL;

        break;
    case AST_NODE_BINARY:
        astnode_destroy(self->binary.left);
        self->binary.left = NULL;
        astnode_destroy(self->binary.right);
        self->binary.right = NULL;

        break;
    case AST_NODE_GROUPING:
        astnode_destroy(self->grouping.expr);
        self->grouping.expr = NULL;

        break;
    }

    free(self);
}

static void print_node(const AstNode *node, FILE *out, int indent) {
    for (int i = 0; i < indent; ++i) {
        fprintf(out, "  ");
    }

    switch (node->kind) {
    case AST_NODE_ERROR:
        fprintf(out, "error\n");

        break;
    case AST_NODE_INTEGER:
        fprintf(out, "literal %" PRId64 "\n", node->literal.i);

        break;
    case AST_NODE_UNARY:
        fprintf(
            out, "unary (%c):\n", node->unary.op == TOKEN_OP_PLUS ? '+' : '-'
        );
        print_node(node->unary.right, out, indent + 1);

        break;
    case AST_NODE_BINARY:
        fprintf(out, "binary (%s):\n", token_kind_to_str(node->binary.op));
        print_node(node->binary.left, out, indent + 1);
        print_node(node->binary.right, out, indent + 1);

        break;
    case AST_NODE_GROUPING:
        fprintf(out, "grouping:\n");
        print_node(node->grouping.expr, out, indent + 1);

        break;
    }
}

void ast_destroy(Ast *self) {
    AstNode **nodes = self->nodes.data;

    for (size_t i = 0; i < self->nodes.len; ++i) {
        astnode_destroy(nodes[i]);
    }

    vec_deinit(&self->nodes);
}

void ast_dump(const Ast *self, FILE *out) {
    const AstNode **nodes = self->nodes.data;

    for (size_t i = 0; i < self->nodes.len; ++i) {
        print_node(nodes[i], out, 0);
    }
}
