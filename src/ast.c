#include "ast.h"
#include "calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

// no recursive
void destroy_ast_node_only(AstNode* node) {
    if (node == NULL) return;
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


static char* strmerge(const char* a, const char* b) {
    char* result = malloc(strlen(a) + strlen(b) + 1);
    strcpy(result, a);
    strcat(result, b);
    return result;
}

static char* strmerge3(const char* a, const char* b, const char* c) {
    char* ab = strmerge(a, b);
    char* abc = strmerge(ab, c);
    free(ab);
    return abc;
}

// operator priority
static int precedence(Operator op) {
    switch (op) {
        case OP_ADD:
        case OP_SUB: return 1;
        case OP_MUL:
        case OP_DIV: return 2;
        case OP_POW: return 3;
        default: return 0;
    }
}

static char* function_to_str(Function func) {
    switch (func) {
    case FUNC_SIN:
        return "sin";
    case FUNC_COS:
        return "cos";
    case FUNC_TAN:
        return "tan";
    case FUNC_LN:
        return "ln";
    case FUNC_LOG:
        return "log";
    case FUNC_EXP:
        return "exp";
    case FUNC_INVALID:
        return "<?>";
    }
}
char* ast_to_infix(AstNode* node) {
    if (!node) return strdup("<?>");

    switch (node->type) {
        case AST_NUM: {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.10g", node->number);
            return strdup(buf);
        }
        case AST_VAR:
            return strdup("x");
        case AST_OP: {
            char* left_str = ast_to_infix(node->op.left);
            char* right_str = ast_to_infix(node->op.right);

            // is pathensesis needed
            int my_prec = precedence(node->op.op);
            int left_prec = (node->op.left && node->op.left->type == AST_OP) ? precedence(node->op.left->op.op) : 99;
            int right_prec = (node->op.right && node->op.right->type == AST_OP) ? precedence(node->op.right->op.op) : 99;

            if (left_prec < my_prec) {
                char* temp = left_str;
                left_str = strmerge3("(", temp, ")");
                free(temp);
            }

            if (right_prec < my_prec || (node->op.op == OP_SUB && right_prec == my_prec)) {
                char* temp = right_str;
                right_str = strmerge3("(", temp, ")");
                free(temp);
            }

            const char* op_str = NULL;
            switch (node->op.op) {
                case OP_ADD: op_str = " + "; break;
                case OP_SUB: op_str = " - "; break;
                case OP_MUL: op_str = " * "; break;
                case OP_DIV: op_str = " / "; break;
                case OP_POW: op_str = " ^ "; break;
                default:     op_str = " ? "; break;
            }

            char* expr = strmerge3(left_str, op_str, right_str);
            free(left_str);
            free(right_str);
            return expr;
        }
        case AST_FUNC: {
            char* arg_str = ast_to_infix(node->func.arg);
            char* temp = strmerge3(function_to_str(node->func.func), "(", arg_str);
            char* result = strmerge(temp, ")");
            free(temp);
            free(arg_str);
            return result;
        }
        case AST_UNARY: {
            char* operand_str = ast_to_infix(node->unary.operand);
            char* result = strmerge3("-(", operand_str, ")");
            free(operand_str);
            return result;
        }
    }
}
