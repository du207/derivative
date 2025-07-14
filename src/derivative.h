#ifndef __DERIVATIVE_H__
#define __DERIVATIVE_H__

#include "ast.h"
#include <stdbool.h>

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


#endif
