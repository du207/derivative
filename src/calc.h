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
2-(x+2) = 2 - x - 2

-> simplify mutliple times until doesn't change

*/





bool simplify_ast_node(AstNode** tree);







#endif
