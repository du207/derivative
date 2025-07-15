#include "calc.h"

#include "ast.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>



static bool is_node_num(AstNode* node) {
    return node->type == AST_NUM;
}



static bool is_node_value_num(AstNode* node, double num) {
    return node->type == AST_NUM && node->number == num;
}


static AstNode* calculate_add(AstNode* a, AstNode* b){
    assert(a->type == AST_NUM && b->type == AST_NUM);
    return create_num_node(a->number + b->number);
}


static AstNode* calculate_mul(AstNode* a, AstNode* b){
    assert(a->type == AST_NUM && b->type == AST_NUM);
    return create_num_node(a->number * b->number);
}

// op only can be OP_ADD or OP_MUL
static bool calculate_operator(AstNode** parent_dp) {
    AstNode* parent = *parent_dp;
    assert(parent->type == AST_OP);

    Operator op = parent->op.op;
    assert(op == OP_ADD || op == OP_MUL);

    AstNode* (*calc)(AstNode*, AstNode*);
    if (op == OP_ADD) {
        calc = &calculate_add;
    } else if (op == OP_MUL) {
        calc = &calculate_mul;
    }

    AstNode* left = parent->op.left;
    AstNode* right = parent->op.right;

    bool is_left_num = is_node_num(left);
    bool is_right_num = is_node_num(right);

    if (is_left_num && is_right_num) {
        // num + num
        *parent_dp = calc(left, right);
        destroy_ast_node(parent); // destroy left, right recursively
        return true;

    } else if (is_left_num || is_right_num) { // Logical XOR (only one of is_left, is_right is true)
        // num + (?) or (?) + num
        AstNode **num1, **expr, **num2, **expr_child;
        if (is_left_num) {
            if (right->type != AST_OP || right->op.op != op) return false;

            num1 = &parent->op.left;
            expr = &parent->op.right;
        } else if (is_right_num) {
            if (left->type != AST_OP || left->op.op != op) return false;

            num1 = &parent->op.right;
            expr = &parent->op.left;
        }

        AstNode* expr_left = (*expr)->op.left;
        AstNode* expr_right = (*expr)->op.right;

        // assume only one of them can be num
        if (is_node_num(expr_left)) {
            num2 = &(*expr)->op.left;
            expr_child = &(*expr)->op.right;
        } else if (is_node_num(expr_right)) {
            num2 = &(*expr)->op.right;
            expr_child = &(*expr)->op.left;
        } else {
            return false;
        }

        parent->op.left = calc(*num1, *num2);
        parent->op.right = *expr_child;
        destroy_ast_node_only(*num1);
        destroy_ast_node_only(*num2);
        return true;
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
        parent->op.left = calc(left_num, right_num);

        right->op.left = left_expr;
        right->op.right = right_expr;

        destroy_ast_node_only(left_num);
        destroy_ast_node_only(right_num);
        return true;
    }
}

static bool calculate_constant(AstNode** parent_dp) {
    // because simplify_expression is called recursively (bottom-up)
    // the left and right node is expected to have already done constant calculation
    // so what this function has to do is
    // just calculate the constant from left and right
    // also all sub will be replaced with add, no need to handle sub
    // also all unary with number will be replaced with just a single number node

    AstNode* parent = *parent_dp;
    AstNode* left = parent->op.left;
    AstNode* right = parent->op.right;
    
    bool is_changed = false;

    if (parent->type == AST_OP) {

        if (parent->op.op == OP_POW) {
            if (is_node_num(left) && is_node_num(right)) {
                double l_num = left->number;
                double r_num = right->number;

                *parent_dp = create_num_node(pow(l_num, r_num));
                destroy_ast_node(parent);
                // r_num == 0 will be handled by 'simplify_ast_node()' so don't worry
                is_changed =  true;
            }
        }

        if (parent->op.op == OP_DIV) {
            if (is_node_num(right)) {
                // calculate inversion
                double r_num = right->number;
                parent->op.op = OP_MUL;
                right->number = 1.0 / r_num;
                is_changed = true;
            }
        }

        if (parent->op.op == OP_SUB) {
            if (is_node_num(right)) {
                parent->op.op = OP_ADD;
                right->number = -right->number;
                is_changed = true;
            }
        }

        if (parent->op.op == OP_ADD || parent->op.op == OP_MUL) {
            is_changed =  calculate_operator(parent_dp);
        }
    } else if (parent->type == AST_FUNC) {
        // calculate the function value
        AstNode* arg = parent->func.arg;
        if (is_node_num(arg)) {
            int func_value;
    
            switch (parent->func.func) {
            case FUNC_SIN:
                func_value = sin(arg->number);
                break;
            case FUNC_COS:
                func_value = cos(arg->number);
                break;
            case FUNC_TAN:
                func_value = tan(arg->number);
                break;
            case FUNC_EXP:
                func_value = exp(arg->number);
                break;
            case FUNC_LOG:
                func_value = log10(arg->number);
                break;
            case FUNC_LN:
                func_value = log(arg->number);
                break;
            case FUNC_INVALID:
                break;
            }

            destroy_ast_node(parent); // recursively
            *parent_dp = create_num_node(func_value);
            is_changed = true;
        }    
    }

    return is_changed;
}



// simplify expression in bottom-up
bool simplify_ast_node(AstNode** parent_dp) {
    AstNode* parent = *parent_dp;
    bool is_changed = false;


    if (parent->type == AST_OP) {
        Operator op = parent->op.op;

        AstNode* left = parent->op.left;
        AstNode* right = parent->op.right;

        // bottom-up
        if (simplify_ast_node(&parent->op.left)) is_changed = true;
        if (simplify_ast_node(&parent->op.right)) is_changed = true;


        if (calculate_constant(parent_dp)) is_changed = true;

        if (op == OP_ADD) {
            // 0+x -> x
            if (is_node_value_num(left, 0)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(left);
                *parent_dp = right;
                is_changed = true;
            } else if (is_node_value_num(right, 0)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                *parent_dp = left;
                is_changed = true;
            }
            
        } else if (op == OP_SUB) {
            if (is_node_value_num(left, 0)) {
                // 0 - x -> -x
                destroy_ast_node_only(parent);
                destroy_ast_node_only(left);
                *parent_dp = create_unary_node(UNARY_MINUS, right);
                is_changed = true;
            } else if (is_node_value_num(right, 0)) {
                // x - 0
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                *parent_dp = left;
                is_changed = true;
            }
        } else if (op == OP_MUL) {

            // 0 * x -> 0
            if (is_node_value_num(left, 0)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                *parent_dp = left;
                is_changed = true;
            } else if (is_node_value_num(right, 0)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(left);
                *parent_dp = right;
                is_changed = true;
            } else if (is_node_value_num(left, 1)) {
                // 1 * x -> x
                destroy_ast_node_only(parent);
                destroy_ast_node_only(left);
                *parent_dp = right;
                is_changed = true;
            } else if (is_node_value_num(right, 1)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                *parent_dp = left;
                is_changed = true;
            }
        } else if (op == OP_DIV) {
            // x / 1 -> x
            if (is_node_value_num(right, 1)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                *parent_dp = left;
                is_changed = true;
            }
        } else if (op == OP_POW) {
            // x ^ 1 -> x
            if (is_node_value_num(right, 1)) {
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                *parent_dp = left;
                is_changed = true;
            } else if (is_node_value_num(right, 0)) {
                // x ^ 0 -> 1 (including x = 0)
                destroy_ast_node_only(parent);
                destroy_ast_node_only(right);
                destroy_ast_node(left);
                *parent_dp = create_num_node(1);
                is_changed = true;
            } else if (is_node_value_num(left, 0)) {
                // 0 ^ x -> 0 (if x->type = num: x->number > 0)
                if (right->type != AST_OP || right->number > 0) {
                    destroy_ast_node_only(parent);
                    destroy_ast_node(left);
                    *parent_dp = right;
                    is_changed = true;
                } else if (right->number < 0) {
                    fprintf(stderr, "0 ^ (%lf) is invalid!\n", right->number);
                    return false;
                }
            } else if (is_node_value_num(left, 1)) {
                // 1 ^ x = 1
                destroy_ast_node(parent);
                destroy_ast_node(right);
                *parent_dp = left;
                is_changed = true;
            }
        }
    } else if (parent->type == AST_UNARY) {
        AstNode* operand = parent->unary.operand;

        if (simplify_ast_node(&parent->unary.operand)) is_changed = true;

        // if constasnt change to negative number
        if (is_node_num(operand)) {
            if (parent->unary.unary == UNARY_MINUS) {
                destroy_ast_node_only(parent);
                operand->number = -operand->number;
                *parent_dp = operand;
            } else {
                destroy_ast_node_only(parent);
                *parent_dp = operand;
            }

            is_changed = true;
        } else if (operand->type == AST_OP) {
            Operator operand_op = operand->op.op;
            AstNode* left = operand->op.left;
            AstNode* right = operand->op.right;

            if (operand_op == OP_ADD) {
                destroy_ast_node_only(parent);
                *parent_dp = operand;
                parent->op.op = OP_SUB;
               
                if (is_node_num(left)) {
                    left->number = -left->number;
                } else {
                    operand->op.left = create_unary_node(UNARY_MINUS, left);
                }
                
                is_changed = true;
            } else if (operand_op == OP_SUB) {
                destroy_ast_node_only(parent);
                *parent_dp = operand;
                parent->op.op = OP_ADD;

                if (is_node_num(left)) {
                    left->number = -left->number;
                } else {
                    operand->op.left = create_unary_node(UNARY_MINUS, left);
                }
                
                is_changed = true;
            } else if (operand_op == OP_MUL || operand_op == OP_DIV) {
                if (is_node_num(left)) {
                    destroy_ast_node_only(parent);
                    *parent_dp = operand;
                    left->number = -left->number;
                    is_changed = true;
                } else if (is_node_num(right)) {
                    destroy_ast_node_only(parent);
                    *parent_dp = operand;
                    right->number = -right->number;
                    is_changed = true;
                }
            }
        } else if (operand->type == AST_UNARY) {
            *parent_dp = operand->unary.operand;
            destroy_ast_node_only(parent);
            destroy_ast_node_only(operand);
            is_changed = true;
        }
    } else if (parent->type == AST_FUNC) {
        if (simplify_ast_node(&parent->func.arg)) is_changed = true;

        if (calculate_constant(parent_dp)) is_changed = true;
    }

    printf("\n");
    print_ast_node(*parent_dp, 0);

    return is_changed;
}



