#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* create_int_literal(int value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_LITERAL_INT;
    node->data.int_value = value;
    return node;
}

ASTNode* create_float_literal(double value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_LITERAL_FLOAT;
    node->data.float_value = value;
    return node;
}

ASTNode* create_string_literal(char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_LITERAL_STRING;
    node->data.string_value = strdup(value);
    return node;
}

ASTNode* create_variable(char* name, int var_type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_VARIABLE;
    node->data.variable.name = strdup(name);
    node->data.variable.var_type = var_type;
    return node;
}

ASTNode* create_binary_op(int op, ASTNode* left, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_BINARY_OP;
    node->data.binary_op.op = op;
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    return node;
}

ASTNode* create_unary_op(int op, ASTNode* operand) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_UNARY_OP;
    node->data.unary_op.op = op;
    node->data.unary_op.operand = operand;
    return node;
}

ASTNode* create_assignment(char* name, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_ASSIGNMENT;
    node->data.assignment.name = strdup(name);
    node->data.assignment.value = value;
    return node;
}

ASTNode* create_declaration(int var_type, char* name, ASTNode* init_value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_DECLARATION;
    node->data.declaration.var_type = var_type;
    node->data.declaration.name = strdup(name);
    node->data.declaration.init_value = init_value;
    return node;
}

ASTNode* create_if_else(ASTNode* condition, ASTNode* then_block, ASTNode* else_block) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_IF_ELSE;
    node->data.if_else.condition = condition;
    node->data.if_else.then_block = then_block;
    node->data.if_else.else_block = else_block;
    return node;
}

ASTNode* create_while_loop(ASTNode* condition, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_WHILE_LOOP;
    node->data.while_loop.condition = condition;
    node->data.while_loop.body = body;
    return node;
}

ASTNode* create_for_loop(ASTNode* init, ASTNode* condition, ASTNode* update, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_FOR_LOOP;
    node->data.for_loop.init = init;
    node->data.for_loop.condition = condition;
    node->data.for_loop.update = update;
    node->data.for_loop.body = body;
    return node;
}

ASTNode* create_print(ASTNode* expression, int is_debug) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = is_debug ? AST_NARRATE : AST_PRINT;
    node->data.print.expression = expression;
    node->data.print.is_debug = is_debug;
    return node;
}

ASTNode* create_block(ASTNode** statements, int count) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_BLOCK;
    node->data.block.statements = malloc(count * sizeof(ASTNode*));
    memcpy(node->data.block.statements, statements, count * sizeof(ASTNode*));
    node->data.block.count = count;
    return node;
}

ASTNode* create_scene(char* name, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_SCENE;
    node->data.scene.name = strdup(name);
    node->data.scene.body = body;
    return node;
}

ASTNode* create_enter_exit(char* resource_name, int is_enter) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_ENTER_EXIT;
    node->data.enter_exit.resource_name = strdup(resource_name);
    node->data.enter_exit.is_enter = is_enter;
    return node;
}

ASTNode* create_improvise(ASTNode* max_value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_IMPROVISE;
    node->data.improvise.max_value = max_value;
    return node;
}

void free_ast_node(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_LITERAL_STRING:
            free(node->data.string_value);
            break;
        case AST_VARIABLE:
            free(node->data.variable.name);
            break;
        case AST_BINARY_OP:
            free_ast_node(node->data.binary_op.left);
            free_ast_node(node->data.binary_op.right);
            break;
        case AST_UNARY_OP:
            free_ast_node(node->data.unary_op.operand);
            break;
        case AST_ASSIGNMENT:
            free(node->data.assignment.name);
            free_ast_node(node->data.assignment.value);
            break;
        case AST_DECLARATION:
            free(node->data.declaration.name);
            if (node->data.declaration.init_value)
                free_ast_node(node->data.declaration.init_value);
            break;
        case AST_IF_ELSE:
            free_ast_node(node->data.if_else.condition);
            free_ast_node(node->data.if_else.then_block);
            if (node->data.if_else.else_block)
                free_ast_node(node->data.if_else.else_block);
            break;
        case AST_WHILE_LOOP:
            free_ast_node(node->data.while_loop.condition);
            free_ast_node(node->data.while_loop.body);
            break;
        case AST_FOR_LOOP:
            free_ast_node(node->data.for_loop.init);
            free_ast_node(node->data.for_loop.condition);
            free_ast_node(node->data.for_loop.update);
            free_ast_node(node->data.for_loop.body);
            break;
        case AST_PRINT:
        case AST_NARRATE:
            free_ast_node(node->data.print.expression);
            break;
        case AST_BLOCK:
            for (int i = 0; i < node->data.block.count; i++) {
                free_ast_node(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;
        case AST_SCENE:
            free(node->data.scene.name);
            free_ast_node(node->data.scene.body);
            break;
        case AST_ENTER_EXIT:
            free(node->data.enter_exit.resource_name);
            break;
        case AST_IMPROVISE:
            free_ast_node(node->data.improvise.max_value);
            break;
        default:
            break;
    }
    
    free(node);
}