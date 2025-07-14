#ifndef __PARSE_H__
#define __PARSE_H__

#include "token.h"
#include "ast.h"

/*
Declare parsing rule by 'EBNF':

expression   → term (("+" | "-") term)*
term         → factor (("*" | "/" | _IMPLICIT_MUL_) factor)*
factor        → ("+" | "-") factor | power
power        → primary ("^" power)?
primary      → NUMBER | VARIABLE | FUNCTION "(" expression ")" | "(" expression ")"

_IMPLICIT_MUL_: like 2x, 3(x+1), (x+1)3, 2sin(x)
specifically,
NUM VAR
NUM FUNC
NUM LPAREN
VAR LPAREN
RPAREN LPAREN
RPAREN NUM
RPAREN VAR
RPAREN FUNC

VAR FUNC is not allowed like x sin(x) because no way to distinguish x and sin(x)
(whitespaces are ignored in tokenizer)
*/

// tokenize + parse from string
// if parsing error, return NULL
AstNode* parse(char* str);

#endif
