#include "utils/hashmap.h"
#include "utils/hashmapoperators.h"
#include "codegenerator.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_SIZE 1024

char* scope_stak[MAX_STACK_SIZE];
size_t scope_stak_size = 0;
int scope_count = 0;
int global_scope = 0;
size_t stack_size = 0;
int current_stack_size_size = 0;
int label_number = 0;
int loop_label_number = 0;
int text_label = 0;
size_t current_stack_size[MAX_STACK_SIZE];
const unsigned initial_size = 100;
struct hashmap_s hashmap;

typedef enum { ADD, SUB, DIV, MUL, MOD, NOT_OPERATOR } OperatorType;

void create_label(FILE* file, int num) {
    label_number--;
    fprintf(file, "Llabel%d:\n", num);
}

void create_end_loop(FILE* file) {
    loop_label_number--;
    fprintf(file, "    b Lloop%d\n", loop_label_number);
}

void create_loop_label(FILE* file) {
    fprintf(file, "Lloop%d:\n", loop_label_number);
    loop_label_number++;
}

void if_label(FILE* file, char* comp, int num) {
    if (strcmp(comp, "eq") == 0) {
        fprintf(file, "    b.ne Llabel%d\n", num);
    } else if (strcmp(comp, "neq") == 0) {
        fprintf(file, "    b.eq Llabel%d\n", num);
    } else if (strcmp(comp, "less") == 0) {
        fprintf(file, "    b.ge Llabel%d\n", num);
    } else if (strcmp(comp, "greater") == 0) {
        fprintf(file, "    b.le Llabel%d\n", num);
    } else {
        printf("ERROR: unexpected comparator\n");
        exit(1);
    }
    label_number++;
}

void stack_push(size_t value) {
    current_stack_size_size++;
    current_stack_size[current_stack_size_size] = value;
}

size_t stack_pop() {
    if (current_stack_size_size == 0) {
        printf("ERROR: stack is already empty\n");
        exit(1);
    }
    size_t result = current_stack_size[current_stack_size_size];
    return result;
}

void scope_stak_push(char* value) {
    scope_stak_size++;
    scope_stak[scope_stak_size] = value;
}

char* scope_stak_pop() {
    if (scope_stak_size == 0) {
        return NULL;
    }
    char* result = scope_stak[scope_stak_size];
    scope_stak_size--;
    return result;
}

char* scope_stak_peek() {
    if (scope_stak_size == 0) {
        return NULL;
    }
    return scope_stak[scope_stak_size];
}

static int log_and_free_out_of_scope(void* const context, struct hashmap_element_s* const e) {
    (void) (context);
    if (*(size_t*) e->data > (current_stack_size[current_stack_size_size] + 1)) {
        if (hashmap_remove(&hashmap, e->key, strlen(e->key)) != 0) {
            printf("COULD NOT REMOVE ELEMENT\n");
        }
    }
    return 0;
}

void push(char* reg, FILE* file) {
    // Check if reg is an immediate value (number)
    if (reg[0] >= '0' && reg[0] <= '9') {
        fprintf(file, "    mov x9, #%s\n", reg);
        fprintf(file, "    str x9, [sp, #-16]!\n");
    } else {
        fprintf(file, "    str %s, [sp, #-16]!\n", reg);
    }
    stack_size++;
}

void push_var(size_t stack_pos, char* var_name, FILE* file) {
    fprintf(file, "    ldr x0, [sp, #%zu]\n", (stack_size - stack_pos) * 16);
    fprintf(file, "    str x0, [sp, #-16]!\n");
    stack_size++;
}

void modify_var(size_t stack_pos, char* new_value, char* var_name, FILE* file) {
    // Check if new_value is a register or immediate
    if (new_value[0] == 'x') {
        // It's a register
        fprintf(file, "    str %s, [sp, #%zu]\n", new_value, (stack_size - stack_pos) * 16);
        fprintf(file, "    ldr x0, [sp, #%zu]\n", (stack_size - stack_pos) * 16);
        fprintf(file, "    str x0, [sp, #-16]!\n");
    } else {
        // It's an immediate value
        fprintf(file, "    mov x9, #%s\n", new_value);
        fprintf(file, "    str x9, [sp, #%zu]\n", (stack_size - stack_pos) * 16);
        fprintf(file, "    ldr x0, [sp, #%zu]\n", (stack_size - stack_pos) * 16);
        fprintf(file, "    str x0, [sp, #-16]!\n");
    }
}

void pop(char* reg, FILE* file) {
    stack_size--;
    fprintf(file, "    ldr %s, [sp], #16\n", reg);
    if (stack_size > 1000) {
        exit(1);
    }
}

void mov(char* reg1, char* reg2, FILE* file) {
    // Check if reg2 is an immediate value (number) or string literal
    if ((reg2[0] >= '0' && reg2[0] <= '9') || strcmp(reg2, "1") == 0) {
        fprintf(file, "    mov %s, #%s\n", reg1, reg2);
    } else {
        fprintf(file, "    mov %s, %s\n", reg1, reg2);
    }
}

OperatorType check_operator(Node* node) {
    if (node->type != OPERATOR) {
        return NOT_OPERATOR;
    }

    if (strcmp(node->value, "+") == 0) {
        return ADD;
    }
    if (strcmp(node->value, "-") == 0) {
        return SUB;
    }
    if (strcmp(node->value, "/") == 0) {
        return DIV;
    }
    if (strcmp(node->value, "*") == 0) {
        return MUL;
    }
    if (strcmp(node->value, "%") == 0) {
        return MOD;
    }
    return NOT_OPERATOR;
}

int mov_if_var_or_not(char* reg, Node* node, FILE* file) {
    if (node->type == IDENTIFIER) {
        int* value = hashmap_get(&hashmap, node->value, strlen(node->value));
        if (value == NULL) {
            printf("ERROR: Variable %s not declared in current scope\n", node->value);
            exit(1);
        }
        push_var(*value, node->value, file);
        pop(reg, file);
        return 0;
    }
    if (node->type == INT) {
        fprintf(file, "    mov %s, #%s\n", reg, node->value);
        return 0;
    }
    return -1;
}

Node* op_handler(FILE* file, Node* cur) {
    mov_if_var_or_not("x0", cur->left, file);
    push("x0", file);
    Node* tmp = cur;
    OperatorType oper_type = check_operator(tmp);
    while (tmp->type == OPERATOR) {
        pop("x0", file);
        oper_type = check_operator(tmp);
        tmp = tmp->right;
        if (tmp->type != OPERATOR) {
            break;
        }
        mov_if_var_or_not("x1", tmp->left, file);
        switch (oper_type) {
            case ADD:
                fprintf(file, "    add x0, x0, x1\n");
                break;
            case SUB:
                fprintf(file, "    sub x0, x0, x1\n");
                break;
            case DIV:
                fprintf(file, "    udiv x0, x0, x1\n");
                break;
            case MUL:
                fprintf(file, "    mul x0, x0, x1\n");
                break;
            case MOD:
                fprintf(file, "    udiv x2, x0, x1\n");
                fprintf(file, "    msub x0, x2, x1, x0\n");
                break;
            case NOT_OPERATOR:
                printf("ERROR: Invalid syntax\n");
                exit(1);
                break;
        }
        push("x0", file);
        oper_type = check_operator(tmp);
    }
    mov_if_var_or_not("x1", tmp, file);
    switch (oper_type) {
        case ADD:
            fprintf(file, "    add x0, x0, x1\n");
            break;
        case SUB:
            fprintf(file, "    sub x0, x0, x1\n");
            break;
        case DIV:
            fprintf(file, "    udiv x0, x0, x1\n");
            break;
        case MUL:
            fprintf(file, "    mul x0, x0, x1\n");
            break;
        case MOD:
            fprintf(file, "    udiv x2, x0, x1\n");
            fprintf(file, "    msub x0, x2, x1, x0\n");
            break;
        case NOT_OPERATOR:
            printf("ERROR: Invalid syntax\n");
            exit(1);
            break;
    }
    push("x0", file);
    cur->left = NULL;
    cur->right = NULL;
    return cur;
}

void traverse_tree(Node* cur, int is_left, FILE* file, int syscall_number) {
    if (cur == NULL) {
        return;
    }
    if (strcmp(cur->value, "exit") == 0) {
        syscall_number = 1; // exit syscall on macOS
    }

    if (strcmp(cur->value, "int") == 0) {
        printf("reading int\n\n");

        if (!cur->left || !cur->left->left || !cur->left->left->left) {
            printf("ERROR: Malformed AST for int declaration\n");
            exit(1);
        }

        Node* value = malloc(sizeof(Node));
        value = cur->left->left->left;
        if (value->type == IDENTIFIER) {
            size_t* var_value = malloc(sizeof(size_t));
            var_value = hashmap_get(&hashmap, value->value, strlen(value->value));
            if (var_value != 0) {
                printf("ERROR\n");
                exit(1);
            }
            if (var_value == NULL) {
                printf("ERROR: %s Not declared in Currect Scope\n", value->value);
                exit(1);
            }
            push_var(*var_value, value->value, file);
        } else if (value->type == INT) {
            push(value->value, file);
        } else if (value->type == OPERATOR) {
            op_handler(file, value);
        } else {
            printf("ERROR\n");
            exit(1);
        }
        size_t* var_location = malloc(sizeof(size_t));
        size_t* cur_size = malloc(sizeof(size_t));
        *cur_size = stack_size;
        if (hashmap_get(&hashmap, cur->left->value, strlen(cur->left->value)) != NULL) {
            printf("ERROR: Variable %s is alerady declared in current scope\n", cur->left->value);
            exit(1);
        }
        if (hashmap_put(&hashmap, cur->left->value, strlen(cur->left->value), cur_size) != 0) {
            printf("ERROR: Could not insert into hashmap\n");
            exit(1);
        }
        cur->left = NULL;
    } else if (strcmp(cur->value, "IF") == 0) {
        printf("reading if\n\n");
        scope_stak_push("IF");
        Node* current = malloc(sizeof(Node));
        current = cur->left->left;
        if (current->left->type == INT || current->left->type == IDENTIFIER) {
            mov_if_var_or_not("x0", current->left, file);
            push("x0", file);
        } else {
            op_handler(file, cur->left);
        }

        if (current->right->type == INT || current->right->type == IDENTIFIER) {
            mov_if_var_or_not("x0", current->right, file);
            push("x0", file);
        } else {
            op_handler(file, current->right);
        }
        pop("x0", file);
        pop("x1", file);
        fprintf(file, "    cmp x0, x1\n");
        if_label(file, current->value, scope_count);
        cur->left->left = NULL;
    } else if (strcmp(cur->value, "while") == 0) {
        printf("reading while\n\n");
        scope_stak_push("W");
        create_loop_label(file);
        Node* current = malloc(sizeof(Node));
        current = cur->left->left;
        if (current->left->type == INT || current->left->type == IDENTIFIER) {
            mov_if_var_or_not("x0", current->left, file);
            push("x0", file);
        } else {
            op_handler(file, current->left);
        }
        if (current->right->type == INT || current->right->type == IDENTIFIER) {
            mov_if_var_or_not("x1", current->right, file);
            push("x1", file);
        } else {
            op_handler(file, current->right);
        }

        printf("testing...\n\n");

        pop("x1", file);
        pop("x0", file);
        fprintf(file, "    cmp x0, x1\n");

        printf("testing...\n\n");

        if (strcmp(current->value, "eq") == 0) {
            if_label(file, current->value, scope_count);
        } else if (strcmp(current->value, "neq") == 0) {
            if_label(file, current->value, scope_count);
        } else if (strcmp(current->value, "less") == 0) {
            if_label(file, current->value, scope_count);
        } else if (strcmp(current->value, "greater") == 0) {
            if_label(file, current->value, scope_count);
        } else {
            printf("ERROR: unknown operator: %s\n", current->value);
            exit(1);
        }
        cur->left->left = NULL;
    } else if (strcmp(cur->value, "write") == 0) {
        char* text = malloc(sizeof(char) * 8);
        char* identifier = malloc(sizeof(char) * 8);
        if (cur->left->type == IDENTIFIER) {
            identifier = hashmap_get(&hashmap, cur->left->value, strlen(cur->left->value));
            if (identifier == NULL) {
                printf("ERROR: value is not defined\n");
                exit(1);
            }
            push_var(*identifier, cur->right->value, file);
            // For printf on macOS, we'd need to link with system libraries
            // This is a simplified version for demonstration
            pop("x1", file);
            fprintf(file, "    adrp x0, Lprintf_format@PAGE\n");
            fprintf(file, "    add x0, x0, Lprintf_format@PAGEOFF\n");
            fprintf(file, "    bl _printf\n");
        } else {
            identifier = cur->left->value;
            sprintf(text, "Ltext%d", text_label);
            fprintf(file, ".section __DATA,__data\n");
            fprintf(file, "%s: .asciz \"%s\\n\"\n", text, cur->left->value);
            fprintf(file, ".section __TEXT,__text\n");

            // Write system call for macOS
            mov("x16", "4", file); // write syscall
            mov("x0", "1", file);  // stdout
            fprintf(file, "    adrp x1, %s@PAGE\n", text);
            fprintf(file, "    add x1, x1, %s@PAGEOFF\n", text);
            mov("x2", cur->right->value, file); // length
            fprintf(file, "    svc #0x80\n");
            text_label++;
            free(text);
        }
        Node* tmp = malloc(sizeof(Node));
        tmp = cur->right->right;
        cur->right = NULL;
        cur = tmp;
    }

    printf("testing...1\n\n");

    if (strcmp(cur->value, "(") == 0) {
    }

    if (cur->type == OPERATOR) {
        if (cur->value[0] == '=') {
        } else {
            op_handler(file, cur);
        }
    }

    if (cur->type == INT) {
        fprintf(file, "    mov x0, #%s\n", cur->value);
        push("x0", file);
    }

    if (cur->type == IDENTIFIER) {
        if (cur->left && cur->left->type == OPERATOR && strcmp(cur->left->value, "=") == 0) {
            // This is an assignment statement
            Node* equals_node = cur->left;
            Node* expression_node = equals_node->left;

            // Evaluate the expression on the right-hand side
            if (expression_node->type == IDENTIFIER) {
                size_t* var_value =
                    hashmap_get(&hashmap, expression_node->value, strlen(expression_node->value));
                if (var_value == NULL) {
                    printf("ERROR: Variable %s not declared in current scope\n",
                           expression_node->value);
                    exit(1);
                }
                push_var(*var_value, expression_node->value, file);
            } else if (expression_node->type == INT) {
                push(expression_node->value, file);
            } else if (expression_node->type == OPERATOR) {
                op_handler(file, expression_node);
            } else {
                printf("ERROR: Invalid expression in assignment\n");
                exit(1);
            }

            // Get the location of the variable being assigned to
            size_t* var_location = hashmap_get(&hashmap, cur->value, strlen(cur->value));
            if (var_location == NULL) {
                printf("ERROR: Variable %s not declared in current scope\n", cur->value);
                exit(1);
            }

            // Pop the result of the expression into x0 and modify the variable
            pop("x0", file);
            modify_var(*var_location + 1, "x0", cur->value, file);
            cur->left = NULL;             // Mark as processed
        } else if (syscall_number == 1) { // exit syscall
            size_t* var_value = malloc(sizeof(size_t));
            var_value = hashmap_get(&hashmap, cur->value, strlen(cur->value));
            if (var_value == NULL) {
                printf("ERROR: Not declared in current scope: %s\n", cur->value);
                exit(1);
            } else {
            }

            push_var(*var_value, cur->value, file);
            pop("x0", file);
            fprintf(file, "    mov x16, #%d\n", syscall_number);
            fprintf(file, "    svc #0x80\n");
            syscall_number = 0;
        } else {
            if (hashmap_get(&hashmap, cur->value, strlen(cur->value)) == NULL) {
                printf("ERROR: Variable %s is not declared in current scope\n", cur->value);
                exit(1);
            }

            Node* value = cur->left->left;
            size_t* var_location = malloc(sizeof(size_t));
            var_location = hashmap_get(&hashmap, cur->value, strlen(cur->value));
            if (value->type == IDENTIFIER) {
                size_t* var_value = malloc(sizeof(size_t));
                var_value = hashmap_get(&hashmap, value->value, strlen(value->value));
                if (var_value == NULL) {
                    printf("ERROR: %s Not declared in current scope\n", value->value);
                    exit(1);
                }
            } else if (value->type == INT) {
                push(value->value, file);
            } else if (value->type == OPERATOR) {
                op_handler(file, value);
            } else {
                printf("ERROR\n");
                exit(1);
            }
            size_t* cur_size = malloc(sizeof(size_t));
            *cur_size = stack_size;

            pop("x0", file);
            modify_var(*var_location + 1, "x0", cur->value, file);
            cur->left = NULL;
        }
    }

    if (strcmp(cur->value, ")") == 0) {
    }

    if (strcmp(cur->value, "{") == 0) {
        stack_push(stack_size);
        scope_count++;
        char* scope_count_string = malloc(sizeof(char) * 4);
        sprintf(scope_count_string, "%d", scope_count);
        scope_stak_push(scope_count_string);
    }

    if (strcmp(cur->value, "}") == 0) {
        char* current_scope = scope_stak_pop();
        char* next_scope = scope_stak_pop();

        if (!current_scope || !next_scope) {
            printf("ERROR: Scope stack underflow\n");
            exit(1);
        }

        if (next_scope[0] == 'I') {
            create_label(file, atoi(current_scope) - 1);
            global_scope = atoi(current_scope);
        } else if (next_scope[0] == 'W') {
            create_end_loop(file);
            create_label(file, atoi(current_scope) - 1);
            global_scope = atoi(current_scope);
        }

        size_t stack_value = stack_pop();
        for (; stack_size != stack_value;) {
            pop("x1", file);
        }

        void* log = malloc(sizeof(char));
        if (hashmap_iterate_pairs(&hashmap, log_and_free_out_of_scope, (void*) log) != 0) {
            exit(1);
        }
    }

    if (strcmp(cur->value, ";") == 0) {
        if (syscall_number == 1) { // exit syscall
            fprintf(file, "    mov x16, #%d\n", syscall_number);
            fprintf(file, "    ldr x0, [sp], #16\n");
            fprintf(file, "    svc #0x80\n");
            syscall_number = 0;
        }
    }
    if (is_left) {
    } else {
    }
    traverse_tree(cur->left, 1, file, syscall_number);
    traverse_tree(cur->right, 0, file, syscall_number);
}

void generate_code(Node* root) {
    insert('-', "sub");
    insert('+', "add");
    insert('*', "mul");
    insert('/', "udiv");

    FILE* assembly_file = fopen("assembly/generated.s", "w");
    if (assembly_file == NULL) {
        print_error("FILE COULD NOT BE OPENED", 1);
    }
    assert(hashmap_create(initial_size, &hashmap) == 0 && "ERROR: Could not create hashmap\n");

    // Apple Silicon assembly header
    fprintf(assembly_file, ".section __TEXT,__text\n");
    fprintf(assembly_file, ".global _main\n");
    fprintf(assembly_file, ".align 2\n\n");
    fprintf(assembly_file, "_main:\n");

    // Set up stack frame
    fprintf(assembly_file, "    stp x29, x30, [sp, #-16]!\n");
    fprintf(assembly_file, "    mov x29, sp\n");

    printf("entering traverse tree\n\n");

    traverse_tree(root, 0, assembly_file, 0);

    // Clean up and return
    fprintf(assembly_file, "    mov x0, #0\n");
    fprintf(assembly_file, "    ldp x29, x30, [sp], #16\n");
    fprintf(assembly_file, "    ret\n");

    fclose(assembly_file);

    printf("Running assembly");
    // system("assembly/build.sh");
}
