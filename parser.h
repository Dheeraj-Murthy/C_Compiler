#ifndef PARSER_H // if PARSER_H is not defined -> #define PARSER_H

#include "lexer.h"

typedef struct Node {
  char *value;
  TokenType type;
  struct Node *right;
  struct Node *left;
} Node;

void print_tree(Node *current, int depth);
Node *create_Node(char *val, TokenType type);
void print_error(char *msg);
void expect(Token **tokens, int *i, TokenType type, const char *word);
Node *parse_exit(Token **tokens, int *i);
Node *parser(Token **tokens);

#endif // skip if defined
