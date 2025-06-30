#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_token(Token* token) {
    const char* TokenTypeNames[] = {"INT", "KEYWORD", "SEPARATOR", "OPERATOR", "END_TOKEN", "BEGINNING"};
    printf(" Token type: %s || Token word: %s\n", TokenTypeNames[token->type], token->word);
}

Token* generate_number(const char** cursor_ptr) {
    const char* cursor = *cursor_ptr;
    Token* token = malloc(sizeof(Token));
    token->type = INT;

    char buffer[32];
    int idx = 0;

    while (isdigit(*cursor)) {
        buffer[idx++] = *cursor;
        cursor++;
    }

    buffer[idx] = '\0';

    token->word = malloc(idx + 1);
    strcpy(token->word, buffer);

    *cursor_ptr = cursor;
    return token;
}

Token* generate_keyword(const char** cursor_ptr) {
    const char* cursor = *cursor_ptr;
    Token* token = malloc(sizeof(Token));
    token->type = KEYWORD;

    char buffer[64];
    int idx = 0;

    while (isalpha(*cursor)) {
        buffer[idx++] = *cursor;
        cursor++;
    }

    buffer[idx] = '\0';

    token->word = malloc(idx + 1);
    strcpy(token->word, buffer);

    *cursor_ptr = cursor;
    return token;
}

Token* generate_separator_operator(const char** cursor_ptr, TokenType type) {
    const char* cursor = *cursor_ptr;
    Token* token = malloc(sizeof(Token));
    token->type = type;

    token->word = malloc(2);
    token->word[0] = *cursor;
    token->word[1] = '\0';

    *cursor_ptr = cursor + 1;
    return token;
}

Token** lexer(FILE* file) {
    size_t capacity = 16;
    size_t token_index = 0;
    Token** tokens = malloc(sizeof(Token*) * capacity);

    char line[1024];

    while (fgets(line, sizeof(line), file)) {
        const char* cursor = line;

        while (*cursor != '\0') {
            if (isspace(*cursor)) {
                cursor++;
                continue;
            }

            Token* token = NULL;

            if (*cursor == ';' || *cursor == '(' || *cursor == ')') {
                token = generate_separator_operator(&cursor, SEPARATOR);
            } else if(*cursor == '+') {
                token = generate_separator_operator(&cursor, OPERATOR);
            } else if (isdigit(*cursor)) {
                token = generate_number(&cursor);
            } else if (isalpha(*cursor)) {
                token = generate_keyword(&cursor);
            } else {
                // unknown character, skip it
                printf("Skipping unknown char: %c\n", *cursor);
                cursor++;
                continue;
            }

            tokens[token_index++] = token;

            if (token_index >= capacity) {
                capacity *= 2;
                tokens = realloc(tokens, sizeof(Token*) * capacity);
            }
        }
    }

    Token* end_token = malloc(sizeof(Token));
    end_token->type = END_TOKEN;
    end_token->word = NULL;
    tokens[token_index] = end_token;

    return tokens;
}

void free_tokens(Token** tokens) {
    int i;
    for (i = 0; tokens[i]->type != END_TOKEN; i++) {
        free(tokens[i]->word);
        free(tokens[i]);
    }
    free(tokens[i]); // END_TOKEN
    free(tokens);
}
