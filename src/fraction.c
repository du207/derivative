#include "fraction.h"

#include "ast.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

Fraction make_fraction(int x, int y) {
    bool is_valid = true;
    if (y == 0) {
        fprintf(stderr, "Denominator of a fraction cannot be 0\n");
        is_valid = false;
    }

    Fraction fr = (Fraction) {
        .x = x,
        .y = y,
        .is_valid = is_valid,
    };
    return fr;
}


static int double_to_int(double a) {
    if (a > INT_MAX) {
        fprintf(stderr, "Number over INT_MAX found! Calculation errors are expected");
        return INT_MAX;
    } else if (a < INT_MIN) {
        fprintf(stderr, "Number under INT_MIN found! Calculation errors are expected");
        return INT_MIN;
    }
    return (int) round(a);
}

// recursively check if the node only has num
static bool is_constant(AstNode* node) {
    if (node->type == AST_NUM) {
        return true;
    } else if (node->type == AST_OP) {
        return is_constant(node->op.left) && is_constant(node->op.right);
    } else if (node->type == AST_UNARY) {
        return is_constant(node->unary.operand);
    } else if (node->type == AST_FUNC) {
        return is_constant(node->func.arg);
    } else {
        return false;
    }
}

// node should be constant
Fraction create_fraction_from_ast_node(AstNode* node) {
    if (node->type == AST_NUM) {
        return make_fraction(node->number, 1);
    } else if (node->type == AST_OP && node->op.op == OP_DIV
        && is_constant(node->op.left) && is_constant(node->op.right)) {
            Fraction l = create_fraction_from_ast_node(node->op.left);
            Fraction r = create_fraction_from_ast_node(node->op.right);

            return fraction_div(l, r);
    } else if (node->type == AST_UNARY && is_constant(node->type)) {

    } else { // FUCKKKKKKKKKKKKKKKKK I'll just do FLoat calculation only giving up fraction
        fprintf(stderr, "This AstNode [%s] cannot be transformed to fraction\n", );
        return NULL;
    }

    return make_fraction(node->op.left->number, node->op.right->number);
}


Fraction fraction_div(Fraction a, Fraction b) {

}
