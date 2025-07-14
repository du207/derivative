#include "calc.h"

#include "ast.h"
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>



static bool is_node_num(AstNode* node) {
    return node->type == AST_NUM;
}

// recursively check if the node only has num
static bool is_node_only_has_num(AstNode* node) {
    if (node->type == AST_NUM) {
        return true;
    } else if (node->type == AST_OP) {
        return is_node_only_has_num(node->op.left) && is_node_only_has_num(node->op.right);
    } else if (node->type == AST_UNARY) {
        return is_node_only_has_num(node->unary.operand);
    } else if (node->type == AST_FUNC) {
        return is_node_only_has_num(node->func.arg);
    } else {
        return false;
    }
}

static bool is_node_value_num(AstNode* node, double num) {
    return node->type == AST_NUM && node->number == num;
}


static double add(double a, double b){
    return a+b;
}

static double mul(double a, double b){
    return a*b;
}

// op only can be OP_ADD or OP_MUL
static bool calculate_operator(AstNode** parent_dp) {
    AstNode* parent = *parent_dp; 
    assert(parent->type == AST_OP);
    
    Operator op = parent->op.op;
    assert(op == OP_ADD || op == OP_MUL);

    double (*calc)(double, double);
    if (op == OP_ADD) {
        calc = &add;
    } else if (op == OP_MUL) {
        calc = &mul;
    }

    AstNode* left = parent->op.left;
    AstNode* right = parent->op.right;

    AstNode *left_num = NULL, *right_num = NULL;

    bool is_left_num = is_node_num(left);
    bool is_right_num = is_node_num(right);

    if (is_left_num && is_right_num) {
        // num + num
        destroy_ast_node(parent); // destroy left, right recursively
        *parent_dp = create_num_node(calc(left->number, right->number));
        return true;

    } else if (is_left_num && right->type == AST_OP && right->op.op == op) {
        // num + (? + ?)
        AstNode* right_left = right->op.left;
        AstNode* right_right = right->op.right;
        
        // assume only one of them can be num
        if (is_node_num(right_left)) {
            parent->op.left = create_num_node(calc(left->number, right_left->number));
            parent->op.right = right_right;
            destroy_ast_node_only(right_left);
            destroy_ast_node_only(left);
            return true;
        } else if (is_node_num(right_right)) {
            parent->op.left = create_num_node(calc(left->number, right_right->number));
            parent->op.right = right_left;
            destroy_ast_node_only(right_right);
            destroy_ast_node_only(left);
            return true;
        } else {
            return false;
        }
    } else if (is_right_num && right->type == AST_OP && right->op.op == op) {
        // (non-num..) + num
        // ok.. bunch of code duplications.. shit
        AstNode* left_left = left->op.left;
        AstNode* left_right = left->op.right;
        
        // assume only one of them can be num
        if (is_node_num(left_left)) {
            parent->op.left = create_num_node(calc(right->number, left_left->number));
            parent->op.right = left_right;
            destroy_ast_node_only(left_left);
            destroy_ast_node_only(right);
            return true;
        } else if (is_node_num(left_right)) {
            parent->op.left = create_num_node(calc(right->number, left_right->number));
            parent->op.right = left_left;
            destroy_ast_node_only(left_right);
            destroy_ast_node_only(right);
            return true;
        } else {
            return false;
        }

    } else {
        // (non-num..) + (non-num..)
        if (left->op.op != op || right->op.op != op) return false;
        
        AstNode *left_num, *right_num, *left_expr, *right_expr;

        if (is_node_num(left->op.left)) {
            left_num = left->op.left;
            left_expr = left->op.right;
        } else if (is_node_num(left->op.right)) {
            left_num = left->op.right;
            left_expr = left->op.left;
        } else {
            return false;
        }

        if (is_node_num(right->op.left)) {
            right_num = right->op.left;
            right_expr = right->op.right;
        } else if (is_node_num(right->op.right)) {
            right_num = right->op.right;
            right_expr = right->op.left;
        } else {
            return false;
        }

        destroy_ast_node_only(left);
        parent->op.left = create_num_node(calc(left_num->number, right_num->number));

        right->op.left = left_expr;
        right->op.right = right_expr;

        destroy_ast_node_only(left_num);
        destroy_ast_node_only(right_num);
        return true;
    }
}

bool calculate_constant(AstNode** parent_dp, ConstantMode cm) {
    // because simplify_expression is called recursively (bottom-up)
    // the left and right node is expected to have already done constant calculation
    // so what this function has to do is
    // just calculate the constant from left and right
    // also all sub will be replaced with add, no need to handle sub
    // also all unary with number will be replaced with just a single number node

    AstNode* parent = *parent_dp;
    AstNode* left = parent->op.left;
    AstNode* right = parent->op.left;

    if (parent->type == AST_OP) {
        // sub to add
        if (parent->op.op == OP_SUB) {
            parent->op.op = OP_ADD;
            parent->op.right = create_unary_node(UNARY_MINUS, parent->op.right);
        }

        if (parent->op.op == OP_DIV) {

        }

        if (parent->op.op == OP_ADD || parent->op.op == OP_MUL) {
            return calculate_operator(parent_dp);
        }

        
    } else if (parent->type == AST_FUNC) {
        // calculate the function value
    }
}


// recursively resolve unary
static bool unary_recursive(AstNode** parent_dp) {
    AstNode* parent = *parent_dp;
    AstNode* operand = parent->unary.operand;

    // if constasnt change to negative number 
    if (is_node_num(operand)) {
        if (parent->unary.unary == UNARY_MINUS) {
            destroy_ast_node_only(parent);
            operand->number = -operand->number;
            *parent_dp = operand;
            return true;
        } else {
            destroy_ast_node_only(parent);
            *parent_dp = operand;
            return true;
        }
    } else if (operand->type == AST_OP) {
        Operator operand_op = operand->op.op;
        assert(operand_op != OP_SUB); // assume sub are all changed to add
        
        if (operand_op == OP_ADD) {
            operand->op.left = create_unary_node(UNARY_MINUS, operand->op.left);
            operand->op.left = create_unary_node(UNARY_MINUS, operand->op.right);
            unary_recursive(&operand->op.left);
            unary_recursive(&operand->op.right);
        } else if (operand_op == OP_MUL || operand_op == OP_DIV) {
            operand->op.left = create_unary_node(UNARY_MINUS, operand->op.left);
            unary_recursive(&operand->op.left);
        }
    }
}


// simplify expression in bottom-up
bool simplify_ast_node(AstNode** parent_dp, ConstantMode cm) {
    AstNode* parent = *parent_dp;
    bool is_changed = false;

    if (calculate_constant(parent_dp, cm)) is_changed = true;
    if (parent->type == AST_OP) {
        Operator op = parent->op.op;
        assert(op != OP_SUB); // assume all subs changed to add
        
        AstNode* left = parent->op.left;
        AstNode* right = parent->op.right;

        // bottom-up
        if (!simplify_ast_node(left, cm)) return false;
        if (!simplify_ast_node(right, cm)) return false;


        if (op == OP_ADD) {
            // 0+x -> x
            if (is_node_value_num(left, 0)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(left);
                *parent_dp = right;
            } else if (is_node_value_num(right, 0)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                *parent_dp = left;
            }
        } else if (op == OP_MUL) {
            // 0 * x -> 0
            if (is_node_value_num(left, 0)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                *parent_dp = left;
            } else if (is_node_value_num(right, 0)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(left);
                *parent_dp = right;
            } else if (is_node_value_num(left, 1)) {
                // 1 * x -> x
                destroy_ast_node_only(parent);
                destroy_ast_node_only(left);
                *parent_dp = right;
            } else if (is_node_value_num(right, 1)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                *parent_dp = left;
            }
        } else if (op == OP_DIV) {
            // x / 1 -> x
            if (is_node_value_num(right, 1)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                *parent_dp = left;
            }
        } else if (op == OP_POW) {
            // x ^ 1 -> x
            if (is_node_value_num(right, 1)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                *parent_dp = left;
            } else if (is_node_value_num(right, 0)) {
                // x ^ 0 -> 1 (including x = 0)
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                destroy_ast_node(left);
                *parent_dp = create_num_node(1);
            } else if (is_node_value_num(left, 0)) {
                // 0 ^ x -> 0 (if x->type = num: x->number > 0)
                if (right->type != AST_OP || right->number > 0) {
                    destroy_ast_node_only(parent);
                    destroy_ast_node(left);
                    *parent_dp = right;
                } else if (right->number < 0) {
                    fprintf(stderr, "0 ^ (%lf) is invalid!\n", right->number);
                    return false;
                }
            } else if (is_node_value_num(left, 1)) {
                // 1 ^ x = 1
                destroy_ast_node(parent);
                destroy_ast_node(right);
                *parent_dp = left;
            }
        }
    } else if (parent->type == AST_UNARY) {
        unary_recursive(parent_dp);
    }

    return true;
}



static void change_sub_to_add(AstNode** node_dp) {
    AstNode* node = *node_dp;
    
    switch (node->type) {
    case AST_OP:
        change_sub_to_add(node->op.left);
        change_sub_to_add(node->op.right);
        break;
    case AST_UNARY:
        change_sub_to_add(node->unary.operand);
        break;
    case AST_FUNC:
        change_sub_to_add(node->func.arg);
        break;
    }

    if (node->type == AST_OP && node->op.op == OP_SUB) {
        node->op.op = OP_ADD;
        node->op.right = create_unary_node(UNARY_MINUS, node->op.right);
    }
}


// Core function!!
void simplify_ast_tree(AstNode** tree_dp) {
    // preprocess stuffs:
    // change a - b = a + (-b) (sub->add)
    change_sub_to_add(tree_dp);

    ConstantMode cm = CON_FR; // TODO: make a checker!
    simplify_ast_node(tree_dp, cm); // TODO: iterate until not change

}