#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t line_number = 0;

void print_token(Token token) {
    const char* TokenTypeNames[] = {
        "BEGINNING",  // 0
        "INT",        // 1
        "KEYWORD",    // 2
        "SEPARATOR",  // 3
        "OPERATOR",   // 4
        "IDENTIFIER", // 5
        "STRING",     // 6
        "COMP",       // 7
        "END_TOKEN"   // 8
    };
    printf(" Token type: %s || Token word: %s\n", TokenTypeNames[token.type], token.word);
}

Token* generate_number(char* current, int* current_index) {
    Token* token = malloc(sizeof(Token));
    token->type = INT;
    token->line_num = line_number;
    char* word = malloc(sizeof(char) * 8);
    int word_index = 0;
    while (isdigit(current[*current_index]) && current[*current_index] != '\0') {
        // if (!isdigit(current[*current_index])) {
        //     break;
        // }
        word[word_index] = current[*current_index];
        word_index++;
        *current_index += 1;
    }
    word[word_index] = '\0';
    token->word = word;
    return token;
}

Token* generate_keyword_or_identifier(char* current, int* current_index) {
    Token* token = malloc(sizeof(Token));
    char* keyword = malloc(sizeof(char) * 10);
    token->line_num = line_number;
    int keyword_index = 0;
    while (isalpha(current[*current_index]) && current[*current_index] != '\0') {
        keyword[keyword_index] = current[*current_index];
        keyword_index++;
        *current_index += 1;
    }
    keyword[keyword_index] = '\0';
    token->word = malloc(strlen(keyword) + 1);
    strcpy(token->word, keyword);
    free(keyword);
    if (strcmp(token->word, "exit") == 0) {
        token->type = KEYWORD;
    } else if (strcmp(token->word, "int") == 0) {
        token->type = KEYWORD;
    } else if (strcmp(token->word, "if") == 0) {
        token->type = KEYWORD;
    } else if (strcmp(token->word, "while") == 0) {
        token->type = KEYWORD;
    } else if (strcmp(token->word, "write") == 0) {
        token->type = KEYWORD;
    } else if (strcmp(token->word, "eq") == 0) {
        token->type = COMP;
    } else if (strcmp(token->word, "neq") == 0) {
        token->type = COMP;
    } else if (strcmp(token->word, "less") == 0) {
        token->type = COMP;
    } else if (strcmp(token->word, "greater") == 0) {
        token->type = COMP;
    } else {
        token->type = IDENTIFIER;
    }
    return token;
}

Token* generate_string_token(char* current, int* current_index) {
    Token* token = malloc(sizeof(Token));
    token->line_num = line_number;
    char* value = malloc(sizeof(char) * 64);
    int value_index = 0;
    (*current_index)++;
    while (current[*current_index] != '"') {
        value[value_index] = current[*current_index];
        value_index++;
        (*current_index)++;
    }
    value[value_index] = '\0';
    token->type = STRING;
    token->word = malloc(strlen(value) + 1);
    strcpy(token->word, value);
    return token;
}

Token* generate_separator_or_operator(char* current, int* current_index, TokenType type) {
    Token* token = malloc(sizeof(Token));
    token->word = malloc(sizeof(char) * 2);
    token->word[0] = current[*current_index];
    token->word[1] = '\0';
    token->type = type;
    token->line_num = line_number;
    (*current_index)++;
    return token;
}

Token** lexer(FILE* file) {
    int length;
    char* current = 0;
    size_t tokens_index;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    current = malloc(sizeof(char) * (length + 1));
    fread(current, 1, length, file);

    fclose(file);

    current[length] = '\0';
    int current_index = 0;

    int number_of_tokens = 50;
    int tokens_size = 0;
    Token** tokens = malloc(sizeof(Token*) * number_of_tokens);
    tokens_index = 0;

    while (current[current_index] != '\0') {
        Token* token = NULL;
        tokens_size++;
        if (tokens_size > number_of_tokens) {
            number_of_tokens *= 2;
            tokens = realloc(tokens, sizeof(Token*) * number_of_tokens);
        }
        if (current[current_index] == '(' || current[current_index] == ')' ||
            current[current_index] == ';' || current[current_index] == ',' ||
            current[current_index] == '{' || current[current_index] == '}') {
            token = generate_separator_or_operator(current, &current_index, SEPARATOR);
            tokens[tokens_index] = token;
        } else if (current[current_index] == '+' || current[current_index] == '-' ||
                   current[current_index] == '*' || current[current_index] == '/' ||
                   current[current_index] == '=' || current[current_index] == '%') {
            token = generate_separator_or_operator(current, &current_index, OPERATOR);
            tokens[tokens_index] = token;
        } else if (current[current_index] == '"') {
            token = generate_string_token(current, &current_index);
            tokens[tokens_index] = token;
        } else if (current[current_index] == ' ') {
            current_index++;
            continue;
        } else if (isdigit(current[current_index])) {
            token = generate_number(current, &current_index);
            tokens[tokens_index] = token;
        } else if (isalpha(current[current_index])) {
            token = generate_keyword_or_identifier(current, &current_index);
            tokens[tokens_index] = token;
        } else if (current[current_index] == '\n') {
            line_number += 1;
            current_index++;
            continue;
        } else {
            current_index++;
            continue;
        }
        // if (token != NULL)
        //     print_token(*token);
        tokens_index++;
    }
    // After the loop, ensure there's space for the END_TOKEN
    if (tokens_size >= number_of_tokens) {
        number_of_tokens++; // Just need one more slot
        tokens = realloc(tokens, sizeof(Token*) * number_of_tokens);
        if (tokens == NULL) {
            perror("realloc failed for END_TOKEN slot");
            exit(EXIT_FAILURE);
        }
    }

    tokens[tokens_index] = malloc(sizeof(Token));
    tokens[tokens_index]->word = malloc(2);
    // strcpy(tokens[tokens_index]->word, "\0");
    tokens[tokens_index]->word[0] = '\0';
    tokens[tokens_index]->type = END_TOKEN;
    // print_token(*tokens[tokens_index]);

    //

    // for (int i = 0; tokens[i]->type != END_TOKEN; i++) {
    //     print_token(*tokens[i]);
    // }

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
