#ifndef AST_H
#define AST_H

typedef enum {
    AST_PROGRAM,
    AST_DECLARATIONS,
    AST_VAR_DECLARATION,
    AST_TYPE,
    AST_CONDITIONAL,
    AST_IF_BLOCKS,
    AST_IF_BLOCK,
    AST_ELSE_BLOCK,
    AST_FOR_LOOP,
    AST_WHILE_LOOP,
    AST_FUNCTION_DEF,
    AST_PARAMS,
    AST_RETURN,
    AST_FUNCTION_CALL,
    AST_ARGS,
    AST_BINARY_EXPR,
    AST_IDENTIFIER,
    AST_OPERATOR,
    AST_LOG,
    AST_NUMBER,
    AST_STRING,
    AST_ARRAY
} ASTNodeType;

typedef enum {
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_GREATER_EQUAL,
    OP_LESS_EQUAL,
    OP_AND,
    OP_OR,
    OP_ASSIGN
} OperatorType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        struct {
            char *name;
            char *end_name;
            struct ASTNode *declarations;
        } program;
        
        struct {
            struct ASTNode *declaration;
            struct ASTNode *next;
        } declarations;
        
        struct {
            struct ASTNode *type;
            char *name;
            struct ASTNode *value;
        } var_declaration;
        
        struct {
            char *name;
        } type;
        
        struct {
            struct ASTNode *blocks;
        } conditional;
        
        struct {
            struct ASTNode *if_block;
            struct ASTNode *next;
            struct ASTNode *else_block;
        } if_blocks;
        
        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } if_block;
        
        struct {
            struct ASTNode *body;
        } else_block;
        
        struct {
            struct ASTNode *expression;
            char *iterator;
            struct ASTNode *body;
        } for_loop;
        
        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_loop;
        
        struct {
            char *name;
            struct ASTNode *params;
            struct ASTNode *body;
            struct ASTNode *return_stmt;
        } function_def;
        
        struct {
            char *name;
            struct ASTNode *next;
        } params;
        
        struct {
            struct ASTNode *expression;
        } return_stmt;
        
        struct {
            char *name;
            struct ASTNode *args;
        } function_call;
        
        struct {
            struct ASTNode *arg;
            struct ASTNode *next;
        } args;
        
        struct {
            struct ASTNode *op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binary_expr;
        
        struct {
            char *name;
        } identifier;
        
        struct {
            OperatorType op_type;
        } op;
        
        struct {
            struct ASTNode *expression;
        } log;
        
        struct {
            int value;
        } number;
        
        struct {
            char *value;
        } string;
        
        struct {
            struct ASTNode *elements;
        } array;
    } data;
} ASTNode;

ASTNode *create_program_node(char *name, ASTNode *declarations, char *end_name);
ASTNode *create_declarations_node(ASTNode *declaration, ASTNode *next);
ASTNode *add_declaration(ASTNode *declarations, ASTNode *declaration);
ASTNode *create_var_declaration_node(ASTNode *type, char *name, ASTNode *value);
ASTNode *create_type_node(char *name);
ASTNode *create_conditional_node(ASTNode *blocks);
ASTNode *create_if_blocks_node(ASTNode *if_block, ASTNode *else_block);
ASTNode *add_if_block(ASTNode *blocks, ASTNode *new_block);
ASTNode *add_else_block(ASTNode *blocks, ASTNode *else_block);
ASTNode *create_if_block_node(ASTNode *condition, ASTNode *body);
ASTNode *create_else_block_node(ASTNode *body);
ASTNode *create_for_loop_node(ASTNode *expression, char *iterator, ASTNode *body);
ASTNode *create_while_loop_node(ASTNode *condition, ASTNode *body);
ASTNode *create_function_def_node(char *name, ASTNode *params, ASTNode *body, ASTNode *return_stmt);
ASTNode *create_params_node(char *name);
ASTNode *add_param(ASTNode *params, char *name);
ASTNode *create_return_node(ASTNode *expression);
ASTNode *create_function_call_node(char *name, ASTNode *args);
ASTNode *create_args_node(ASTNode *arg);
ASTNode *add_arg(ASTNode *args, ASTNode *arg);
ASTNode *add_args(ASTNode *args1, ASTNode *args2);
ASTNode *create_binary_expr_node(ASTNode *op, ASTNode *left, ASTNode *right);
ASTNode *create_identifier_node(char *name);
ASTNode *create_operator_node(OperatorType op_type);
ASTNode *create_log_node(ASTNode *expression);
ASTNode *create_number_node(int value);
ASTNode *create_string_node(char *value);
ASTNode *create_array_node(ASTNode *elements);

void free_ast(ASTNode *node);

#endif