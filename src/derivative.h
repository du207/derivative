#ifndef __DERIVATIVE_H__
#define __DERIVATIVE_H__

#include "ast.h"

/*
derivative rule:
(factor1 +/- factor2)' = factor1' +/- factor2'
(factor1 * factor2)' = factor1 * factor2' + factor1' * factor2
(factor1 / factor2)' = (factor1' * factor2 - factor1 * factor2')/(factor2)^2
(factor ^ num)' = num * factor ^ (num-1) * factor'
(factor1 ^ factor2)' = (exp(ln(factor1) * factor2))'
(func(expr))' = func'(expr) * expr'
*/

// 'AstNode* tree' must be the result of 'parse()' in parse.h
AstNode* derivative_expression(AstNode* tree);



/*
simplify rule:
x + 0 = x, x * 1 = x, x - 0 = x, x / 1 = x
x * 0 = 0, x ^ 1 = x, x ^ 0 = 1
2 + 3 = 5 (constant calculation)
ln(e^x) = x, log(10^x) = x
-(-x) = x, -0 = 0, -(x+2) = -x-2
*/
AstNode* simplify_expression(AstNode* tree);

#endif
