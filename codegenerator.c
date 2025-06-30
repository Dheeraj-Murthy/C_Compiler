#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

void traverse_tree(Node* cur) {
    if (cur == NULL) {
        return;
    }
    printf("%s", cur->value);
    traverse_tree(cur->left);
    traverse_tree(cur->right);
}

void generate_code(Node* root) {
    print_tree(root, 0);

    FILE* assembly_file = fopen("assembly/generated.s", "w");
    if (assembly_file == NULL) {
        print_error("FILE COULD NOT BE OPENED");
    }

    fprintf(assembly_file, ".section __TEXT,__text\n");
    fprintf(assembly_file, ".global _start\n");
    fprintf(assembly_file, "_start:\n");
    fprintf(assembly_file, "\tmov X0, #1\n\tadrp X1, helloworld@PAGE\n\tadd X1, X1, "
                           "helloworld@PAGEOFF\n\tmov X2, #13\n\tmov X16, "
                           "#4\n\tsvc #0x80\n\n\tmov X0, #0\n\tmov X16, #1\n\tsvc #0x80\n");
    fprintf(assembly_file, ".data\n");
    fprintf(assembly_file, "helloworld: .ascii \"helloworld\"");

    fclose(assembly_file);

    system("assembly/build.sh");
}
