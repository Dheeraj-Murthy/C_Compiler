#include "parser.h"
#include "lexer.h"

#include <stdlib.h>
#include <string.h>

void print_tree(Node* current, int depth) {
    if (!current)
        return;
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

Node* peek_scope(scope_stack* stack) {
    return stack->content[stack->top];
}

void push_scope(scope_stack* stack, Node* element) {
    stack->top++;
    stack->content[stack->top] = element;
}

Node* pop_scope(scope_stack* stack) {
    Node* result = stack->content[stack->top];
    stack->top--;
    return result;
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

Node* parse_expression(Token** tokens, int* token_number, Node* current_node) {
    Node* expr_node = malloc(sizeof(Node));
    expr_node = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
    (*token_number)++;
    if (tokens[*token_number]->type != OPERATOR) {
        return expr_node;
    }
    return expr_node;
}

void oper_tree(Token** tokens, int* i, Node* par) {
    expect(tokens, *i, OPERATOR, tokens[*i]->word);
    Node* opr_node = create_Node(tokens[*i]->word, OPERATOR);
    if (par->right == NULL) {
        opr_node->left = par->left;
        par->left = opr_node;
    } else {
        opr_node->left = par->right;
        par->right = opr_node;
    }
    (*i)++;
    par = opr_node;

    // Expect literal
    if (tokens[*i]->type != INT) {
        print_error("Expected integer literal after OPERATOR", tokens[*i]->line_num);
        exit(1);
    }
    Node* expr_node = create_Node(tokens[*i]->word, INT);
    par->right = expr_node;
    (*i)++;

    if (tokens[*i]->type != END_TOKEN && tokens[*i]->type == OPERATOR) {
        oper_tree(tokens, i, par);
    }
}

void print_error(char* msg, size_t line_number) {
    printf("ERROR: %s || line_number: %zu\n", msg, line_number);

    exit(1);
}

void expect(Token** tokens, int i, TokenType type, const char* word) {
    if (tokens[i]->type != type || (word && strcmp(tokens[i]->word, word) != 0)) {
        printf("Syntax error: expected '%s' at token %d, found '%s', at line %zu\n", word, i,
               tokens[i]->word, tokens[i]->line_num);
        exit(1);
    }
}

Node* parse_exit(Token** tokens, int* i, Node* current) {
    Node* exit_node = create_Node(tokens[*i]->word, KEYWORD);
    (*i)++;
    current->right = exit_node;

    // Expect (
    expect(tokens, *i, SEPARATOR, "(");
    Node* open_paren = create_Node(tokens[*i]->word, SEPARATOR);
    exit_node->left = open_paren;
    (*i)++;

    // Expect literal
    if (tokens[*i]->type != INT)
        print_error("Expected integer literal inside exit()", tokens[*i]->line_num);
    Node* expr_node = create_Node(tokens[*i]->word, INT);
    open_paren->left = expr_node;
    (*i)++;

    if (tokens[*i]->type != END_TOKEN && tokens[*i]->type == OPERATOR) {
        oper_tree(tokens, i, open_paren);
    }

    // Expect )
    expect(tokens, *i, SEPARATOR, ")");
    Node* close_paren = create_Node(tokens[*i]->word, SEPARATOR);
    open_paren->right = close_paren;
    (*i)++;

    // Expect ;
    expect(tokens, *i, SEPARATOR, ";");
    Node* semi = create_Node(tokens[*i]->word, SEPARATOR);
    exit_node->right = semi;
    (*i)++;

    return semi;
}

Node* create_var_reuseable(Token** tokens, int* token_number, Node* current) {
    Node* identifier_main = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
    current->left = identifier_main;
    current = current->left;
    (*token_number)++;

    expect(tokens, *token_number, OPERATOR, "=");
    Node* equals_node = create_Node(tokens[*token_number]->word, OPERATOR);
    current->left = equals_node;
    current = current->left;
    (*token_number)++;

    expect(tokens, *token_number, INT, tokens[*token_number]->word);
    // ??

    (*token_number)++;

    if (tokens[*token_number]->type == OPERATOR) {
        Node* oper_node = create_Node(tokens[*token_number]->word, INT);
        current->left = oper_node;
        current = current->left;
        (*token_number)--;

        if (tokens[*token_number]->type == INT) {
            Node* expr_node = create_Node(tokens[*token_number]->word, INT);
            current->left = expr_node;
            (*token_number)++;
            (*token_number)++;
        } else if (tokens[*token_number]->type == IDENTIFIER) {
            Node* identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
            current->left = identifier_node;
            (*token_number)++;
            (*token_number)++;
        } else {
            print_error("ERROR: Expected IDENTIFIER or INT", tokens[*token_number]->line_num);
        }
        (*token_number)++;

        if (tokens[*token_number]->type == OPERATOR) {
            Node* oper_node = create_Node(tokens[*token_number]->word, OPERATOR);
            current->right = oper_node;
            current = current->right;
            int operation = 1;
            (*token_number) -= 2;
            while (operation) {
                (*token_number)++;
                if (tokens[*token_number]->type == INT) {
                    Node* expr_node = create_Node(tokens[*token_number]->word, INT);
                    current->left = expr_node;
                } else if (tokens[*token_number]->type == IDENTIFIER) {
                    Node* identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
                    current->left = identifier_node;
                } else {
                    print_error("Unexpected Token\n", tokens[*token_number]->line_num);
                }
                (*token_number)++;

                if (tokens[*token_number]->type == OPERATOR) {
                    (*token_number) += 2;
                    if (tokens[*token_number]->type != OPERATOR) {
                        (*token_number)--;
                        if (tokens[*token_number]->type == INT) {
                            Node* expr_node = create_Node(tokens[*token_number]->word, INT);
                            current->right = expr_node;
                            (*token_number)++;
                        } else if (tokens[*token_number]->type == IDENTIFIER) {
                            Node* identifier_node =
                                create_Node(tokens[*token_number]->word, IDENTIFIER);
                            current->right = identifier_node;
                            (*token_number)++;
                        } else {
                            print_error("UNRECOGNISED TOKEN!", tokens[*token_number]->line_num);
                        }
                        operation = 0;
                    } else {
                        (*token_number) -= 2;
                        Node* oper_node = create_Node(tokens[*token_number]->word, OPERATOR);
                        current->right = oper_node;
                        current = current->right;
                    }
                } else {
                    operation = 0;
                }
            }
        } else {
            (*token_number)--;
            if (tokens[*token_number]->type == INT) {
                Node* expr_node = create_Node(tokens[*token_number]->word, INT);
                current->right = expr_node;
            } else if (tokens[*token_number]->type == IDENTIFIER) {
                Node* identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
                current->right = identifier_node;
            }
            (*token_number)++;
        }
    } else {
        (*token_number)--;
        if (tokens[*token_number]->type == INT) {
            Node* expr_node = create_Node(tokens[*token_number]->word, INT);
            current->left = expr_node;
        } else if (tokens[*token_number]->type == IDENTIFIER) {
            Node* identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
            current->left = identifier_node;
        }
        (*token_number)++;
    }
    expect(tokens, *token_number, SEPARATOR, ";");

    current = identifier_main;
    Node* semi_node = create_Node(tokens[*token_number]->word, SEPARATOR);
    current->right = semi_node;
    current = current->right;
    return current;
}

Node* create_variable(Token** tokens, int* token_number, Node* current) {
    Node* var_node = create_Node(tokens[*token_number]->word, KEYWORD);
    current->left = var_node;
    current = var_node;
    (*token_number)++;

    expect(tokens, *token_number, IDENTIFIER, tokens[*token_number]->word);
    Node* identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
    current->left = identifier_node;
    current = identifier_node;
    (*token_number)++;

    expect(tokens, *token_number, OPERATOR, "=");
    Node* equals_node = create_Node(tokens[*token_number]->word, OPERATOR);
    current->left = equals_node;
    current = equals_node;
    (*token_number)++;

    if (tokens[*token_number]->type == END_TOKEN ||
        (tokens[*token_number]->type != INT && tokens[*token_number]->type != IDENTIFIER)) {
        print_error("Invalid syntax after Equals", tokens[*token_number]->line_num);
    }

    (*token_number)++;
    if (tokens[*token_number]->type == OPERATOR) {
        Node* oper_node = create_Node(tokens[*token_number]->word, OPERATOR);
        current->left = oper_node;
        current = oper_node;
        (*token_number)--;

        if (tokens[*token_number]->type == INT) {
            Node* expr_node = create_Node(tokens[*token_number]->word, INT);
            oper_node->left = expr_node;
            (*token_number) += 2;
        } else if (tokens[*token_number]->type == IDENTIFIER) {
            Node* identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
            oper_node->left = identifier_node;
            (*token_number) += 2;
        } else {
            print_error("Expected Identifier or INT", tokens[*token_number]->line_num);
        }
        (*token_number)++;

        if (tokens[*token_number]->type == OPERATOR) {
            Node* oper_node = create_Node(tokens[*token_number]->word, OPERATOR);
            current->right = oper_node;
            current = oper_node;
            int operation = 1;
            (*token_number) -= 2;
            while (operation) {
                (*token_number)++;
                if (tokens[*token_number]->type == INT) {
                    Node* expr_node = create_Node(tokens[*token_number]->word, INT);
                    current->left = expr_node;
                } else if (tokens[*token_number]->type == IDENTIFIER) {
                    Node* identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
                    current->left = identifier_node;
                } else {
                    print_error("Expected INT or IDENTIFIER", tokens[*token_number]->line_num);
                }
                (*token_number)++;

                if (tokens[*token_number]->type == OPERATOR) {
                    (*token_number) += 2;
                    if (tokens[*token_number]->type != OPERATOR) {
                        (*token_number)--;
                        if (tokens[*token_number]->type == INT) {
                            Node* expr_node = create_Node(tokens[*token_number]->word, INT);
                            current->right = expr_node;
                        } else if (tokens[*token_number]->type == IDENTIFIER) {
                            Node* identifier_node =
                                create_Node(tokens[*token_number]->word, IDENTIFIER);
                            current->right = identifier_node;
                        } else {
                            print_error("expected int or identifier",
                                        tokens[*token_number]->line_num);
                        }
                        (*token_number)++;
                        operation = 0;
                    } else {
                        (*token_number) -= 2;
                        Node* oper_node = create_Node(tokens[*token_number]->word, OPERATOR);
                        current->right = oper_node;
                        current = oper_node;
                    }
                } else {
                    operation = 0;
                }
            }
        } else {
            (*token_number)--;
            if (tokens[*token_number]->type == INT) {
                Node* expr_node = create_Node(tokens[*token_number]->word, INT);
                oper_node->right = expr_node;
            } else if (tokens[*token_number]->type == IDENTIFIER) {
                Node* identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
                oper_node->right = identifier_node;
            } else {
                // print_error("expected int or identifier", tokens[*token_number]->line_num);
            }
            (*token_number)++;
        }
    } else {
        (*token_number)--;
        if (tokens[*token_number]->type == INT) {
            Node* expr_node = create_Node(tokens[*token_number]->word, INT);
            current->left = expr_node;
        } else if (tokens[*token_number]->type == IDENTIFIER) {
            Node* identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
            current->left = identifier_node;
        } else {
            // print_error("expected int or identifier", tokens[*token_number]->line_num);
        }
        (*token_number)++;
    }

    expect(tokens, *token_number, SEPARATOR, ";");
    current = var_node;
    Node* semi_node = create_Node(tokens[*token_number]->word, SEPARATOR);
    current->right = semi_node;
    current = semi_node;
    return semi_node;
}

int generate_if_operation_nodes(Token** tokens, int* token_number, Node* current_node) {
    Node* oper_node = create_Node(tokens[*token_number]->word, OPERATOR);
    current_node->left->left = oper_node;
    current_node = oper_node;
    (*token_number)--;

    Node* expr_node = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
    current_node->left = expr_node;
    (*token_number) += 2;

    while (tokens[*token_number]->type == INT || tokens[*token_number]->type == IDENTIFIER ||
           tokens[*token_number]->type == OPERATOR) {
        // if ((tokens[*token_number]->type != INT && tokens[*token_number]->type != IDENTIFIER) &&
        //     tokens[*token_number] == NULL) {
        //     print_error("Invalid token", tokens[*token_number]->line_num);
        // }
        if (tokens[*token_number + 1]->type != OPERATOR ||
            strcmp(tokens[*token_number + 1]->word, "=") == 0) {
            if (tokens[*token_number]->type == INT) {
                Node* second_expr_node = create_Node(tokens[*token_number]->word, INT);
                current_node->right = second_expr_node;
            } else if (tokens[*token_number]->type == IDENTIFIER) {
                Node* second_identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
                current_node->right = second_identifier_node;
            } else {
                print_error("Expected Integer or identifier ", tokens[*token_number]->line_num);
            }
        }
        if (strcmp(tokens[*token_number]->word, "=") == 0) {
            break;
        } else if (tokens[*token_number]->type == OPERATOR) {
            Node* next_oper_node = create_Node(tokens[*token_number]->word, OPERATOR);
            current_node->right = next_oper_node;
            (*token_number)--;
            if (tokens[*token_number]->type == INT) {
                Node* second_expr_node = create_Node(tokens[*token_number]->word, INT);
                current_node->left = second_expr_node;
            } else if (tokens[*token_number]->type == IDENTIFIER) {
                Node* second_identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
                current_node->left = second_identifier_node;
            } else {
                print_error("Expected INT or IDENTIFIER", tokens[*token_number]->line_num);
            }
            (*token_number)++;
        }
        (*token_number)++;
    }
    return *token_number;
}

int generate_if_operation_nodes_right(Token** tokens, int* token_number, Node* current_node) {
    Node* oper_node = create_Node(tokens[*token_number]->word, OPERATOR);
    current_node->left->right = oper_node;
    current_node = oper_node;
    (*token_number)--;

    Node* expr_node = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
    current_node->left = expr_node;
    (*token_number) += 2;

    while (tokens[*token_number]->type == INT || tokens[*token_number]->type == IDENTIFIER ||
           tokens[*token_number]->type == OPERATOR) {
        // if ((tokens[*token_number]->type != INT && tokens[*token_number]->type != IDENTIFIER) &&
        //     tokens[*token_number] == NULL) {
        //     print_error("Invalid token", tokens[*token_number]->line_num);
        // }
        if (tokens[*token_number + 1]->type != OPERATOR ||
            strcmp(tokens[*token_number + 1]->word, "=") == 0) {
            if (tokens[*token_number]->type == INT) {
                Node* second_expr_node = create_Node(tokens[*token_number]->word, INT);
                current_node->right = second_expr_node;
            } else if (tokens[*token_number]->type == IDENTIFIER) {
                Node* second_identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
                current_node->right = second_identifier_node;
            } else {
                print_error("Expected Integer or identifier ", tokens[*token_number]->line_num);
            }
        }
        if (strcmp(tokens[*token_number]->word, "=") == 0) {
            break;
        } else if (tokens[*token_number]->type == OPERATOR) {
            Node* next_oper_node = create_Node(tokens[*token_number]->word, OPERATOR);
            current_node->right = next_oper_node;
            (*token_number)--;
            if (tokens[*token_number]->type == INT) {
                Node* second_expr_node = create_Node(tokens[*token_number]->word, INT);
                current_node->left = second_expr_node;
            } else if (tokens[*token_number]->type == IDENTIFIER) {
                Node* second_identifier_node = create_Node(tokens[*token_number]->word, IDENTIFIER);
                current_node->left = second_identifier_node;
            } else {
                print_error("Expected INT or IDENTIFIER", tokens[*token_number]->line_num);
            }
            (*token_number)++;
        }
        (*token_number)++;
    }
    return *token_number;
}

Node* create_if_statement(Token** tokens, int* token_number, Node* current_node) {
    Node* if_node = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
    current_node->left = if_node;
    current_node = if_node;
    (*token_number)++;

    expect(tokens, *token_number, SEPARATOR, "(");
    Node* open_paren_node = create_Node(tokens[*token_number]->word, SEPARATOR);
    current_node->left = open_paren_node;
    current_node = open_paren_node;

    (*token_number)++;
    if (tokens[*token_number]->type != IDENTIFIER && tokens[*token_number]->type != INT) {
        print_error("Expected Identifier or INT", tokens[*token_number]->line_num);
    }

    while (tokens[*token_number]->type != END_TOKEN && tokens[*token_number]->type != COMP) {
        (*token_number)++;
    }

    expect(tokens, *token_number, COMP, tokens[*token_number]->word);
    Node* comp_node = create_Node(tokens[*token_number]->word, COMP);
    open_paren_node->left = comp_node;

    while (current_node->type != SEPARATOR) {
        (*token_number)--;
    }

    (*token_number) += 2;
    if (tokens[*token_number]->type != OPERATOR) {
        (*token_number)--;
        Node* expr_node = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
        comp_node->left = expr_node;
    } else {
        *token_number = generate_if_operation_nodes(tokens, token_number, current_node);
    }

    (*token_number)++;
    while ((tokens[*token_number]->type != END_TOKEN && tokens[*token_number]->type != OPERATOR &&
            tokens[*token_number]->type != SEPARATOR) ||
           strcmp(tokens[*token_number]->word, "=") == 0) {
        (*token_number)++;
    }
    if (tokens[*token_number]->type == SEPARATOR) {
        (*token_number)--;
        Node* expr_node = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
        comp_node->right = expr_node;
    } else {
        *token_number = generate_if_operation_nodes_right(tokens, token_number, current_node);
    }

    expect(tokens, *token_number, SEPARATOR, ")");
    Node* close_paren_node = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
    open_paren_node->right = close_paren_node;
    current_node = close_paren_node;
    return current_node;
}

Node* handle_write_node(Token** tokens, int* token_number, Node* current_node) {
    Node* write_node = NULL;
    write_node = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
    current_node->left = write_node;
    current_node = write_node;

    (*token_number)++;

    expect(tokens, *token_number, SEPARATOR, "(");

    (*token_number)++;
    // if (tokens[*token_number]->type != STRING && tokens[*token_number]->type != IDENTIFIER &&
    //     tokens[*token_number]->type != INT) {
    //     print_error("Expected String or variable or int", tokens[*token_number]->line_num);
    // }
    expect(tokens, *token_number, STRING, tokens[*token_number]->word);
    Node* string_node = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
    current_node->left = string_node;
    (*token_number)++;

    expect(tokens, *token_number, SEPARATOR, ",");
    (*token_number)++;

    Node* number_node = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
    current_node->right = number_node;
    (*token_number)++;

    expect(tokens, *token_number, SEPARATOR, ")");
    (*token_number)++;

    expect(tokens, *token_number, SEPARATOR, ";");
    Node* semi_node = create_Node(tokens[*token_number]->word, tokens[*token_number]->type);
    number_node->right = semi_node;
    current_node = semi_node;
    return current_node;
}

Node* parser(Token** tokens) {
    Node* root = create_Node("PROGRAM", BEGINNING);
    Node* current_node = root;

    Node* open_scope = malloc(sizeof(Node));
    scope_stack* stack = malloc(sizeof(scope_stack));

    int i = 0;
    while (tokens[i] && tokens[i]->type != END_TOKEN) {
        if (current_node == NULL)
            break;

        switch (tokens[i]->type) {
            case INT:
                break;
            case KEYWORD:
                if (!strcmp(tokens[i]->word, "exit")) {
                    current_node = parse_exit(tokens, &i, root);
                } else if (!strcmp(tokens[i]->word, "int")) {
                    current_node = create_variable(tokens, &i, current_node);
                } else if (!strcmp(tokens[i]->word, "if")) {
                    current_node = create_if_statement(tokens, &i, current_node);
                } else if (!strcmp(tokens[i]->word, "while")) {
                    current_node = create_if_statement(tokens, &i, current_node);
                } else if (!strcmp(tokens[i]->word, "write")) {
                    current_node = handle_write_node(tokens, &i, current_node);
                }
                break;
            case SEPARATOR:
                if (strcmp(tokens[i]->word, "{") == 0) {
                    open_scope = create_Node(tokens[i]->word, SEPARATOR);
                    current_node->left = open_scope;
                    current_node = open_scope;
                    push_scope(stack, open_scope);
                    current_node = peek_scope(stack);
                }
                if (strcmp(tokens[i]->word, "}") == 0) {
                    Node* close_scope = create_Node(tokens[i]->word, tokens[i]->type);
                    open_scope = pop_scope(stack);
                    if (open_scope == NULL) {
                        print_error("Closed brace does not have opening brace",
                                    tokens[i]->line_num);
                    }
                    current_node->right = close_scope;
                    current_node = close_scope;
                }
                break;
            case OPERATOR:
                break;
            case END_TOKEN:
                break;
            case BEGINNING:
                break;
            case IDENTIFIER:
                i--;
                if (tokens[i]->type == SEPARATOR &&
                    ((strcmp(tokens[i]->word, ";") == 0) || (strcmp(tokens[i]->word, "}") == 0) ||
                     (strcmp(tokens[i]->word, "{") == 0))) {
                    current_node = create_var_reuseable(tokens, &i, current_node);
                } else {
                    i++;
                }
                break;
            case STRING:
                break;
            case COMP:
                break;
        }
        i++;
    }
    print_tree(root, 0);
    return root;
}
