#include "ast.h"
#include <stdio.h>
#include <stdlib.h>


AstNode* create_num_node(double num) {
    AstNode* node = (AstNode*) malloc(sizeof(AstNode));
    node->type = AST_NUM;
    node->number = num;
    return node;
}

AstNode* create_var_node() {
    AstNode* node = (AstNode*) malloc(sizeof(AstNode));
    node->type = AST_VAR;
    return node;
}

AstNode* create_op_node(Operator op, AstNode* left, AstNode* right) {
    AstNode* node = (AstNode*) malloc(sizeof(AstNode));
    node->type = AST_OP;
    node->op.op = op;
    node->op.left = left;
    node->op.right = right;
    return node;
}

AstNode* create_func_node(Function func, AstNode* arg) {
    AstNode* node = (AstNode*) malloc(sizeof(AstNode));
    node->type = AST_FUNC;
    node->func.func = func;
    node->func.arg = arg;
    return node;
}

AstNode* create_unary_node(Unary unary, AstNode* operand) {
    AstNode* node = (AstNode*) malloc(sizeof(AstNode));
    node->type = AST_UNARY;
    node->unary.unary = unary;
    node->unary.operand = operand;
    return node;
}


AstNode* clone_ast_node(AstNode *node) {
    if (node == NULL) return NULL;

    if (node->type == AST_NUM) {
        return create_num_node(node->number);
    } else if (node->type == AST_VAR) {
        return create_var_node();
    } else if (node->type == AST_OP) {
        AstNode* left = clone_ast_node(node->op.left);
        AstNode* right = clone_ast_node(node->op.right);

        return create_op_node(node->op.op, left, right);
    } else if (node->type == AST_FUNC) {
        AstNode* arg = clone_ast_node(node->func.arg);

        return create_func_node(node->func.func, arg);
    } else if (node->type == AST_UNARY) {
        AstNode* operand = clone_ast_node(node->unary.operand);

        return create_unary_node(node->unary.unary, operand);
    } else {
        return NULL;
    }
}


void destroy_ast_node(AstNode* node) {
    if (node == NULL) return;

    if (node->type == AST_OP) {
        destroy_ast_node(node->op.left);
        destroy_ast_node(node->op.right);
    } else if (node->type == AST_FUNC) {
        destroy_ast_node(node->func.arg);
    } else if (node->type == AST_UNARY) {
        destroy_ast_node(node->unary.operand);
    }

    free(node);
}


void print_ast_node(AstNode *node, int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");;
    }

    switch (node->type) {
    case AST_NUM:
        printf("[NUM: %lf]\n", node->number);
        break;
    case AST_VAR:
        printf("[VAR: x]\n");
        break;
    case AST_OP:
        printf("[OP: %d]\n", node->op.op);
        print_ast_node(node->op.left, indent+1);
        print_ast_node(node->op.right, indent+1);
        break;
    case AST_FUNC:
        printf("[FUNC: %d]\n", node->func.func);
        print_ast_node(node->func.arg, indent+1);
        break;
    case AST_UNARY:
        printf("[UNARY: %d]\n", node->unary.unary);
        print_ast_node(node->unary.operand, indent+1);
    }
}
