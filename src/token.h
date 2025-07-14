#ifndef __TOKEN_H__
#define __TOKEN_H__

// each token characters length should be under 20
#define MAX_TOKEN_LENGTH 20

typedef enum {
    TOKEN_NUM, TOKEN_VAR,
    TOKEN_ADD, TOKEN_SUB, TOKEN_MUL, TOKEN_DIV, TOKEN_POW,
    TOKEN_LPAREN, TOKEN_RPAREN, // parentheses
    TOKEN_FUNC, // sin, cos, tan, ln, log, exp
    TOKEN_END // end of expression
} TokenType;

typedef struct TokenNode {
    TokenType type;
    char* value;
    struct TokenNode* next;
    struct TokenNode* prev;
} TokenNode;

typedef struct {
    TokenNode* head;
    TokenNode* tail;
    int size;
} TokenList;

// TokenList iterator
typedef struct {
    TokenNode* current;
} TokenStream;


TokenNode* create_token_node(TokenType type, char* value, TokenNode* prev, TokenNode* next);
void destroy_token_node(TokenNode* token); // free the heap memory

TokenList* init_token_list();
void add_token_list(TokenList* list, TokenType type, char* value); // add a node next to the tail
void destroy_token_list(TokenList* list); // destroy and free the list


TokenStream* create_token_stream(TokenList* list);
 // return current TokenNode
TokenNode* peek_token_stream(TokenStream* s);
// return current TokenNode and move to the next
TokenNode* advance_token_stream(TokenStream* s);
// check if current type is expected
// if true: return current node and advance
// if false: return null and do nothing
TokenNode* match_token_stream(TokenStream* s, TokenType expected);
// CAUTION: destroy_token_stream doesn't destroy TokenNode* current !!!
// because the iterator itself doesn't have responsibility to its linked list in charge
void destroy_token_stream(TokenStream* s);

// tokenize the string
// return TokenList
// return NULL when fail to tokenize
TokenList* tokenize_string(char* str);

#endif
