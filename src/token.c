#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "token.h"

// create token
TokenNode* create_token_node(TokenType type, char* value, TokenNode* prev, TokenNode* next) {
    TokenNode* token_node = (TokenNode*) malloc(sizeof(TokenNode));
    token_node->type = type;
    token_node->next = next;
    token_node->prev = prev;

    if (value == NULL)
        token_node->value = NULL;
    else
        token_node->value = strdup(value);

    return token_node;
}

// free the heap memory
void destroy_token_node(TokenNode *token) {
    if (token == NULL) return;

    if (token->value != NULL) free(token->value);
    free(token);
}

TokenList* init_token_list() {
    TokenList* list = (TokenList*) malloc(sizeof(TokenList));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}


void add_token_list(TokenList* list, TokenType type, char* value) {
    if (list->head == NULL) {
        list->head = create_token_node(type, value, NULL, NULL);
        list->tail = list->head;
        list->size = 1;
    } else {
        TokenNode* node = create_token_node(type, value, list->tail, NULL);
        list->tail->next = node;
        list->tail = node;
        list->size++;
    }
}

void destroy_token_list(TokenList* list) {
    TokenNode* curr = list->head;
    TokenNode* next;

    while (curr != NULL) {
        next = curr->next;
        destroy_token_node(curr);
        curr = next;
    }

    free(list);
}


TokenStream* create_token_stream(TokenList* list){
    TokenStream* stream = (TokenStream*) malloc(sizeof(TokenStream));
    stream->current = list->head;
    return stream;
}

TokenNode* peek_token_stream(TokenStream* s) {
    return s->current;
}

TokenNode* advance_token_stream(TokenStream* s) {
    if (s->current == NULL) return NULL;

    TokenNode* curr = s->current;
    s->current = curr->next;
    return curr;
}

TokenNode* match_token_stream(TokenStream* s, TokenType expected) {
    if (s->current && s->current->type == expected) {
        TokenNode* curr = s->current;
        advance_token_stream(s);
        return curr;
    } else {
        return NULL;
    }
}

void destroy_token_stream(TokenStream *s) {
    if (s != NULL) free(s);
}


// check if only space
static bool is_blank(char* str) {
    bool is_blank = true;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isspace((unsigned char)str[i])) {
            is_blank = 0;
            break;
        }
    }
    return is_blank;
}


TokenList* tokenize_string(char* str) {
    if (is_blank(str)) return NULL;

    enum ReadingState { R_NUM, R_ALPHA, R_OTHER };
    // read number until the char is not digit
    // read function or variable until the char is not alphabet
    // operators and brackets are one character so no need to distinguish state
    enum ReadingState prev_state;
    enum ReadingState current_state = R_OTHER;

    int i;
    char* begin; // memorize the beginning of numbers or alphabets
    int dot_count = 0; // counting dots to prevent wrong number e.g. '2.0.3'
    int str_len = strlen(str);

    TokenList* token_list = init_token_list();
    int tokens_index = 0;
    char token_value[MAX_TOKEN_LENGTH];

    // loop until \0 (including \0)
    // the reason including null char in loop is to create the token
    // from the last numbers or alphabets and create END token
    for (i = 0; i <= str_len; i++) {
        if (str[i] == ' ') continue; // ignore whitespaces

        prev_state = current_state;
        memset(token_value, 0, sizeof(token_value));

        // set current state
        if (isdigit(str[i])){
            current_state = R_NUM;
        } else if (str[i] == '.') { // '.' is for decimal point
            current_state = R_NUM;
            dot_count++;

            if (dot_count > 1)
                goto tokenize_error; // wrong numbers e.g. "2.0.3"

        } else if (isalpha(str[i])) {
            current_state = R_ALPHA;
        } else {
            current_state = R_OTHER;
        }

        if (prev_state == R_NUM && current_state != R_NUM) {
            // numbers ended
            strncpy(token_value, begin, str+i-begin);
            token_value[str+i-begin] = '\0'; // strncpy doesn't put \0 automatically
            add_token_list(token_list, TOKEN_NUM, token_value);

            dot_count = 0;
        } else if (prev_state == R_ALPHA && current_state != R_ALPHA) {
            // alphabets ended
            strncpy(token_value, begin, str+i - begin);
            token_value[str+i-begin] = '\0';

            if (strcmp(token_value, "x") == 0) {
                // x is the variable
                add_token_list(token_list, TOKEN_VAR, token_value);
            } else {
                // otherwise, it's a function
                add_token_list(token_list, TOKEN_FUNC, token_value);
            }
        }

        if (
            (prev_state != R_NUM && current_state == R_NUM) ||
            (prev_state != R_ALPHA && current_state == R_ALPHA)
        ) {
            // numbers or alphabet begin
            begin = str + i;
        }

        if (current_state == R_OTHER) {
            switch(str[i]) {
            case '+':
                add_token_list(token_list, TOKEN_ADD, "+");
                break;
            case '-':
                add_token_list(token_list, TOKEN_SUB, "-");
                break;
            case '*':
                add_token_list(token_list, TOKEN_MUL, "*");
                break;
            case '/':
                add_token_list(token_list, TOKEN_DIV, "/");
                break;
            case '^':
                add_token_list(token_list, TOKEN_POW, "^");
                break;
            case '(':
                add_token_list(token_list, TOKEN_LPAREN, "(");
                break;
            case ')':
                add_token_list(token_list, TOKEN_RPAREN, ")");
                break;
            case '\0':
                add_token_list(token_list, TOKEN_END, NULL);
                break;
            default: // unidentified char found, failed to tokenize
                goto tokenize_error;
            }
        }
    }

    return token_list;

tokenize_error:
    destroy_token_list(token_list);
    return NULL;
}
