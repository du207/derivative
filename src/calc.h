#ifndef __CALC_H__
#define __CALC_H__


#include <stdbool.h>
#include "ast.h"

/*
simplify rule:
x + 0 = x, x * 1 = x, x - 0 = x, 0 - x = -x, x / 1 = x
x * 0 = 0, x ^ 1 = x, x ^ 0 = 1
0 ^ x = 0 (x>0), 0 ^ 0 = 1, 0 ^ x (x<0 num) = error, 1 ^ x = 1
x^(-n) = 1 / x^n
2 + 3 = 5 (constant calculation)
ln(e^x) = x, log(10^x) = x
-(-x) = x, -0 = 0, -(x+2) = -x-2
2x*(x+3) = 2x*x + 3 * 2x (distributive law)

-> simplify mutliple times until doesn't change


- Constant Calculation rule (when ONLY integers)
1. calculate multiply, sum, sub
2. the division will be expressed as fraction
3. fractions' multiply and division will be calculated by abbreviation
4. fractions' add and sub will be calculated by a common denominator

+ function calculation:
sin, cos, tan: only for common angles (pi/6, pi/3, pi/2, ...)
ln, log: only when the function value is rational number
sqrt: square number



- IF even SINGLE non-integer constant is included,
it just do whole double calculation!!!
*/




// CON_FR: use fraction (every constants should be integer)
// CON_DB: use double (when even single non-integer contant included)
typedef enum { CON_FR, CON_DB } ConstantMode;


bool simplify_ast_node(AstNode** tree, ConstantMode cm);







#endif
