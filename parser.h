#ifndef PARSER_H // if PARSER_H is not defined -> #define PARSER_H

#include "lexer.h"

#define MAX_scope_STACK_LENGTH 64

typedef struct Node {
  char *value;
  TokenType type;
  struct Node *right;
  struct Node *left;
} Node;

typedef struct scope_stack {
  Node *content[MAX_scope_STACK_LENGTH];
  int top;
} scope_stack;

void print_tree(Node *current, int depth);
Node *create_Node(char *val, TokenType type);
void print_error(char *msg, size_t line_number);
void expect(Token **tokens, int i, TokenType type, const char *word);
Node *parse_exit(Token **tokens, int *i, Node *current);
Node *parser(Token **tokens);
Node *create_Node(char *val, TokenType type);

#endif // skip if defined
