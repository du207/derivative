#include "parse.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "ast.h"

static AstNode* parse_expression(TokenStream* s);
static AstNode* parse_term(TokenStream* s);
static AstNode* parse_factor(TokenStream* s);
static AstNode* parse_power(TokenStream* s);
static AstNode* parse_primary(TokenStream* s);


AstNode* parse(char* str) {
    TokenList* token_list = tokenize_string(str);
    if (token_list == NULL) {
        fprintf(stderr, "Tokenizing error!\n");
        return NULL;
    }

    TokenStream* s = create_token_stream(token_list);
    AstNode* ast_tree = parse_expression(s);
    if (ast_tree == NULL) {
        fprintf(stderr, "Parsing error!\n");
        destroy_token_list(token_list);
        destroy_token_stream(s);
        return NULL;
    }

    destroy_token_list(token_list);
    destroy_token_stream(s);

    return ast_tree;
}

// recursive is kinda magic
static AstNode* parse_expression (TokenStream* s) {
    AstNode* node = parse_term(s);
    // current = + or - or END

    if (node == NULL) return NULL;

    TokenNode* op;

    while ((op = match_token_stream(s, TOKEN_ADD)) || (op = match_token_stream(s, TOKEN_SUB))) {
        // current = term

        AstNode* next = parse_term(s);
        if (next == NULL) {
            destroy_ast_node(node);
            return NULL;
        }

        Operator operator = op->type == TOKEN_ADD ? OP_ADD : OP_SUB;

        node = create_op_node(operator, node, next);
    }

    return node;
}


static bool check_implicit_mul(TokenStream* s) {
    /*
    _IMPLICIT_MUL_: like 2x, 3(x+1), (x+1)3, 2sin(x)
    NUM VAR
    NUM FUNC
    NUM LPAREN
    VAR LPAREN
    VAR FUNC
    RPAREN LPAREN
    RPAREN NUM
    RPAREN VAR
    RPAREN FUNC
    */

    TokenNode* curr = peek_token_stream(s);
    TokenType prev_t = curr->prev->type;
    TokenType curr_t = curr->type;

    return (prev_t == TOKEN_NUM && curr_t == TOKEN_VAR)
        || (prev_t == TOKEN_NUM && curr_t == TOKEN_FUNC)
        || (prev_t == TOKEN_NUM && curr_t == TOKEN_LPAREN)
        || (prev_t == TOKEN_VAR && curr_t == TOKEN_LPAREN)
        || (prev_t == TOKEN_VAR && curr_t == TOKEN_FUNC)
        || (prev_t == TOKEN_RPAREN && curr_t == TOKEN_LPAREN)
        || (prev_t == TOKEN_RPAREN && curr_t == TOKEN_NUM)
        || (prev_t == TOKEN_RPAREN && curr_t == TOKEN_VAR)
        || (prev_t == TOKEN_RPAREN && curr_t == TOKEN_FUNC);
}

static AstNode* parse_term(TokenStream* s) {
    if (peek_token_stream(s) == NULL) return NULL;

    AstNode* node = parse_factor(s);
    if (node == NULL) return NULL;

    // current = * or / or else

    TokenNode* op = NULL;

    while (1) {
        Operator op;

        if (match_token_stream(s, TOKEN_MUL) || check_implicit_mul(s)) {
            op = OP_MUL;
        } else if (match_token_stream(s, TOKEN_DIV)) {
            op = OP_DIV;
        } else {
            break;
        }
        // current = factor

        AstNode* next = parse_factor(s);
        if (next == NULL) {
            destroy_ast_node(node);
            return NULL;
        }

        node = create_op_node(op, node, next);
    }

    return node;
}


static AstNode* parse_factor(TokenStream* s) {
    TokenNode* unary;
    AstNode* node;

    if ((unary = match_token_stream(s, TOKEN_ADD)) || (unary = match_token_stream(s, TOKEN_SUB))) {
        Unary unary_type = unary->type == TOKEN_ADD ? UNARY_PLUS : UNARY_MINUS;
        AstNode* next = parse_factor(s);
        if (next == NULL) return NULL;

        node = create_unary_node(unary_type, next);
    } else {
        node = parse_power(s);
    }

    return node;
}

static AstNode* parse_power(TokenStream* s) {
    AstNode* node = parse_primary(s);
    if (node == NULL) return NULL;

    if (match_token_stream(s, TOKEN_POW)) {
        AstNode* next = parse_power(s);
        if (next == NULL) {
            destroy_ast_node(node);
            return NULL;
        }

        node = create_op_node(OP_POW, node, next);
    }

    return node;
}

static Function get_function(char* str) {
    // so sad cuz no switch-case for string
    if (strcmp(str, "sin") == 0) {
        return FUNC_SIN;
    } else if (strcmp(str, "cos") == 0) {
        return FUNC_COS;
    } else if (strcmp(str, "tan") == 0) {
        return FUNC_TAN;
    } else if (strcmp(str, "ln") == 0) {
        return FUNC_LN;
    } else if (strcmp(str, "log") == 0) {
        return FUNC_LOG;
    } else if (strcmp(str, "exp") == 0) {
        return FUNC_EXP;
    } else {
        return FUNC_INVALID;
    }
}

static AstNode* parse_primary(TokenStream* s) {
    TokenNode* curr = advance_token_stream(s);
    AstNode* node;

    if (curr->type == TOKEN_NUM) {
        node = create_num_node(atof(curr->value));
    } else if (curr->type == TOKEN_VAR) {
        node = create_var_node();
    } else if (curr->type == TOKEN_FUNC) {
        Function func_name = get_function(curr->value);

        if (func_name == FUNC_INVALID) {
            fprintf(stderr, "Invalid function '%s'\n", curr->value);
            return NULL;
        }

        // s->current should be lparen
        if (!(match_token_stream(s, TOKEN_LPAREN))) {
            fprintf(stderr, "Left parenthese expected!\n");
            return NULL;
        }
        // s->current = expression (after lparen)

        AstNode* expr = parse_expression(s);
        if (expr == NULL) return NULL;
        // s->current should be rparen
        if (!(match_token_stream(s, TOKEN_RPAREN))) {
            fprintf(stderr, "Right parenthese expected!\n");
            destroy_ast_node(expr);
            return NULL;
        }

        node = create_func_node(func_name, expr);
    } else if (curr->type == TOKEN_LPAREN) {
        // s->current = expression (after lparen)
        AstNode* expr = parse_expression(s);
        // s_.current should be rparen
        if (!(match_token_stream(s, TOKEN_RPAREN))) {
            fprintf(stderr, "Right parenthese expected!\n");
            destroy_ast_node(expr);
            return NULL;
        }

        node = expr;
    } else {
        fprintf(stderr, "Invalid token at '%s'\n", curr->value);
        return NULL;
    }

    return node;
}
