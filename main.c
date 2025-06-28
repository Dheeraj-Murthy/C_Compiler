#include "lexer.h"

int main()
{
    FILE* file;
    file = fopen("test.bling", "r");
    Token** tokens = lexer(file);
    for (int i = 0; i < 12 && tokens[i]->type != END_TOKEN; i++)
    {
        print_token(tokens[i]);
    }

    free_tokens(tokens);
}
