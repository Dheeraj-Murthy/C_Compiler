#ifndef LEXER_H_
#define LEXER_H_

#include <stdio.h>
typedef enum {
  BEGINNING,
  INT,
  KEYWORD,
  SEPARATOR,
  OPERATOR,
  IDENTIFIER,
  STRING,
  COMP,
  END_TOKEN,
} TokenType;

typedef struct {
  TokenType type;
  char *word;
  size_t line_num;
} Token;

void print_token(Token token);
Token *generate_number(char *current, int *current_index);
Token *generate_keyword_or_identifier(char *current, int *current_index);
Token *generate_separator_or_operator(char *current, int *current_index,
                                      TokenType type);
Token **lexer(FILE *file);

void free_tokens(Token **tokens);
#endif
