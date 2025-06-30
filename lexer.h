#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

typedef enum {
    INT,
    KEYWORD,
    SEPARATOR,
    OPERATOR,
    END_TOKEN,
    BEGINNING,
} TokenType;

typedef struct {
    TokenType type;
    char *word;
} Token;

void print_token(Token *token);
Token *generate_number(const char **cursor);
Token *generate_keyword(const char **cursor);
Token *generate_separator_operator(const char **cursor, TokenType type);
Token **lexer(FILE *file);
void free_tokens(Token **tokens);

#endif // LEXER_H
