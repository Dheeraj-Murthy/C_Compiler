#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void print_token(Token* token)
{
    const char* TokenTypeNames[] = {"INT", "KEYWORD", "SEPARATOR", "END_TOKEN"};
    printf(" Token type: %s || Token word: %s\n", TokenTypeNames[token->type], token->word);
}

Token* generate_number(FILE* file)
{
    Token* token = malloc(sizeof(Token));
    token->type = INT;
    char current;
    int idx = 0;
    char* nums = malloc(sizeof(char) * (10));

    // read in the literal
    while ((current = fgetc(file)) != EOF && isdigit(current))
    {
        nums[idx] = current;
        idx++;
    }
    nums[idx] = '\0';
    ungetc(current, file);

    // assign the value
    token->word = malloc((idx + 1) * sizeof(char));
    strcpy(token->word, nums);
    free(nums);
    return token;
}

Token* generate_keyword(FILE* file)
{
    Token* token = malloc(sizeof(Token));
    token->type = KEYWORD;
    char* word;
    word = malloc(sizeof(char) * 10);
    int index = 0;
    char current;

    // recover the keyword
    while ((current = fgetc(file)) != EOF && isalpha(current))
    {
        word[index] = current;
        index++;
    }
    ungetc(current, file);

    // malloc token and copy the word into it
    token->word = malloc(sizeof(char) * (index + 1));
    strcpy(token->word, word);
    free(word);
    token->word[index] = '\0';
    // set the type of token

    if (strcmp(token->word, "exit") == 0)
    {
        // token->type = KEYWORD;
        //
    }
    return token;
}

Token* generate_separator(FILE* file)
{
    Token* token = malloc(sizeof(Token));
    token->type = SEPARATOR;
    char temp = fgetc(file);
    token->word = malloc(sizeof(char) * 2);
    token->word[0] = temp;
    token->word[1] = '\0';
    return token;
}

Token** lexer(FILE* file)
{
    char current;
    size_t capacity = 12;
    Token** tokens = malloc(sizeof(Token*) * capacity);
    size_t token_index = 0;

    while ((current = fgetc(file)) != EOF)
    {
        Token* test_token = NULL;
        if (current == ';')
        {
            ungetc(current, file);
            test_token = generate_separator(file);
        }
        else if (current == '(')
        {
            ungetc(current, file);
            test_token = generate_separator(file);
        }
        else if (current == ')')
        {
            ungetc(current, file);
            test_token = generate_separator(file);
        }
        else if (isdigit(current))
        {
            ungetc(current, file);
            test_token = generate_number(file);
        }
        else if (isalpha(current))
        {
            ungetc(current, file);
            test_token = generate_keyword(file);
        }
        else
        {
            printf("the problem char: %c", current);
            continue;
        }
        tokens[token_index++] = test_token;
        // free(test_token);
    }
    Token* end_token = malloc(sizeof(Token));
    end_token->type = END_TOKEN;
    tokens[token_index] = end_token;
    return tokens;
}

void free_tokens(Token** tokens)
{
    int i;
    for (i = 0; tokens[i]->type != END_TOKEN; i++)
    {
        free(tokens[i]->word);
        free(tokens[i]);
    }
    free(tokens[i]); // END_TOKEN
    free(tokens);
}
