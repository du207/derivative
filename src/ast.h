#ifndef __AST_H__
#define __AST_H__


typedef enum {
    AST_NUM, AST_VAR, AST_OP, AST_FUNC, AST_UNARY
} AstType;


typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_POW
} Operator;

typedef enum {
    FUNC_SIN, FUNC_COS, FUNC_TAN,
    FUNC_LN, FUNC_LOG, FUNC_EXP,
    FUNC_INVALID // invalid function, parsing error
} Function;

typedef enum {
    UNARY_PLUS, UNARY_MINUS
} Unary;

typedef struct AstNode {
    AstType type;
    union {
        // AST_NUM
        double number;

        // AST_OP
        struct {
            Operator op;
            struct AstNode* left;
            struct AstNode* right;
        } op;

        // AST_FUNC
        struct {
            Function func;
            struct AstNode* arg;
        } func;

        // AST_UNARY
        struct {
            Unary unary;
            struct AstNode* operand;
        } unary;
    };
} AstNode;


AstNode* create_num_node(double num);
AstNode* create_var_node();
AstNode* create_op_node(Operator op, AstNode* left, AstNode* right);
AstNode* create_func_node(Function func, AstNode* arg);
AstNode* create_unary_node(Unary unary, AstNode* operand);

// clone ast node recursively (deep clone)
AstNode* clone_ast_node(AstNode* node);

void destroy_ast_node(AstNode* node);

// Print ast tree nodes (just for test and debug)
void print_ast_node(AstNode* node, int indent);

#endif
