#include "parser.h"
#include "lexer.h"

#include <stdlib.h>
#include <string.h>

void print_tree(Node* current, int depth) {
    if (!current)
        return;
    const char* TokenTypeNames[] = {"INT", "KEYWORD", "SEPARATOR", "END_TOKEN", "BEGINNING"};
    for (int i = 0; i < depth; i++)
        printf("  ");
    printf("Value: %s || Type: %s\n", current->value, TokenTypeNames[current->type]);
    if (current->left) {
        for (int i = 0; i < depth + 1; i++)
            printf("  ");
        printf("left:");
        print_tree(current->left, depth + 2);
    }
    if (current->right) {
        for (int i = 0; i < depth + 1; i++)
            printf("  ");
        printf("right:");
        print_tree(current->right, depth + 2);
    }
}

Node* create_Node(char* val, TokenType type) {
    if (val == NULL) {
        val = "NO_VALUE";
    }
    Node* node = malloc(sizeof(Node));
    node->value = malloc(strlen(val) + 1);
    node->type = type;
    strcpy(node->value, val);
    node->left = NULL;
    node->right = NULL;
    return node;
}

void print_error(char* msg) {
    printf("ERROR: %s\n", msg);
    exit(1);
}

void expect(Token** tokens, int* i, TokenType type, const char* word) {
    if (tokens[*i]->type != type || (word && strcmp(tokens[*i]->word, word) != 0)) {
        printf("Syntax error: expected '%s' at token %d, found '%s'\n", word, *i, tokens[*i]->word);
        exit(1);
    }
}

Node* parse_exit(Token** tokens, int* i) {
    Node* exit_node = create_Node(tokens[*i]->word, KEYWORD);
    (*i)++;

    // Expect (
    expect(tokens, i, SEPARATOR, "(");
    Node* open_paren = create_Node(tokens[*i]->word, SEPARATOR);
    exit_node->left = open_paren;
    (*i)++;

    // Expect literal
    if (tokens[*i]->type != INT)
        print_error("Expected integer literal inside exit()");
    Node* expr_node = create_Node(tokens[*i]->word, INT);
    open_paren->left = expr_node;
    (*i)++;

    // Expect )
    expect(tokens, i, SEPARATOR, ")");
    Node* close_paren = create_Node(tokens[*i]->word, SEPARATOR);
    open_paren->right = close_paren;
    (*i)++;

    // Expect ;
    expect(tokens, i, SEPARATOR, ";");
    Node* semi = create_Node(tokens[*i]->word, SEPARATOR);
    exit_node->right = semi;
    (*i)++;

    return exit_node;
}

Node* parser(Token** tokens) {
    Node* root = create_Node("PROGRAM", BEGINNING);
    Node* current_node = root;

    int i = 0;
    while (tokens[i]->type != END_TOKEN) {
        if (current_node == root) {
            // printf("start");
        } else if (current_node == NULL) {
            break;
        }

        switch (tokens[i]->type) {
            case INT:
                break;
            case KEYWORD:
                if (!strcmp(tokens[i]->word, "exit")) {
                    root->right = parse_exit(tokens, &i);
                } else {
                    i++;
                }
                break;
            case SEPARATOR:
                break;
            case END_TOKEN:
                break;
            case BEGINNING:
                break;
        }
        // i++;
    }
    // print_tree(root, 0);
    return root;
}
