#include "derivative.h"

#include "ast.h"
#include <stdlib.h>
#include <stdbool.h>



// DONT FORGET!!: If building a new ast tree, it should ALWAYS clone the ast node
//
// 'derivative_expression' clone the 'AstNode* tree'
// so it doesn't have responsibility to destroy 'AstNode* tree'
// and caller should destroy 'AstNode* tree'
AstNode* derivative_expression(AstNode* tree) {
    if (tree == NULL) return NULL;

    AstNode* node;

    if (tree->type == AST_OP) {
        Operator op = tree->op.op;

        if (op == OP_ADD || op == OP_SUB) {
            // (factor1 +/- factor2)' = factor1' +/- factor2'
            AstNode* left_d = derivative_expression(tree->op.left);
            AstNode* right_d = derivative_expression(tree->op.right);
            node = create_op_node(op, left_d, right_d);
        } else if (op == OP_MUL) {
            // (factor1 * factor2)' = factor1 * factor2' + factor1' * factor2
            AstNode* left = clone_ast_node(tree->op.left);
            AstNode* right = clone_ast_node(tree->op.right);
            AstNode* left_d = derivative_expression(left);
            AstNode* right_d = derivative_expression(right);

            node = create_op_node(OP_ADD,
                create_op_node(OP_MUL, left, right_d),
                create_op_node(OP_MUL, left_d, right)
            );
        } else if (op == OP_DIV) {
            // (factor1 / factor2)' = (factor1' * factor2 - factor1 * factor2')/(factor2)^2
            AstNode* left = clone_ast_node(tree->op.left);
            AstNode* right = clone_ast_node(tree->op.right);
            AstNode* left_d = derivative_expression(left);
            AstNode* right_d = derivative_expression(right);

            // clone for the denominator part
            // should not use a same node twice in ast tree
            AstNode* right2 = clone_ast_node(tree->op.right);

            node = create_op_node(OP_DIV,
                create_op_node(OP_SUB,
                    create_op_node(OP_MUL, left_d, right),
                    create_op_node(OP_MUL, left, right_d)
                ),
                create_op_node(OP_POW, right2, create_num_node(2))
            );
        } else if (op == OP_POW) {
            AstNode* left = clone_ast_node(tree->op.left);
            AstNode* right = clone_ast_node(tree->op.right);

            if (right->type == AST_NUM) {
                // (factor ^ num)' = num * factor ^ (num-1) * factor'
                AstNode* left_d = derivative_expression(left);

                node = create_op_node(OP_MUL,
                    create_op_node(OP_MUL,
                        right,
                        create_op_node(OP_POW, left, create_num_node(right->number-1))
                    ),
                    left_d
                );
            } else if (right->type == AST_UNARY) {
                AstNode* left_d = derivative_expression(left);

                if (right->unary.unary == UNARY_PLUS) {
                    node = derivative_expression(right->unary.operand);
                } else { // minus
                    // (f^(-a))' = (-a) * f * (-(a+1)) * f'
                    node = create_op_node(OP_MUL,
                        create_op_node(OP_MUL,
                            right,
                            create_op_node(OP_POW, left,
                                create_unary_node(UNARY_MINUS, create_num_node(right->unary.operand->number+1))
                            )
                        ),
                        left_d
                    );
                }
            } else {
                // (factor1 ^ factor2)' = (exp(ln(factor1) * factor2))'

                // these are temporary nodes! they should be destroyed in this block
                AstNode* ln_left = create_func_node(FUNC_LN, left);
                AstNode* arg = create_op_node(OP_MUL, ln_left, right);
                AstNode* exp = create_func_node(FUNC_EXP, arg);

                node = derivative_expression(exp);

                destroy_ast_node(exp); // temporary node 'exp' becomes useless
                // also recursively destroy arg and ln_left
            }
        }
    } else if (tree->type == AST_NUM) {
        node = create_num_node(0);
    } else if (tree->type == AST_VAR) {
        node = create_num_node(1);
    } else if (tree->type == AST_UNARY) {
        node = create_unary_node(tree->unary.unary, derivative_expression(tree->unary.operand));
    } else if (tree->type == AST_FUNC) {
        // (func(expr))' = func'(expr) * expr'
        AstNode* arg = clone_ast_node(tree->func.arg);
        AstNode* arg_d = derivative_expression(arg);

        Function func = tree->func.func;

        if (func == FUNC_SIN) {
            // sin' = cos
            node = create_op_node(OP_MUL,
                create_func_node(FUNC_COS, arg),
                arg_d
            );
        } else if (func == FUNC_COS) {
            // cos' = -sin
            node = create_op_node(OP_MUL,
                create_unary_node(UNARY_MINUS,
                    create_func_node(FUNC_SIN, arg)
                ),
                arg_d
            );
        } else if (func == FUNC_TAN) {
            // tan' = 1/cos^2
            node = create_op_node(OP_MUL,
                create_op_node(OP_DIV, create_num_node(1),
                    create_op_node(OP_POW,
                        create_func_node(FUNC_COS, arg),
                        create_num_node(2)
                    )
                ),
                arg_d
            );
        } else if (func == FUNC_LN) {
            // ln' = 1/()
            node = create_op_node(OP_DIV, arg_d, arg);
        } else if (func == FUNC_LOG) {
            // log' = 1/(ln(10)*())
            node = create_op_node(OP_DIV, arg_d,
                create_op_node(OP_MUL,
                    create_func_node(FUNC_LN, create_num_node(10)),
                    arg
                )
            );
        } else if (func == FUNC_EXP) {
            // exp' = exp
            node = create_op_node(OP_MUL,
                create_func_node(FUNC_EXP, arg),
                arg_d
            );
        } else { // else should never happen (FUNC_INVALID should not be parsed)
            node = NULL;
        }
    }

    return node;
}

