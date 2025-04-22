#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static ASTNode *create_node(ASTNodeType type) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "Erro: Falha ao alocar memória para nó AST\n");
        exit(1);
    }
    node->type = type;
    return node;
}

ASTNode *create_program_node(char *name, ASTNode *declarations, char *end_name) {
    ASTNode *node = create_node(AST_PROGRAM);
    node->data.program.name = strdup(name);
    node->data.program.declarations = declarations;
    node->data.program.end_name = strdup(end_name);
    return node;
}

ASTNode *create_declarations_node(ASTNode *declaration, ASTNode *next) {
    ASTNode *node = create_node(AST_DECLARATIONS);
    node->data.declarations.declaration = declaration;
    node->data.declarations.next = next;
    return node;
}

ASTNode *add_declaration(ASTNode *declarations, ASTNode *declaration) {
    if (declarations == NULL) {
        return create_declarations_node(declaration, NULL);
    }
    
    ASTNode *current = declarations;
    while (current->data.declarations.next != NULL) {
        current = current->data.declarations.next;
    }
    
    current->data.declarations.next = create_declarations_node(declaration, NULL);
    return declarations;
}

ASTNode *create_var_declaration_node(ASTNode *type, char *name, ASTNode *value) {
    ASTNode *node = create_node(AST_VAR_DECLARATION);
    node->data.var_declaration.type = type;
    node->data.var_declaration.name = strdup(name);
    node->data.var_declaration.value = value;
    return node;
}

ASTNode *create_type_node(char *name) {
    ASTNode *node = create_node(AST_TYPE);
    node->data.type.name = strdup(name);
    return node;
}

ASTNode *create_conditional_node(ASTNode *blocks) {
    ASTNode *node = create_node(AST_CONDITIONAL);
    node->data.conditional.blocks = blocks;
    return node;
}

ASTNode *create_if_blocks_node(ASTNode *if_block, ASTNode *else_block) {
    ASTNode *node = create_node(AST_IF_BLOCKS);
    node->data.if_blocks.if_block = if_block;
    node->data.if_blocks.next = NULL;
    node->data.if_blocks.else_block = else_block;
    return node;
}

ASTNode *add_if_block(ASTNode *blocks, ASTNode *new_block) {
    if (blocks == NULL) {
        return create_if_blocks_node(new_block, NULL);
    }
    
    ASTNode *current = blocks;
    while (current->data.if_blocks.next != NULL) {
        current = current->data.if_blocks.next;
    }
    
    current->data.if_blocks.next = create_if_blocks_node(new_block, NULL);
    return blocks;
}

ASTNode *add_else_block(ASTNode *blocks, ASTNode *else_block) {
    if (blocks == NULL) {
        return NULL;
    }
    
    ASTNode *current = blocks;
    while (current->data.if_blocks.next != NULL) {
        current = current->data.if_blocks.next;
    }
    
    current->data.if_blocks.else_block = else_block;
    return blocks;
}

ASTNode *create_if_block_node(ASTNode *condition, ASTNode *body) {
    ASTNode *node = create_node(AST_IF_BLOCK);
    node->data.if_block.condition = condition;
    node->data.if_block.body = body;
    return node;
}

ASTNode *create_else_block_node(ASTNode *body) {
    ASTNode *node = create_node(AST_ELSE_BLOCK);
    node->data.else_block.body = body;
    return node;
}

ASTNode *create_for_loop_node(ASTNode *expression, char *iterator, ASTNode *body) {
    ASTNode *node = create_node(AST_FOR_LOOP);
    node->data.for_loop.expression = expression;
    node->data.for_loop.iterator = strdup(iterator);
    node->data.for_loop.body = body;
    return node;
}

ASTNode *create_while_loop_node(ASTNode *condition, ASTNode *body) {
    ASTNode *node = create_node(AST_WHILE_LOOP);
    node->data.while_loop.condition = condition;
    node->data.while_loop.body = body;
    return node;
}

ASTNode *create_function_def_node(char *name, ASTNode *params, ASTNode *body, ASTNode *return_stmt) {
    ASTNode *node = create_node(AST_FUNCTION_DEF);
    node->data.function_def.name = strdup(name);
    node->data.function_def.params = params;
    node->data.function_def.body = body;
    node->data.function_def.return_stmt = return_stmt;
    return node;
}

ASTNode *create_params_node(char *name) {
    ASTNode *node = create_node(AST_PARAMS);
    node->data.params.name = name ? strdup(name) : NULL;
    node->data.params.next = NULL;
    return node;
}

ASTNode *add_param(ASTNode *params, char *name) {
    if (params == NULL) {
        return create_params_node(name);
    }
    
    ASTNode *current = params;
    while (current->data.params.next != NULL) {
        current = current->data.params.next;
    }
    
    current->data.params.next = create_params_node(name);
    return params;
}

ASTNode *create_return_node(ASTNode *expression) {
    ASTNode *node = create_node(AST_RETURN);
    node->data.return_stmt.expression = expression;
    return node;
}

ASTNode *create_function_call_node(char *name, ASTNode *args) {
    ASTNode *node = create_node(AST_FUNCTION_CALL);
    node->data.function_call.name = strdup(name);
    node->data.function_call.args = args;
    return node;
}

ASTNode *create_args_node(ASTNode *arg) {
    ASTNode *node = create_node(AST_ARGS);
    node->data.args.arg = arg;
    node->data.args.next = NULL;
    return node;
}

ASTNode *add_arg(ASTNode *args, ASTNode *arg) {
    if (args == NULL) {
        return create_args_node(arg);
    }
    
    ASTNode *current = args;
    while (current->data.args.next != NULL) {
        current = current->data.args.next;
    }
    
    current->data.args.next = create_args_node(arg);
    return args;
}

ASTNode *add_args(ASTNode *args1, ASTNode *args2) {
    if (args1 == NULL) {
        return args2;
    }
    
    ASTNode *current = args1;
    while (current->data.args.next != NULL) {
        current = current->data.args.next;
    }
    
    current->data.args.next = args2;
    return args1;
}

ASTNode *create_binary_expr_node(ASTNode *op, ASTNode *left, ASTNode *right) {
    ASTNode *node = create_node(AST_BINARY_EXPR);
    node->data.binary_expr.op = op;
    node->data.binary_expr.left = left;
    node->data.binary_expr.right = right;
    return node;
}

ASTNode *create_identifier_node(char *name) {
    ASTNode *node = create_node(AST_IDENTIFIER);
    node->data.identifier.name = strdup(name);
    return node;
}

ASTNode *create_operator_node(OperatorType op_type) {
    ASTNode *node = create_node(AST_OPERATOR);
    node->data.op.op_type = op_type;
    return node;
}

ASTNode *create_log_node(ASTNode *expression) {
    ASTNode *node = create_node(AST_LOG);
    node->data.log.expression = expression;
    return node;
}

ASTNode *create_number_node(int value) {
    ASTNode *node = create_node(AST_NUMBER);
    node->data.number.value = value;
    return node;
}

ASTNode *create_string_node(char *value) {
    ASTNode *node = create_node(AST_STRING);
    node->data.string.value = strdup(value);
    return node;
}

ASTNode *create_array_node(ASTNode *elements) {
    ASTNode *node = create_node(AST_ARRAY);
    node->data.array.elements = elements;
    return node;
}

void free_ast(ASTNode *node) {
    if (node == NULL) {
        return;
    }
    
    switch (node->type) {
        case AST_PROGRAM:
            free(node->data.program.name);
            free(node->data.program.end_name);
            free_ast(node->data.program.declarations);
            break;
            
        case AST_DECLARATIONS:
            free_ast(node->data.declarations.declaration);
            free_ast(node->data.declarations.next);
            break;
            
        case AST_VAR_DECLARATION:
            free(node->data.var_declaration.name);
            free_ast(node->data.var_declaration.type);
            free_ast(node->data.var_declaration.value);
            break;
            
        case AST_TYPE:
            free(node->data.type.name);
            break;
            
        case AST_CONDITIONAL:
            free_ast(node->data.conditional.blocks);
            break;
            
        case AST_IF_BLOCKS:
            free_ast(node->data.if_blocks.if_block);
            free_ast(node->data.if_blocks.next);
            free_ast(node->data.if_blocks.else_block);
            break;
            
        case AST_IF_BLOCK:
            free_ast(node->data.if_block.condition);
            free_ast(node->data.if_block.body);
            break;
            
        case AST_ELSE_BLOCK:
            free_ast(node->data.else_block.body);
            break;
            
        case AST_FOR_LOOP:
            free_ast(node->data.for_loop.expression);
            free(node->data.for_loop.iterator);
            free_ast(node->data.for_loop.body);
            break;
            
        case AST_WHILE_LOOP:
            free_ast(node->data.while_loop.condition);
            free_ast(node->data.while_loop.body);
            break;
            
        case AST_FUNCTION_DEF:
            free(node->data.function_def.name);
            free_ast(node->data.function_def.params);
            free_ast(node->data.function_def.body);
            free_ast(node->data.function_def.return_stmt);
            break;
            
        case AST_PARAMS:
            if (node->data.params.name) free(node->data.params.name);
            free_ast(node->data.params.next);
            break;
            
        case AST_RETURN:
            free_ast(node->data.return_stmt.expression);
            break;
            
        case AST_FUNCTION_CALL:
            free(node->data.function_call.name);
            free_ast(node->data.function_call.args);
            break;
            
        case AST_ARGS:
            free_ast(node->data.args.arg);
            free_ast(node->data.args.next);
            break;
            
        case AST_BINARY_EXPR:
            free_ast(node->data.binary_expr.op);
            free_ast(node->data.binary_expr.left);
            free_ast(node->data.binary_expr.right);
            break;
            
        case AST_IDENTIFIER:
            free(node->data.identifier.name);
            break;
            
        case AST_OPERATOR:
            break;
            
        case AST_LOG:
            free_ast(node->data.log.expression);
            break;
            
        case AST_NUMBER:
            break;
            
        case AST_STRING:
            free(node->data.string.value);
            break;
            
        case AST_ARRAY:
            free_ast(node->data.array.elements);
            break;
    }
    
    free(node);
}