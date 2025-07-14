#ifndef __FRACTION_H__
#define __FRACTION_H__

#include "ast.h"

typedef struct {
    int x;
    int y;
    bool is_valid;
} Fraction;


Fraction make_fraction(int x, int y);
Fraction create_fraction_from_ast_node(AstNode* node);

Fraction fraction_div(Fraction a, Fraction b);

#endif
