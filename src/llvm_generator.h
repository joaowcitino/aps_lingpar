#ifndef LLVM_GENERATOR_H
#define LLVM_GENERATOR_H

#include <stdbool.h>

typedef enum {
    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_REPEAT,
    NODE_SWITCH,
    NODE_CASE,
    NODE_PRINT,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_INT_VAL,
    NODE_STRING_VAL,
    NODE_BOOL_VAL,
    NODE_IDENTIFIER
} NodeType;

typedef struct Node {
    NodeType type;
    union {
        int int_value;
        char* str_value;
        int bool_value;
        struct {
            char* operator;
            struct Node* left;
            struct Node* right;
        } binary_op;
        struct {
            char* operator;
            struct Node* operand;
        } unary_op;
        struct {
            char* name;
            char* data_type;
            struct Node* init_expr;
        } var_decl;
        struct {
            char* name;
            struct Node* value;
        } assign;
        struct {
            struct Node* condition;
            struct Node* then_branch;
            struct Node* else_branch;
        } if_stmt;
        struct {
            struct Node* condition;
            struct Node* body;
        } while_stmt;
        struct {
            struct Node* body;
            struct Node* condition;
        } repeat_stmt;
        struct {
            struct Node* expr;
        } print_stmt;
        struct {
            struct Node* condition;
            struct Node** cases;
            int case_count;
            struct Node* default_case;
        } switch_stmt;
        struct {
            struct Node* value;
            struct Node* body;
        } case_stmt;
        struct {
            struct Node** statements;
            int stmt_count;
        } block;
        struct {
            struct Node* body;
        } program;
    } data;
    struct Node* next;
} Node;

void generate_llvm_code(Node* ast_root, const char* output_file);

#endif