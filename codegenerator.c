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
int label_number = 0;
int loop_label_number = 0;
int text_label = 0;

// Frame pointer based addressing
int current_frame_offset = 0;      // Current offset from frame pointer (grows negative)
int frame_offsets[MAX_STACK_SIZE]; // Stack of frame offsets for nested scopes
int frame_stack_size = 0;

// Temporary expression stack (separate from variable storage)
int temp_stack_depth = 0;

const unsigned initial_size = 100;
struct hashmap_s hashmap;

typedef enum { ADD, SUB, DIV, MUL, MOD, NOT_OPERATOR } OperatorType;

// Frame management functions
void push_frame_scope() {
    frame_offsets[frame_stack_size++] = current_frame_offset;
}

void pop_frame_scope() {
    if (frame_stack_size > 0) {
        current_frame_offset = frame_offsets[--frame_stack_size];
    }
}

int allocate_frame_slot() {
    current_frame_offset -= 16; // Each slot is 16 bytes (8-byte aligned)
    return current_frame_offset;
}

// Label creation functions
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

// Scope stack functions
void scope_stak_push(char* value) {
    if (scope_stak_size >= MAX_STACK_SIZE - 1) {
        printf("ERROR: Scope stack overflow\n");
        exit(1);
    }
    scope_stak[++scope_stak_size] = strdup(value);
}

char* scope_stak_pop() {
    if (scope_stak_size == 0) {
        return NULL;
    }
    char* result = scope_stak[scope_stak_size];
    scope_stak_size--;
    return result;
}

// Forward declarations
void evaluate_expression(Node* expr, FILE* file);
void load_value_to_reg(char* reg, Node* node, FILE* file);

// Expression stack functions (for temporary values during expression evaluation)
void push_temp(char* reg, FILE* file) {
    fprintf(file, "    str %s, [sp, #-16]!\n", reg);
    temp_stack_depth++;
}

void pop_temp(char* reg, FILE* file) {
    if (temp_stack_depth <= 0) {
        printf("ERROR: Temporary stack underflow (depth: %d)\n", temp_stack_depth);
        printf("Attempting to pop into register: %s\n", reg);
        exit(1);
    }
    fprintf(file, "    ldr %s, [sp], #16\n", reg);
    temp_stack_depth--;
}

// Variable access functions
void store_var(int frame_offset, char* reg, FILE* file) {
    fprintf(file, "    str %s, [x29, #%d]\n", reg, frame_offset);
}

void load_var(int frame_offset, char* reg, FILE* file) {
    fprintf(file, "    ldr %s, [x29, #%d]\n", reg, frame_offset);
}

void mov_immediate_or_reg(char* dest_reg, char* value, FILE* file) {
    if (value[0] >= '0' && value[0] <= '9') {
        fprintf(file, "    mov %s, #%s\n", dest_reg, value);
    } else {
        fprintf(file, "    mov %s, %s\n", dest_reg, value);
    }
}

// Operator handling
OperatorType check_operator(Node* node) {
    if (node->type != OPERATOR) {
        return NOT_OPERATOR;
    }

    if (strcmp(node->value, "+") == 0)
        return ADD;
    if (strcmp(node->value, "-") == 0)
        return SUB;
    if (strcmp(node->value, "/") == 0)
        return DIV;
    if (strcmp(node->value, "*") == 0)
        return MUL;
    if (strcmp(node->value, "%") == 0)
        return MOD;
    return NOT_OPERATOR;
}

// Load value into register (handles variables, constants, expressions)
void load_value_to_reg(char* reg, Node* node, FILE* file) {
    if (node->type == IDENTIFIER) {
        int* frame_offset = hashmap_get(&hashmap, node->value, strlen(node->value));
        if (frame_offset == NULL) {
            printf("ERROR: Variable %s not declared in current scope\n", node->value);
            exit(1);
        }
        load_var(*frame_offset, reg, file);
    } else if (node->type == INT) {
        mov_immediate_or_reg(reg, node->value, file);
    } else if (node->type == OPERATOR) {
        // Handle expression - evaluate and get result directly
        evaluate_expression(node, file);
        pop_temp(reg, file);
    } else {
        printf("ERROR: Unsupported node type in load_value_to_reg\n");
        exit(1);
    }
}

// Expression evaluation with proper temporary stack management
void evaluate_expression(Node* expr, FILE* file) {
    if (expr->type == INT) {
        mov_immediate_or_reg("x0", expr->value, file);
        push_temp("x0", file);
    } else if (expr->type == IDENTIFIER) {
        int* frame_offset = hashmap_get(&hashmap, expr->value, strlen(expr->value));
        if (frame_offset == NULL) {
            printf("ERROR: Variable %s not declared\n", expr->value);
            exit(1);
        }
        load_var(*frame_offset, "x0", file);
        push_temp("x0", file);
    } else if (expr->type == OPERATOR) {
        OperatorType op = check_operator(expr);
        if (op == NOT_OPERATOR) {
            printf("ERROR: Unknown operator %s\n", expr->value);
            exit(1);
        }

        // Evaluate left operand
        evaluate_expression(expr->left, file);

        // Evaluate right operand
        evaluate_expression(expr->right, file);

        // Pop operands and perform operation
        pop_temp("x1", file); // Right operand
        pop_temp("x0", file); // Left operand

        switch (op) {
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
            default:
                printf("ERROR: Unhandled operator\n");
                exit(1);
        }

        // Push result back onto temp stack
        push_temp("x0", file);
    }
}

// Cleanup function to remove variables that have gone out of scope
static int remove_out_of_scope_vars(void* const context, struct hashmap_element_s* const e) {
    int current_offset = *(int*) context;
    int* var_offset = (int*) e->data;

    // Remove variables allocated after the current frame offset
    if (*var_offset < current_offset) {
        if (hashmap_remove(&hashmap, e->key, strlen(e->key)) != 0) {
            printf("WARNING: Could not remove variable from hashmap\n");
        }
    }
    return 0;
}

void traverse_tree(Node* cur, int is_left, FILE* file, int syscall_number) {
    if (cur == NULL) {
        return;
    }

    if (strcmp(cur->value, "exit") == 0) {
        syscall_number = 1;
    }

    if (strcmp(cur->value, "int") == 0) {
        // Variable declaration: int var_name = expression;
        if (!cur->left || !cur->left->left || !cur->left->left->left) {
            printf("ERROR: Malformed AST for int declaration\n");
            exit(1);
        }

        char* var_name = cur->left->value;
        Node* init_expr = cur->left->left->left;

        // Check if variable already exists in current scope
        if (hashmap_get(&hashmap, var_name, strlen(var_name)) != NULL) {
            printf("ERROR: Variable %s already declared in current scope\n", var_name);
            exit(1);
        }

        // Allocate frame slot for this variable
        int frame_offset = allocate_frame_slot();
        int* offset_ptr = malloc(sizeof(int));
        *offset_ptr = frame_offset;

        // Evaluate initialization expression
        evaluate_expression(init_expr, file);
        pop_temp("x0", file);

        // Store in allocated slot
        store_var(frame_offset, "x0", file);

        // Add to symbol table
        if (hashmap_put(&hashmap, var_name, strlen(var_name), offset_ptr) != 0) {
            printf("ERROR: Could not add variable to symbol table\n");
            exit(1);
        }

    } else if (strcmp(cur->value, "IF") == 0) {
        scope_stak_push("IF");
        push_frame_scope();

        Node* condition = cur->left->left;

        // Evaluate condition operands
        load_value_to_reg("x0", condition->left, file);
        load_value_to_reg("x1", condition->right, file);

        fprintf(file, "    cmp x0, x1\n");
        if_label(file, condition->value, scope_count);

    } else if (strcmp(cur->value, "while") == 0) {
        scope_stak_push("W");
        push_frame_scope();
        create_loop_label(file);

        Node* condition = cur->left->left;

        // Evaluate condition
        load_value_to_reg("x0", condition->left, file);
        load_value_to_reg("x1", condition->right, file);

        fprintf(file, "    cmp x0, x1\n");
        if_label(file, condition->value, scope_count);

    } else if (strcmp(cur->value, "write") == 0) {
        if (cur->left->type == IDENTIFIER) {
            int* frame_offset = hashmap_get(&hashmap, cur->left->value, strlen(cur->left->value));
            if (frame_offset == NULL) {
                printf("ERROR: Variable %s not declared\n", cur->left->value);
                exit(1);
            }
            load_var(*frame_offset, "x1", file);
            fprintf(file, "    adrp x0, Lprintf_format@PAGE\n");
            fprintf(file, "    add x0, x0, Lprintf_format@PAGEOFF\n");
            fprintf(file, "    bl _printf\n");
        } else {
            // String literal
            char text_label_name[32];
            sprintf(text_label_name, "Ltext%d", text_label);

            fprintf(file, ".section __DATA,__data\n");
            fprintf(file, "%s: .asciz \"%s\\n\"\n", text_label_name, cur->left->value);
            fprintf(file, ".section __TEXT,__text\n");

            mov_immediate_or_reg("x16", "4", file);
            mov_immediate_or_reg("x0", "1", file);
            fprintf(file, "    adrp x1, %s@PAGE\n", text_label_name);
            fprintf(file, "    add x1, x1, %s@PAGEOFF\n", text_label_name);
            if (cur->right && cur->right->value) {
                mov_immediate_or_reg("x2", cur->right->value, file);
            } else {
                mov_immediate_or_reg("x2", "1", file); // Default length
            }
            fprintf(file, "    svc #0x80\n");
            text_label++;
        }
    }

    // Handle assignment
    if (cur->type == IDENTIFIER && cur->left && cur->left->type == OPERATOR &&
        strcmp(cur->left->value, "=") == 0) {
        char* var_name = cur->value;
        Node* expr = cur->left->left;

        int* frame_offset = hashmap_get(&hashmap, var_name, strlen(var_name));
        if (frame_offset == NULL) {
            printf("ERROR: Variable %s not declared\n", var_name);
            exit(1);
        }

        // Evaluate expression and store result
        evaluate_expression(expr, file);
        pop_temp("x0", file);
        store_var(*frame_offset, "x0", file);
    }

    // Handle scope opening
    if (strcmp(cur->value, "{") == 0) {
        push_frame_scope();
        scope_count++;
        char scope_str[16];
        sprintf(scope_str, "%d", scope_count);
        scope_stak_push(scope_str);
    }

    // Handle scope closing
    if (strcmp(cur->value, "}") == 0) {
        char* current_scope = scope_stak_pop();
        char* prev_scope = scope_stak_pop();

        if (current_scope && prev_scope) {
            if (prev_scope[0] == 'I') {
                create_label(file, atoi(current_scope) - 1);
            } else if (prev_scope[0] == 'W') {
                create_end_loop(file);
                create_label(file, atoi(current_scope) - 1);
            }
        }

        // Clean up variables that went out of scope
        hashmap_iterate_pairs(&hashmap, remove_out_of_scope_vars, &current_frame_offset);

        // Restore previous frame scope
        pop_frame_scope();
    }

    // Handle exit syscall
    if (cur->type == IDENTIFIER && syscall_number == 1) {
        int* frame_offset = hashmap_get(&hashmap, cur->value, strlen(cur->value));
        if (frame_offset == NULL) {
            printf("ERROR: Variable %s not declared\n", cur->value);
            exit(1);
        }
        load_var(*frame_offset, "x0", file);
        fprintf(file, "    mov x16, #%d\n", syscall_number);
        fprintf(file, "    svc #0x80\n");
        syscall_number = 0;
    }

    // Handle semicolon (statement terminator)
    if (strcmp(cur->value, ";") == 0) {
        // Just a statement terminator, no action needed
    }

    // Recursively traverse
    traverse_tree(cur->left, 1, file, syscall_number);
    traverse_tree(cur->right, 0, file, syscall_number);
}

void generate_code(Node* root) {
    FILE* assembly_file = fopen("assembly/generated.s", "w");
    if (assembly_file == NULL) {
        printf("ERROR: Could not open assembly file\n");
        exit(1);
    }

    assert(hashmap_create(initial_size, &hashmap) == 0);

    // Apple Silicon assembly header
    fprintf(assembly_file, ".section __TEXT,__text\n");
    fprintf(assembly_file, ".global _main\n");
    fprintf(assembly_file, ".align 2\n\n");

    // Printf format string
    fprintf(assembly_file, ".section __DATA,__data\n");
    fprintf(assembly_file, "Lprintf_format: .asciz \"%%ld\\n\"\n");
    fprintf(assembly_file, ".section __TEXT,__text\n\n");

    fprintf(assembly_file, "_main:\n");

    // Establish frame pointer
    fprintf(assembly_file, "    stp x29, x30, [sp, #-16]!\n");
    fprintf(assembly_file, "    mov x29, sp\n");
    fprintf(assembly_file, "    sub sp, sp, #1024\n"); // Allocate space for local variables

    // Initialize frame management
    current_frame_offset = 0;
    frame_stack_size = 0;
    temp_stack_depth = 0;

    traverse_tree(root, 0, assembly_file, 0);

    // Cleanup and return
    fprintf(assembly_file, "    mov x0, #0\n");
    fprintf(assembly_file, "    add sp, sp, #1024\n"); // Deallocate local variables
    fprintf(assembly_file, "    ldp x29, x30, [sp], #16\n");
    fprintf(assembly_file, "    ret\n");

    fclose(assembly_file);

    // Build the assembly
    system("assembly/build.sh");
}
