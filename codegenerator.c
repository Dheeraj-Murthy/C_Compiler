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
    fprintf(file, "label%d\n", num);
}

void create_end_loop(FILE* file) {
    loop_label_number--;
    fprintf(file, " jmp loop%d\n", loop_label_number);
}

void create_loop_label(FILE* file) {
    fprintf(file, "loop%d\n", loop_label_number);
    loop_label_number++;
}

void if_label(FILE* file, char* comp, int num) {
    if (strcmp(comp, "EQ") == 0) {
        fprintf(file, " jne lable%d\n", num);
    } else if (strcmp(comp, "NEQ") == 0) {
        fprintf(file, " je label%d\n", num);
    } else if (strcmp(comp, "LESS") == 0) {
        fprintf(file, " jge label%d\n", num);
    } else if (strcmp(comp, "GREATER") == 0) {
        fprintf(file, " jle label%d\n", num);
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
    fprintf(file, " push %s\n", reg);
}
void push_var(size_t stack_pos, char* var_name, FILE* file) {
    fprintf(file, " push QWORD [rsp + %zu]\n", (stack_size - stack_pos) * 8);
    stack_size++;
}

void modify_var(size_t stack_pos, char* new_value, char* var_name, FILE* file) {
    fprintf(file, "mov QWORD [rsp + %zu], %s\n", (stack_size - stack_pos) * 8, new_value);
    fprintf(file, " push QWORD [rsp + %zu]\n", (stack_size - stack_pos) * 8);
}

void pop(char* reg, FILE* file) {
    stack_size--;
    fprintf(file, " pop %s\n", reg);
    if (stack_size > 1000) {
        exit(1);
    }
}

void mov(char* reg1, char* reg2, FILE* file) {
    fprintf(file, " mov %s, %s\n", reg1, reg2);
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
        int* value = malloc(sizeof(int));
        value = hashmap_get(&hashmap, node->value, strlen(node->value));
        if (value == NULL) {
            printf("ERROR: Variable %s not declared in current scope\n", node->value);
            exit(1);
        }
        push_var(*value, node->value, file);
        pop(reg, file);
        return 0;
    }
    if (node->type == INT) {
        fprintf(file, " mov %s, %s\n", reg, node->value);
        return 0;
    }
    return -1;
}

Node* op_handler(FILE* file, Node* cur) {
    mov_if_var_or_not("rax", cur->left, file);
    push("rax", file);
    Node* tmp = cur;
    OperatorType oper_type = check_operator(tmp);
    while (tmp->type == OPERATOR) {
        pop("rax", file);
        oper_type = check_operator(tmp);
        tmp = tmp->right;
        if (tmp->type != OPERATOR) {
            break;
        }
        mov_if_var_or_not("rbx", tmp->left, file);
        switch (oper_type) {
            case ADD:
                fprintf(file, " add rax, rbx\n");
                break;
            case SUB:
                fprintf(file, " sub rax, rbx\n");
                break;
            case DIV:
                fprintf(file, " xor rdx, rdx\n");
                fprintf(file, " div rbx\n");
                break;
            case MUL:
                fprintf(file, " mul rbx\n");
                break;
            case MOD:
                fprintf(file, " xor rdx, rdx\n");
                fprintf(file, " div rbx\n");
                break;
            case NOT_OPERATOR:
                printf("ERROR: Invalid syntax\n");
                exit(1);
                break;
        }
        if (oper_type != MOD) {
            push("rax", file);
        } else {
            push("rdx", file);
        }
        oper_type = check_operator(tmp);
    }
    mov_if_var_or_not("rbx", tmp, file);
    switch (oper_type) {
        case ADD:
            fprintf(file, " add rax, rbx\n");
            break;
        case SUB:
            fprintf(file, " sub rax, rbx\n");
            break;
        case DIV:
            fprintf(file, " xor rdx, rdx\n");
            fprintf(file, " div rbx\n");
            break;
        case MUL:
            fprintf(file, " mul rbx\n");
            break;
        case MOD:
            fprintf(file, " xor rdx, rdx\n");
            fprintf(file, " div rbx\n");
            break;
        case NOT_OPERATOR:
            printf("ERROR: Invalid syntax\n");
            exit(1);
            break;
    }
    if (oper_type != MOD) {
        push("rax", file);
    } else {
        push("rdx", file);
    }
    cur->left = NULL;
    cur->right = NULL;
    return cur;
    //
    // if (strcmp(cur->value, "=") == 0) {
    //     // generate_code(cur->right)
    //     fprintf(file, " push %s\n", cur->left->value);
    //     cur->left = NULL;
    //     return cur;
    // }
    //
    // fprintf(file, "\tmov X0, #%s\n", cur->left->value);
    // Node* tmp = cur->right;
    // char* operator = search(cur->value[0])->data;
    // while (tmp->type == OPERATOR) {
    //     printf("going inside operator rn, %s\n", tmp->value);
    //
    //     if (tmp->left && tmp->left->type == INT) {
    //         if (strcmp(operator, "mul") == 0 || strcmp(operator, "sdiv") == 0) {
    //             fprintf(file, "\tmov X1, #%s\n", tmp->left->value);
    //             fprintf(file, "\t%s X0, X0, X1\n", operator);
    //         } else
    //             fprintf(file, "\t%s X0, X0, #%s\n", operator, tmp->left->value);
    //     }
    //
    //     operator = search(tmp->value[0])->data;
    //     if (tmp->right && tmp->right->type == INT) {
    //         // fprintf(file, "\t%s X0, X0, #%s\n", operator, tmp->right->value);
    //         tmp = tmp->right;
    //         break;
    //     } else if (tmp->right && tmp->right->type == OPERATOR) {
    //         // This handles deeply nested expressions
    //         tmp = tmp->right;
    //     } else {
    //         print_error("Invalid expression after operator");
    //     }
    //     // fprintf(file, "\t%s X0, X0, #%s\n", operator, tmp->left->value);
    // }
    //
    // if (strcmp(operator, "mul") == 0 || strcmp(operator, "sdiv") == 0) {
    //     fprintf(file, "\tmov X1, #%s\n", tmp->value);
    //     fprintf(file, "\t%s X0, X0, X1\n", operator);
    // } else
    //     fprintf(file, "\t%s X0, X0, #%s\n", operator, tmp->value);
    // cur->left = NULL;
    // cur->right = NULL;
    // return cur;
}

void traverse_tree(Node* cur, int is_left, FILE* file, int syscall_number) {
    if (cur == NULL) {
        return;
    }
    if (strcmp(cur->value, "exit") == 0) {
        // fprintf(file, "\tmov X16, #1\n");
        syscall_number = 60;
    }

    if (strcmp(cur->value, "int") == 0) {
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
        scope_stak_push("IF");
        Node* current = malloc(sizeof(Node));
        current = cur->left->left;
        if (current->left->type == INT || current->left->type == IDENTIFIER) {
            mov_if_var_or_not("rax", current->left, file);
            push("rax", file);
        } else {
            op_handler(file, cur->left);
        }

        if (current->right->type == INT || current->right->type == IDENTIFIER) {
            mov_if_var_or_not("rax", current->right, file);
            push("rax", file);
        } else {
            op_handler(file, current->right);
        }
        pop("rax", file);
        pop("rbx", file);
        fprintf(file, " cmp rax, rbx\n");
        if_label(file, current->value, scope_count);
        cur->left->left == NULL;
    } else if (strcmp(cur->value, "while") == 0) {
        scope_stak_push("W");
        create_loop_label(file);
        Node* current = malloc(sizeof(Node));
        current = cur->left->left;
        if (current->left->type == INT || current->left->type == IDENTIFIER) {
            mov_if_var_or_not("rax", current->left, file);
            push("rax", file);
        } else {
            op_handler(file, current->left);
        }
        if (current->right->type == INT || current->right->type == IDENTIFIER) {
            mov_if_var_or_not("rbx", current->right, file);
            push("rbx", file);
        } else {
            op_handler(file, current->right);
        }
        pop("rbx", file);
        pop("rax", file);
        fprintf(file, " cmp rax, rbx\n");

        if (strcmp(current->value, "EQ") == 0) {
            if_label(file, current->value, scope_count);
        } else if (strcmp(current->value, "NEQ") == 0) {
            if_label(file, current->value, scope_count);
        } else if (strcmp(current->value, "LESS") == 0) {
            if_label(file, current->value, scope_count);
        } else if (strcmp(current->value, "GREATER") == 0) {
            if_label(file, current->value, scope_count);
        } else {
            printf("ERROR: unknown operator\n");
            exit(1);
        }
        cur->left->left = NULL;
    } else if (strcmp(cur->value, "WRITE") == 0) {
        char* text = malloc(sizeof(char) * 8);
        char* identifier = malloc(sizeof(char) * 8);
        if (cur->left->type == IDENTIFIER) {
            identifier = hashmap_get(&hashmap, cur->left->value, strlen(cur->left->value));
            if (identifier == NULL) {
                printf("ERROR: value is not defined\n");
                exit(1);
            }
            push_var(*identifier, cur->right->value, file);
            mov("rdi", "printf_format", file);
            pop("rsi", file);

            fprintf(file, " xor rax, rax\n");
            fprintf(file, " call printf WRT ..plt\n");
        } else {
            identifier = cur->left->value;
            sprintf(text, "text%d", text_label);
            fprintf(file, "section .data\n");
            fprintf(file, " %s db \"%s\", 10\n", text, cur->left->value);
            fprintf(file, "section .text\n");

            mov("rax", "1", file);
            mov("rdx", cur->right->value, file);
            mov("rdi", "1", file);
            mov("rsi", text, file);
            text_label++;
            free(text);
            fprintf(file, " syscall\n");
        }
        Node* tmp = malloc(sizeof(Node));
        tmp = cur->right->right;
        cur->right = NULL;
        cur = tmp;
    }

    if (strcmp(cur->value, "(") == 0) {
    }

    if (cur->type == OPERATOR) {
        if (cur->value[0] == '=') {
        } else {
            op_handler(file, cur);
        }
    }

    if (cur->type == INT) {
        fprintf(file, " mov rax, %s\n", cur->value);
        push("rax", file);
    }

    if (cur->type == IDENTIFIER) {
        if (syscall_number == 60) {
            size_t* var_value = malloc(sizeof(size_t));
            var_value = hashmap_get(&hashmap, cur->value, strlen(cur->value));
            if (var_value == NULL) {
                printf("ERROR: Not declared in current scope: %s\n", cur->value);
                exit(1);
            } else {
            }

            push_var(*var_value, cur->value, file);
            pop("rdi", file);
            fprintf(file, " mov rax, %d\n", syscall_number);
            fprintf(file, " syscall\n");
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

            pop("rax", file);
            modify_var(*var_location + 1, "rax", cur->value, file);
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

        if (next_scope[0] == 'I') {
            create_label(file, atoi(current_scope) - 1);
            global_scope = atoi(current_scope);
        } else if (next_scope[0] == 'W') {
            create_end_loop(file);
            create_label(file, atoi(current_scope) - 1);
            global_scope = atoi(current_scope);
        }

        size_t stack_value = stack_pop();
        while (stack_size == stack_value) {
            pop("rsi", file);
        }

        void* log = malloc(sizeof(char));
        if (hashmap_iterate_pairs(&hashmap, log_and_free_out_of_scope, (void*) log) != 0) {
            exit(1);
        }
    }

    if (strcmp(cur->value, ";") == 0) {
        if (syscall_number == 60) {
            fprintf(file, " mov rax, %d\n", syscall_number);
            fprintf(file, " pop rdi\n");
            fprintf(file, " syscall\n");
            syscall_number = 0;
        }
    }
    if (is_left) {
    } else {
    }
    traverse_tree(cur->left, 1, file, syscall_number);
    traverse_tree(cur->right, 0, file, syscall_number);

    // if (cur->type == IDENTIFIER) {
    //     if (syscall_number == 60) {
    //     }
    // }
    // if (cur->type == OPERATOR) {
    //     cur = op_handler(file, cur);
    // }
    // if (cur->type == INT) {
    //     fprintf(file, "\tmov X0, %s\n", cur->value);
    // }
    // if (strcmp(cur->value, ")") == 0) {
    // }
    // if (strcmp(cur->value, ";") == 0) {
    //     fprintf(file, "\tsvc 0x80\n");
    // }
    // if (is_left) {
    // } else {
    // }
    //
    // for (size_t i = 0; cur->value[i] != '\0'; i++) {
    //     printf("%c", cur->value[i]);
    // }
    // printf("\n");
    // traverse_tree(cur->left, 1, file, syscall_number);
    // traverse_tree(cur->right, 0, file, syscall_number);
}

void generate_code(Node* root) {
    // print_tree(root, 0);
    insert('-', "sub");
    insert('+', "add");
    insert('*', "mul");
    insert('/', "sdiv");

    FILE* assembly_file = fopen("assembly/generated.s", "w");
    if (assembly_file == NULL) {
        print_error("FILE COULD NOT BE OPENED", 1);
    }
    assert(hashmap_create(initial_size, &hashmap) == 0 && "ERROR: Could not create hashmap\n");

    fprintf(assembly_file, ".global _start\n\n");
    fprintf(assembly_file, "_start:\n");
    // print_tree(root, 0, "root");

    traverse_tree(root, 0, assembly_file, 0);
    fclose(assembly_file);

    printf("Running assembly");
    system("assembly/build.sh");
}
