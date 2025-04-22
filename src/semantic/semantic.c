#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

bool analyze_semantics(ASTNode *ast_root) {
    if (!ast_root || ast_root->type != AST_PROGRAM) {
        fprintf(stderr, "Erro semântico: AST inválida ou nó raiz não é um programa\n");
        return false;
    }
    
    SymbolTable *symbol_table = create_symbol_table(101);
    
    bool result = check_declarations(symbol_table, ast_root->data.program.declarations);
    
    free_symbol_table(symbol_table);
    
    return result;
}

bool check_declarations(SymbolTable *st, ASTNode *declarations) {
    if (!declarations) {
        return true;
    }
    
    ASTNode *current = declarations;
    bool result = true;
    
    while (current && current->type == AST_DECLARATIONS) {
        if (current->data.declarations.declaration) {
            ASTNode *decl = current->data.declarations.declaration;
            
            switch (decl->type) {
                case AST_VAR_DECLARATION:
                    result &= check_var_declaration(st, decl);
                    break;
                
                case AST_FUNCTION_DEF:
                    result &= check_function_def(st, decl);
                    break;
                
                case AST_CONDITIONAL:
                    result &= check_conditional(st, decl);
                    break;
                
                case AST_FOR_LOOP:
                    result &= check_for_loop(st, decl);
                    break;
                
                case AST_WHILE_LOOP:
                    result &= check_while_loop(st, decl);
                    break;
                
                case AST_FUNCTION_CALL: {
                    TypeInfo type_info = check_function_call(st, decl);
                    result &= type_info.is_valid;
                    break;
                }
                
                case AST_LOG: {
                    TypeInfo type_info = check_expression(st, decl->data.log.expression);
                    result &= type_info.is_valid;
                    break;
                }
                
                default:
                    fprintf(stderr, "Erro semântico: Tipo de declaração desconhecido\n");
                    result = false;
            }
        }
        
        current = current->data.declarations.next;
    }
    
    return result;
}

bool check_var_declaration(SymbolTable *st, ASTNode *var_decl) {
    if (!var_decl || var_decl->type != AST_VAR_DECLARATION) {
        return false;
    }
    
    char *type_name = var_decl->data.var_declaration.type->data.type.name;
    DataType data_type = string_to_data_type(type_name);
    
    TypeInfo expr_type = check_expression(st, var_decl->data.var_declaration.value);
    if (!expr_type.is_valid) {
        return false;
    }
    
    if (data_type != expr_type.type && !(data_type == TYPE_BYTE && expr_type.type == TYPE_BIT)) {
        fprintf(stderr, "Erro semântico: Tipos incompatíveis na atribuição para '%s'\n", 
                var_decl->data.var_declaration.name);
        return false;
    }
    
    if (!insert_symbol(st, var_decl->data.var_declaration.name, SYM_VARIABLE, data_type)) {
        fprintf(stderr, "Erro semântico: Variável '%s' já definida no escopo atual\n", 
                var_decl->data.var_declaration.name);
        return false;
    }
    
    return true;
}

bool check_function_def(SymbolTable *st, ASTNode *func_def) {
    if (!func_def || func_def->type != AST_FUNCTION_DEF) {
        return false;
    }
    
    int param_count = 0;
    DataType *param_types = NULL;
    
    if (func_def->data.function_def.params) {
        ASTNode *current = func_def->data.function_def.params;
        while (current && current->type == AST_PARAMS) {
            if (current->data.params.name) {
                param_count++;
            }
            current = current->data.params.next;
        }
        
        param_types = (DataType *)malloc(param_count * sizeof(DataType));
        for (int i = 0; i < param_count; i++) {
            param_types[i] = TYPE_BYTE;
        }
    }
    
    DataType return_type = TYPE_VOID;
    if (func_def->data.function_def.return_stmt) {
        return_type = TYPE_BYTE;
    }
    
    if (!insert_function(st, func_def->data.function_def.name, param_count, param_types, return_type)) {
        fprintf(stderr, "Erro semântico: Função '%s' já definida no escopo atual\n", 
                func_def->data.function_def.name);
        free(param_types);
        return false;
    }
    
    if (param_types) {
        free(param_types);
    }
    
    enter_scope(st);
    
    if (func_def->data.function_def.params) {
        ASTNode *current = func_def->data.function_def.params;
        while (current && current->type == AST_PARAMS) {
            if (current->data.params.name) {
                if (!insert_symbol(st, current->data.params.name, SYM_PARAMETER, TYPE_BYTE)) {
                    fprintf(stderr, "Erro semântico: Parâmetro '%s' duplicado na função '%s'\n", 
                            current->data.params.name, func_def->data.function_def.name);
                    leave_scope(st);
                    return false;
                }
            }
            current = current->data.params.next;
        }
    }
    
    bool body_valid = check_declarations(st, func_def->data.function_def.body);
    
    if (func_def->data.function_def.return_stmt) {
        TypeInfo return_expr_type = check_expression(st, func_def->data.function_def.return_stmt->data.return_stmt.expression);
        if (!return_expr_type.is_valid || return_expr_type.type != return_type) {
            fprintf(stderr, "Erro semântico: Tipo de retorno incompatível na função '%s'\n", 
                    func_def->data.function_def.name);
            body_valid = false;
        }
    }
    
    leave_scope(st);
    
    return body_valid;
}

bool check_conditional(SymbolTable *st, ASTNode *conditional) {
    if (!conditional || conditional->type != AST_CONDITIONAL) {
        return false;
    }
    
    ASTNode *blocks = conditional->data.conditional.blocks;
    bool result = true;
    
    while (blocks && blocks->type == AST_IF_BLOCKS) {
        if (blocks->data.if_blocks.if_block) {
            TypeInfo cond_type = check_expression(st, blocks->data.if_blocks.if_block->data.if_block.condition);
            if (!cond_type.is_valid) {
                result = false;
            } else if (cond_type.type != TYPE_BIT) {
                fprintf(stderr, "Erro semântico: Condição do if deve ser do tipo 'bit'\n");
                result = false;
            }
            
            enter_scope(st);
            result &= check_declarations(st, blocks->data.if_blocks.if_block->data.if_block.body);
            leave_scope(st);
        }
        
        if (blocks->data.if_blocks.else_block) {
            enter_scope(st);
            result &= check_declarations(st, blocks->data.if_blocks.else_block->data.else_block.body);
            leave_scope(st);
        }
        
        blocks = blocks->data.if_blocks.next;
    }
    
    return result;
}

bool check_for_loop(SymbolTable *st, ASTNode *for_loop) {
    if (!for_loop || for_loop->type != AST_FOR_LOOP) {
        return false;
    }
    
    TypeInfo expr_type = check_expression(st, for_loop->data.for_loop.expression);
    if (!expr_type.is_valid) {
        return false;
    }
    
    if (expr_type.type != TYPE_ARRAY && expr_type.type != TYPE_BYTE) {
        fprintf(stderr, "Erro semântico: Expressão do loop 'for' deve ser um array ou um número\n");
        return false;
    }
    
    enter_scope(st);
    
    insert_symbol(st, for_loop->data.for_loop.iterator, SYM_VARIABLE, 
                  (expr_type.type == TYPE_ARRAY) ? TYPE_BYTE : TYPE_BYTE);
    
    bool body_valid = check_declarations(st, for_loop->data.for_loop.body);
    
    leave_scope(st);
    
    return body_valid;
}

bool check_while_loop(SymbolTable *st, ASTNode *while_loop) {
    if (!while_loop || while_loop->type != AST_WHILE_LOOP) {
        return false;
    }
    
    TypeInfo cond_type = check_expression(st, while_loop->data.while_loop.condition);
    if (!cond_type.is_valid) {
        return false;
    }
    
    if (cond_type.type != TYPE_BIT) {
        fprintf(stderr, "Erro semântico: Condição do loop 'while' deve ser do tipo 'bit'\n");
        return false;
    }
    
    enter_scope(st);
    
    bool body_valid = check_declarations(st, while_loop->data.while_loop.body);
    
    leave_scope(st);
    
    return body_valid;
}

TypeInfo check_function_call(SymbolTable *st, ASTNode *func_call) {
    TypeInfo result = {TYPE_VOID, false};
    
    if (!func_call || func_call->type != AST_FUNCTION_CALL) {
        return result;
    }
    
    Symbol *func = lookup_symbol(st, func_call->data.function_call.name);
    if (!func || func->sym_type != SYM_FUNCTION) {
        fprintf(stderr, "Erro semântico: Função '%s' não definida\n", func_call->data.function_call.name);
        return result;
    }
    
    int arg_count = 0;
    ASTNode *current = func_call->data.function_call.args;
    while (current && current->type == AST_ARGS) {
        TypeInfo arg_type = check_expression(st, current->data.args.arg);
        if (!arg_type.is_valid) {
            return result;
        }
        
        arg_count++;
        current = current->data.args.next;
    }
    
    if (arg_count != func->function.param_count) {
        fprintf(stderr, "Erro semântico: Número incorreto de argumentos na chamada de '%s'. Esperado %d, recebido %d\n", 
                func_call->data.function_call.name, func->function.param_count, arg_count);
        return result;
    }
    
    result.type = func->function.return_type;
    result.is_valid = true;
    
    return result;
}

TypeInfo check_expression(SymbolTable *st, ASTNode *expr) {
    TypeInfo result = {TYPE_VOID, false};
    
    if (!expr) {
        return result;
    }
    
    switch (expr->type) {
        case AST_IDENTIFIER: {
            Symbol *sym = lookup_symbol(st, expr->data.identifier.name);
            if (!sym) {
                fprintf(stderr, "Erro semântico: Variável '%s' não definida\n", expr->data.identifier.name);
                return result;
            }
            
            result.type = sym->data_type;
            result.is_valid = true;
            break;
        }
        
        case AST_NUMBER:
            result.type = TYPE_BYTE;
            result.is_valid = true;
            break;
        
        case AST_STRING:
            result.type = TYPE_CHIP;
            result.is_valid = true;
            break;
        
        case AST_ARRAY:
            result.type = TYPE_ARRAY;
            result.is_valid = true;
            
            if (expr->data.array.elements) {
                ASTNode *current = expr->data.array.elements;
                while (current && current->type == AST_ARGS) {
                    TypeInfo elem_type = check_expression(st, current->data.args.arg);
                    if (!elem_type.is_valid) {
                        result.is_valid = false;
                        return result;
                    }
                    current = current->data.args.next;
                }
            }
            break;
        
        case AST_FUNCTION_CALL:
            result = check_function_call(st, expr);
            break;
        
        case AST_BINARY_EXPR: {
            TypeInfo left_type = check_expression(st, expr->data.binary_expr.left);
            TypeInfo right_type = check_expression(st, expr->data.binary_expr.right);
            
            if (!left_type.is_valid || !right_type.is_valid) {
                return result;
            }
            
            OperatorType op = expr->data.binary_expr.op->data.op.op_type;
            
            if (op == OP_PLUS || op == OP_MINUS || op == OP_MULTIPLY || op == OP_DIVIDE) {
                if (left_type.type != TYPE_BYTE || right_type.type != TYPE_BYTE) {
                    fprintf(stderr, "Erro semântico: Operadores aritméticos requerem operandos do tipo 'byte'\n");
                    return result;
                }
                result.type = TYPE_BYTE;
            }
            else if (op == OP_EQUAL || op == OP_NOT_EQUAL) {
                if (left_type.type != right_type.type) {
                    fprintf(stderr, "Erro semântico: Comparação entre tipos diferentes\n");
                    return result;
                }
                result.type = TYPE_BIT;
            }
            else if (op == OP_GREATER || op == OP_LESS || op == OP_GREATER_EQUAL || op == OP_LESS_EQUAL) {
                if (left_type.type != TYPE_BYTE || right_type.type != TYPE_BYTE) {
                    fprintf(stderr, "Erro semântico: Operadores relacionais requerem operandos do tipo 'byte'\n");
                    return result;
                }
                result.type = TYPE_BIT;
            }
            else if (op == OP_AND || op == OP_OR) {
                if (left_type.type != TYPE_BIT || right_type.type != TYPE_BIT) {
                    fprintf(stderr, "Erro semântico: Operadores lógicos requerem operandos do tipo 'bit'\n");
                    return result;
                }
                result.type = TYPE_BIT;
            }
            
            result.is_valid = true;
            break;
        }
        
        default:
            fprintf(stderr, "Erro semântico: Tipo de expressão desconhecido\n");
            return result;
    }
    
    return result;
}