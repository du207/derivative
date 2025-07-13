#include <stdio.h>
#include "ast.h"
#include "derivative.h"
#include "parse.h"


int main (int argc, char **argv) {
    printf("***Enter the function***\n");
    printf("f(x) = ");

    char user_input[200];

    if (scanf("%[^\n]", user_input) == 0) {
        printf("No input!\n");
        return 1;
    }
    getchar(); // prevent \n for next scanf

    AstNode* ast_tree = parse(user_input);
    if (ast_tree == NULL) {
        printf("Parsing error!\n");
        return 1;
    }

    print_ast_node(ast_tree, 0);
    printf("\n\n");

    AstNode* derv_tree = derivative_expression(ast_tree);
    if (derv_tree == NULL) {
        printf("Derivative error!\n");
        return 1;
    }
    print_ast_node(derv_tree, 0);

    destroy_ast_node(ast_tree);
    destroy_ast_node(derv_tree);

    return 0;
}
