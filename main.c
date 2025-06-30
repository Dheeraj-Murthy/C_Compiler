#include "codegenerator.h"
#include "lexer.h"
#include "parser.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Incorrect syntax\nCorrect syntax: %s <filename.bling>\n", argv[0]);
        exit(1);
    }
    FILE* file;
    file = fopen(argv[1], "r");

    if (!file) {
        printf("Error: File not found\n");
        exit(1);
    }

    Token** tokens = lexer(file);
    Node* root = parser(tokens);
    generate_code(root);

    free_tokens(tokens);
}
