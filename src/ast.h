#ifndef AST_H
#define AST_H

typedef enum {
    AST_LITERAL_INT,
    AST_LITERAL_FLOAT,
    AST_LITERAL_STRING,
    AST_VARIABLE,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_ASSIGNMENT,
    AST_DECLARATION,
    AST_IF_ELSE,
    AST_WHILE_LOOP,
    AST_FOR_LOOP,
    AST_PRINT,
    AST_NARRATE,
    AST_BLOCK,
    AST_SCENE,
    AST_ENTER_EXIT,
    AST_IMPROVISE
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        int int_value;
        double float_value;
        char* string_value;
        
        struct {
            char* name;
            int var_type;
        } variable;
        
        struct {
            int op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;
        
        struct {
            int op;
            struct ASTNode* operand;
        } unary_op;
        
        struct {
            char* name;
            struct ASTNode* value;
        } assignment;
        
        struct {
            int var_type;
            char* name;
            struct ASTNode* init_value;
        } declaration;
        
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_block;
            struct ASTNode* else_block;
        } if_else;
        
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_loop;
        
        struct {
            struct ASTNode* init;
            struct ASTNode* condition;
            struct ASTNode* update;
            struct ASTNode* body;
        } for_loop;
        
        struct {
            struct ASTNode* expression;
            int is_debug;
        } print;
        
        struct {
            struct ASTNode** statements;
            int count;
        } block;
        
        struct {
            char* name;
            struct ASTNode* body;
        } scene;
        
        struct {
            char* resource_name;
            int is_enter;
        } enter_exit;
        
        struct {
            struct ASTNode* max_value;
        } improvise;
    } data;
} ASTNode;

ASTNode* create_int_literal(int value);
ASTNode* create_float_literal(double value);
ASTNode* create_string_literal(char* value);
ASTNode* create_variable(char* name, int var_type);
ASTNode* create_binary_op(int op, ASTNode* left, ASTNode* right);
ASTNode* create_unary_op(int op, ASTNode* operand);
ASTNode* create_assignment(char* name, ASTNode* value);
ASTNode* create_declaration(int var_type, char* name, ASTNode* init_value);
ASTNode* create_if_else(ASTNode* condition, ASTNode* then_block, ASTNode* else_block);
ASTNode* create_while_loop(ASTNode* condition, ASTNode* body);
ASTNode* create_for_loop(ASTNode* init, ASTNode* condition, ASTNode* update, ASTNode* body);
ASTNode* create_print(ASTNode* expression, int is_debug);
ASTNode* create_block(ASTNode** statements, int count);
ASTNode* create_scene(char* name, ASTNode* body);
ASTNode* create_enter_exit(char* resource_name, int is_enter);
ASTNode* create_improvise(ASTNode* max_value);
void free_ast_node(ASTNode* node);

#endif