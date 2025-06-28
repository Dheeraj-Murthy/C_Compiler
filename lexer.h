#ifndef LEXER_H
#define LEXER_H
#include <stdio.h>

typedef enum {
  INT,
  KEYWORD,
  SEPARATOR,
  END_TOKEN,
} TokenType;

typedef struct {
  TokenType type;
  char *word;
} Token;

void print_token(Token *token);
Token *generate_number(FILE *file);
Token *generate_keyword(FILE *file);
Token *generate_separator(FILE *file);
Token **lexer(FILE *file);
void free_tokens(Token **tokens);

#endif // LEXER_H
