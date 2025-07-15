#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "derivative.h"
#include "parse.h"
#include "ast.h"
#include "calc.h"


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
        printf("Parsing failed, abort!\n");
        return 1;
    }
    printf("\n\n");

//    while (simplify_ast_node(&ast_tree));
    char* inflix = ast_to_infix(ast_tree);
    printf("%s\n", inflix);

    free(inflix);

    AstNode* derv_tree = derivative_expression(ast_tree);
    if (derv_tree == NULL) {
        printf("Derivative error!\n");
        return 1;
    }
//    print_ast_node(derv_tree,0);
    //while (simplify_ast_node(&derv_tree));

    char* derv_inflix = ast_to_infix(derv_tree);
    printf("%s\n", derv_inflix);

    free(derv_inflix);

    destroy_ast_node(ast_tree);
    destroy_ast_node(derv_tree);

    return 0;
}
