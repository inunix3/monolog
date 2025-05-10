#include <monolog/ast.h>
#include <monolog/utils.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

AstNode *astnode_new(AstNodeKind kind) {
    AstNode *node = mem_alloc(sizeof(*node));

    if (!node) {
        return NULL;
    }

    node->kind = kind;

    return node;
}

void astnode_destroy(AstNode *self) {
    if (!self) {
        return;
    }

    switch (self->kind) {
    case AST_NODE_ERROR:
    case AST_NODE_INTEGER:
        break;
    case AST_NODE_STRING:
        str_deinit(&self->literal.str);

        break;
    case AST_NODE_IDENT:
        str_deinit(&self->ident.str);

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
    case AST_NODE_SUFFIX:
        astnode_destroy(self->suffix.left);
        self->suffix.left = NULL;

        break;
    case AST_NODE_GROUPING:
        astnode_destroy(self->grouping.expr);
        self->grouping.expr = NULL;

        break;
    case AST_NODE_FN_CALL: {
        astnode_destroy(self->fn_call.name);
        self->fn_call.name = NULL;

        AstNode **values = self->fn_call.values.data;

        for (size_t i = 0; i < self->fn_call.values.len; ++i) {
            astnode_destroy(values[i]);
        }

        vec_deinit(&self->fn_call.values);

        break;
    }
    case AST_NODE_SUBSCRIPT:
        astnode_destroy(self->subscript.expr);
        self->subscript.expr = NULL;

        astnode_destroy(self->subscript.left);
        self->subscript.left = NULL;

        break;
    case AST_NODE_IF:
        astnode_destroy(self->kw_if.cond);
        self->kw_if.cond = NULL;

        astnode_destroy(self->kw_if.body);
        self->kw_if.body = NULL;

        astnode_destroy(self->kw_if.else_body);
        self->kw_if.else_body = NULL;

        break;
    case AST_NODE_WHILE:
        astnode_destroy(self->kw_while.cond);
        self->kw_while.cond = NULL;

        astnode_destroy(self->kw_while.body);
        self->kw_while.body = NULL;

        break;
    case AST_NODE_FOR:
        astnode_destroy(self->kw_for.init);
        self->kw_for.init = NULL;

        astnode_destroy(self->kw_for.cond);
        self->kw_for.cond = NULL;

        astnode_destroy(self->kw_for.iter);
        self->kw_for.iter = NULL;

        astnode_destroy(self->kw_for.body);
        self->kw_for.body = NULL;

        break;
    case AST_NODE_BLOCK: {
        AstNode **nodes = self->block.nodes.data;

        for (size_t i = 0; i < self->block.nodes.len; ++i) {
            astnode_destroy(nodes[i]);
        }

        vec_deinit(&self->block.nodes);

        break;
    }
    case AST_NODE_INT_TYPE:
    case AST_NODE_STRING_TYPE:
    case AST_NODE_VOID_TYPE:
        break;
    case AST_NODE_OPTION_TYPE:
        astnode_destroy(self->opt_type.type);
        self->opt_type.type = NULL;

        break;
    case AST_NODE_LIST_TYPE:
        astnode_destroy(self->list_type.type);
        self->list_type.type = NULL;

        break;
    case AST_NODE_VAR_DECL:
        astnode_destroy(self->var_decl.type);
        self->var_decl.type = NULL;

        astnode_destroy(self->var_decl.name);
        self->var_decl.name = NULL;

        astnode_destroy(self->var_decl.rvalue);
        self->var_decl.rvalue = NULL;

        break;
    case AST_NODE_PARAM_DECL:
        astnode_destroy(self->param_decl.type);
        self->param_decl.type = NULL;

        astnode_destroy(self->param_decl.name);
        self->param_decl.name = NULL;

        break;
    case AST_NODE_FN_DECL:
        astnode_destroy(self->fn_decl.type);
        self->fn_decl.type = NULL;

        astnode_destroy(self->fn_decl.name);
        self->fn_decl.name = NULL;

        AstNode **values = self->fn_decl.params.data;

        for (size_t i = 0; i < self->fn_decl.params.len; ++i) {
            astnode_destroy(values[i]);
        }

        vec_deinit(&self->fn_decl.params);

        astnode_destroy(self->fn_decl.body);
        self->fn_decl.body = NULL;

        break;
    case AST_NODE_RETURN:
        astnode_destroy(self->kw_return.expr);
        self->kw_return.expr = NULL;

        break;
    case AST_NODE_BREAK:
    case AST_NODE_CONTINUE:
    case AST_NODE_NIL:
        break;
    }

    free(self);
}

static void print_node(const AstNode *node, FILE *out, int indent) {
    for (int i = 0; i < indent; ++i) {
        fprintf(out, "  ");
    }

    if (!node) {
        fprintf(out, "null\n");

        return;
    }

    switch (node->kind) {
    case AST_NODE_ERROR:
        fprintf(out, "error\n");

        break;
    case AST_NODE_INTEGER:
        fprintf(out, "literal %" PRId64 "\n", node->literal.i);

        break;
    case AST_NODE_STRING:
        fprintf(out, "literal \"%s\"\n", node->literal.str.data);

        break;
    case AST_NODE_IDENT:
        fprintf(out, "identifier %s\n", node->ident.str.data);

        break;
    case AST_NODE_UNARY:
        fprintf(out, "unary (%s):\n", token_kind_to_str(node->unary.op));
        print_node(node->unary.right, out, indent + 1);

        break;
    case AST_NODE_BINARY:
        fprintf(out, "binary (%s):\n", token_kind_to_str(node->binary.op));
        print_node(node->binary.left, out, indent + 1);
        print_node(node->binary.right, out, indent + 1);

        break;
    case AST_NODE_SUFFIX:
        fprintf(out, "suffix (%s):\n", token_kind_to_str(node->suffix.op));
        print_node(node->suffix.left, out, indent + 1);

        break;
    case AST_NODE_GROUPING:
        fprintf(out, "grouping:\n");
        print_node(node->grouping.expr, out, indent + 1);

        break;
    case AST_NODE_FN_CALL: {
        fprintf(out, "fn-call:\n");
        print_node(node->fn_call.name, out, indent + 1);

        AstNode **values = node->fn_call.values.data;

        for (size_t i = 0; i < node->fn_call.values.len; ++i) {
            print_node(values[i], out, indent + 1);
        }

        break;
    }
    case AST_NODE_SUBSCRIPT:
        fprintf(out, "subscript:\n");
        print_node(node->subscript.expr, out, indent + 1);
        print_node(node->subscript.left, out, indent + 1);

        break;
    case AST_NODE_IF:
        if (node->kw_if.else_body) {
            fprintf(out, "if-else:\n");
        } else {
            fprintf(out, "if:\n");
        }

        print_node(node->kw_if.cond, out, indent + 1);
        print_node(node->kw_if.body, out, indent + 1);

        if (node->kw_if.else_body) {
            print_node(node->kw_if.else_body, out, indent + 1);
        }

        break;
    case AST_NODE_BLOCK: {
        fprintf(out, "block (%zu):\n", node->block.nodes.len);

        AstNode **nodes = node->block.nodes.data;

        for (size_t i = 0; i < node->block.nodes.len; ++i) {
            print_node(nodes[i], out, indent + 1);
        }

        break;
    }
    case AST_NODE_WHILE:
        fprintf(out, "while:\n");

        print_node(node->kw_while.cond, out, indent + 1);
        print_node(node->kw_while.body, out, indent + 1);

        break;
    case AST_NODE_FOR:
        fprintf(out, "for:\n");
        print_node(node->kw_for.init, out, indent + 1);
        print_node(node->kw_for.cond, out, indent + 1);
        print_node(node->kw_for.iter, out, indent + 1);
        print_node(node->kw_for.body, out, indent + 1);

        break;
    case AST_NODE_INT_TYPE:
        fprintf(out, "int-type\n");

        break;
    case AST_NODE_STRING_TYPE:
        fprintf(out, "string-type\n");

        break;
    case AST_NODE_VOID_TYPE:
        fprintf(out, "void-type\n");

        break;
    case AST_NODE_OPTION_TYPE:
        fprintf(out, "option-type:\n");
        print_node(node->opt_type.type, out, indent + 1);

        break;
    case AST_NODE_LIST_TYPE:
        fprintf(out, "list-type:\n");
        print_node(node->list_type.type, out, indent + 1);

        break;
    case AST_NODE_VAR_DECL:
        fprintf(out, "var-decl:\n");
        print_node(node->var_decl.type, out, indent + 1);
        print_node(node->var_decl.name, out, indent + 1);
        print_node(node->var_decl.rvalue, out, indent + 1);

        break;
    case AST_NODE_PARAM_DECL:
        fprintf(out, "param-decl:\n");
        print_node(node->param_decl.type, out, indent + 1);
        print_node(node->param_decl.name, out, indent + 1);

        break;
    case AST_NODE_FN_DECL: {
        fprintf(out, "fn-decl:\n");
        print_node(node->fn_decl.type, out, indent + 1);
        print_node(node->fn_decl.name, out, indent + 1);

        AstNode **nodes = node->fn_decl.params.data;

        for (size_t i = 0; i < node->fn_decl.params.len; ++i) {
            print_node(nodes[i], out, indent + 1);
        }

        print_node(node->fn_decl.body, out, indent + 1);

        break;
    }
    case AST_NODE_RETURN:
        fprintf(out, "return:\n");
        print_node(node->kw_return.expr, out, indent + 1);

        break;
    case AST_NODE_BREAK:
        fprintf(out, "break\n");

        break;
    case AST_NODE_CONTINUE:
        fprintf(out, "continue\n");

        break;
    case AST_NODE_NIL:
        fprintf(out, "nil\n");

        break;
    }
}

void ast_destroy(Ast *self) {
    AstNode **nodes = self->nodes.data;

    for (size_t i = 0; i < self->nodes.len; ++i) {
        astnode_destroy(nodes[i]);
        nodes[i] = NULL;
    }

    vec_deinit(&self->nodes);
}

void ast_dump(const Ast *self, FILE *out) {
    const AstNode **nodes = self->nodes.data;

    for (size_t i = 0; i < self->nodes.len; ++i) {
        print_node(nodes[i], out, 0);
    }
}
