#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "llvm_generator.h"

typedef enum {
    VAL_INT,
    VAL_STRING,
    VAL_BOOL
} ValueType;

typedef struct {
    ValueType type;
    union {
        int int_val;
        char* str_val;
        bool bool_val;
    } data;
} Value;

typedef struct Symbol {
    char* name;
    char* type;
    Value value;
    struct Symbol* next;
} Symbol;

typedef struct {
    Symbol* first;
} SymbolTable;

static SymbolTable* init_symbol_table() {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    table->first = NULL;
    return table;
}

static Symbol* get_symbol(SymbolTable* table, const char* name) {
    Symbol* current = table->first;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void set_symbol(SymbolTable* table, const char* name, const char* type, Value value) {
    Symbol* symbol = get_symbol(table, name);
    
    if (symbol != NULL) {
        if (strcmp(symbol->type, type) != 0) {
            fprintf(stderr, "Erro: Tipo incompatível para variável '%s'\n", name);
            exit(1);
        }
        
        if (symbol->value.type == VAL_STRING && symbol->value.data.str_val != NULL) {
            free(symbol->value.data.str_val);
        }
        
        symbol->value = value;
    } else {
        symbol = (Symbol*)malloc(sizeof(Symbol));
        symbol->name = strdup(name);
        symbol->type = strdup(type);
        symbol->value = value;
        
        symbol->next = table->first;
        table->first = symbol;
    }
}

static void free_symbol_table(SymbolTable* table) {
    Symbol* current = table->first;
    while (current != NULL) {
        Symbol* next = current->next;
        
        free(current->name);
        free(current->type);
        
        if (current->value.type == VAL_STRING && current->value.data.str_val != NULL) {
            free(current->value.data.str_val);
        }
        
        free(current);
        current = next;
    }
    
    free(table);
}

static Value create_int_value(int val) {
    Value value;
    value.type = VAL_INT;
    value.data.int_val = val;
    return value;
}

static Value create_string_value(const char* val) {
    Value value;
    value.type = VAL_STRING;
    value.data.str_val = strdup(val);
    return value;
}

static Value create_bool_value(bool val) {
    Value value;
    value.type = VAL_BOOL;
    value.data.bool_val = val;
    return value;
}

static char* value_to_string(Value value) {
    char buffer[256];
    
    switch (value.type) {
        case VAL_INT:
            sprintf(buffer, "%d", value.data.int_val);
            break;
        case VAL_STRING:
            return strdup(value.data.str_val);
        case VAL_BOOL:
            return strdup(value.data.bool_val ? "true" : "false");
        default:
            return strdup("");
    }
    
    return strdup(buffer);
}

static bool check_same_type(Value a, Value b) {
    return a.type == b.type;
}

static Value evaluate_expression(Node* node, SymbolTable* table);
static void execute_statement(Node* node, SymbolTable* table);

void execute_ast(Node* root) {
    if (root == NULL || root->type != NODE_PROGRAM) {
        fprintf(stderr, "Erro: Raiz da AST inválida\n");
        return;
    }
    
    SymbolTable* table = init_symbol_table();
    execute_statement(root->data.program.body, table);
    free_symbol_table(table);
}

static Value evaluate_expression(Node* node, SymbolTable* table) {
    if (node == NULL) {
        Value null_value;
        null_value.type = VAL_INT;
        null_value.data.int_val = 0;
        return null_value;
    }
    
    switch (node->type) {
        case NODE_INT_VAL: {
            return create_int_value(node->data.int_value);
        }
        
        case NODE_STRING_VAL: {
            return create_string_value(node->data.str_value);
        }
        
        case NODE_BOOL_VAL: {
            return create_bool_value(node->data.bool_value);
        }
        
        case NODE_IDENTIFIER: {
            Symbol* symbol = get_symbol(table, node->data.str_value);
            if (symbol == NULL) {
                fprintf(stderr, "Erro: Variável '%s' não definida\n", node->data.str_value);
                exit(1);
            }
            return symbol->value;
        }
        
        case NODE_BINARY_OP: {
            Value left = evaluate_expression(node->data.binary_op.left, table);
            Value right = evaluate_expression(node->data.binary_op.right, table);
            Value result;
            
            if (strcmp(node->data.binary_op.operator, "CONCAT") == 0) {
                char* left_str = value_to_string(left);
                char* right_str = value_to_string(right);
                
                char* result_str = (char*)malloc(strlen(left_str) + strlen(right_str) + 1);
                strcpy(result_str, left_str);
                strcat(result_str, right_str);
                
                free(left_str);
                free(right_str);
                
                result = create_string_value(result_str);
                free(result_str);
                return result;
            }
            
            if (!check_same_type(left, right)) {
                fprintf(stderr, "Erro: Operação com tipos incompatíveis\n");
                exit(1);
            }
            
            if (left.type == VAL_INT && right.type == VAL_INT) {
                if (strcmp(node->data.binary_op.operator, "+") == 0) {
                    return create_int_value(left.data.int_val + right.data.int_val);
                } else if (strcmp(node->data.binary_op.operator, "-") == 0) {
                    return create_int_value(left.data.int_val - right.data.int_val);
                } else if (strcmp(node->data.binary_op.operator, "*") == 0) {
                    return create_int_value(left.data.int_val * right.data.int_val);
                } else if (strcmp(node->data.binary_op.operator, "/") == 0) {
                    if (right.data.int_val == 0) {
                        fprintf(stderr, "Erro: Divisão por zero\n");
                        exit(1);
                    }
                    return create_int_value(left.data.int_val / right.data.int_val);
                } else if (strcmp(node->data.binary_op.operator, "%") == 0) {
                    if (right.data.int_val == 0) {
                        fprintf(stderr, "Erro: Módulo por zero\n");
                        exit(1);
                    }
                    return create_int_value(left.data.int_val % right.data.int_val);
                }
            }
            
            if (strcmp(node->data.binary_op.operator, "<") == 0) {
                if (left.type == VAL_INT && right.type == VAL_INT) {
                    return create_bool_value(left.data.int_val < right.data.int_val);
                } else if (left.type == VAL_STRING && right.type == VAL_STRING) {
                    return create_bool_value(strcmp(left.data.str_val, right.data.str_val) < 0);
                }
            } else if (strcmp(node->data.binary_op.operator, ">") == 0) {
                if (left.type == VAL_INT && right.type == VAL_INT) {
                    return create_bool_value(left.data.int_val > right.data.int_val);
                } else if (left.type == VAL_STRING && right.type == VAL_STRING) {
                    return create_bool_value(strcmp(left.data.str_val, right.data.str_val) > 0);
                }
            } else if (strcmp(node->data.binary_op.operator, "LE") == 0) {
                if (left.type == VAL_INT && right.type == VAL_INT) {
                    return create_bool_value(left.data.int_val <= right.data.int_val);
                } else if (left.type == VAL_STRING && right.type == VAL_STRING) {
                    return create_bool_value(strcmp(left.data.str_val, right.data.str_val) <= 0);
                }
            } else if (strcmp(node->data.binary_op.operator, "GE") == 0) {
                if (left.type == VAL_INT && right.type == VAL_INT) {
                    return create_bool_value(left.data.int_val >= right.data.int_val);
                } else if (left.type == VAL_STRING && right.type == VAL_STRING) {
                    return create_bool_value(strcmp(left.data.str_val, right.data.str_val) >= 0);
                }
            }
            
            if (strcmp(node->data.binary_op.operator, "EQ") == 0) {
                if (left.type == VAL_INT && right.type == VAL_INT) {
                    return create_bool_value(left.data.int_val == right.data.int_val);
                } else if (left.type == VAL_STRING && right.type == VAL_STRING) {
                    return create_bool_value(strcmp(left.data.str_val, right.data.str_val) == 0);
                } else if (left.type == VAL_BOOL && right.type == VAL_BOOL) {
                    return create_bool_value(left.data.bool_val == right.data.bool_val);
                }
            } else if (strcmp(node->data.binary_op.operator, "NEQ") == 0) {
                if (left.type == VAL_INT && right.type == VAL_INT) {
                    return create_bool_value(left.data.int_val != right.data.int_val);
                } else if (left.type == VAL_STRING && right.type == VAL_STRING) {
                    return create_bool_value(strcmp(left.data.str_val, right.data.str_val) != 0);
                } else if (left.type == VAL_BOOL && right.type == VAL_BOOL) {
                    return create_bool_value(left.data.bool_val != right.data.bool_val);
                }
            }
            
            if (left.type == VAL_BOOL && right.type == VAL_BOOL) {
                if (strcmp(node->data.binary_op.operator, "AND") == 0) {
                    return create_bool_value(left.data.bool_val && right.data.bool_val);
                } else if (strcmp(node->data.binary_op.operator, "OR") == 0) {
                    return create_bool_value(left.data.bool_val || right.data.bool_val);
                }
            }
            
            fprintf(stderr, "Erro: Operador '%s' não suportado para os tipos dados\n", 
                    node->data.binary_op.operator);
            exit(1);
        }
        
        case NODE_UNARY_OP: {
            Value operand = evaluate_expression(node->data.unary_op.operand, table);
            
            if (strcmp(node->data.unary_op.operator, "+") == 0) {
                if (operand.type != VAL_INT) {
                    fprintf(stderr, "Erro: Operador unário '+' requer operando i32\n");
                    exit(1);
                }
                return operand;
            } else if (strcmp(node->data.unary_op.operator, "-") == 0) {
                if (operand.type != VAL_INT) {
                    fprintf(stderr, "Erro: Operador unário '-' requer operando i32\n");
                    exit(1);
                }
                return create_int_value(-operand.data.int_val);
            } else if (strcmp(node->data.unary_op.operator, "not") == 0) {
                if (operand.type != VAL_BOOL) {
                    fprintf(stderr, "Erro: Operador 'not' requer operando bool\n");
                    exit(1);
                }
                return create_bool_value(!operand.data.bool_val);
            }
            
            fprintf(stderr, "Erro: Operador unário '%s' não suportado\n", 
                    node->data.unary_op.operator);
            exit(1);
        }
        
        default:
            fprintf(stderr, "Erro: Tipo de nó inesperado na expressão\n");
            exit(1);
    }
    
    Value null_value;
    null_value.type = VAL_INT;
    null_value.data.int_val = 0;
    return null_value;
}

static void execute_statement(Node* node, SymbolTable* table) {
    if (node == NULL) return;
    
    switch (node->type) {
        case NODE_BLOCK: {
            for (int i = 0; i < node->data.block.stmt_count; i++) {
                execute_statement(node->data.block.statements[i], table);
            }
            break;
        }
        
        case NODE_VAR_DECL: {
            Value init_value;
            
            if (node->data.var_decl.init_expr != NULL) {
                init_value = evaluate_expression(node->data.var_decl.init_expr, table);
                
                if (strcmp(node->data.var_decl.data_type, "i32") == 0) {
                    if (init_value.type != VAL_INT) {
                        fprintf(stderr, "Erro: Tipo incompatível na inicialização de '%s'\n", 
                                node->data.var_decl.name);
                        exit(1);
                    }
                } else if (strcmp(node->data.var_decl.data_type, "bool") == 0) {
                    if (init_value.type != VAL_BOOL) {
                        fprintf(stderr, "Erro: Tipo incompatível na inicialização de '%s'\n", 
                                node->data.var_decl.name);
                        exit(1);
                    }
                } else if (strcmp(node->data.var_decl.data_type, "str") == 0) {
                    if (init_value.type != VAL_STRING) {
                        fprintf(stderr, "Erro: Tipo incompatível na inicialização de '%s'\n", 
                                node->data.var_decl.name);
                        exit(1);
                    }
                }
            } else {
                if (strcmp(node->data.var_decl.data_type, "i32") == 0) {
                    init_value = create_int_value(0);
                } else if (strcmp(node->data.var_decl.data_type, "bool") == 0) {
                    init_value = create_bool_value(false);
                } else if (strcmp(node->data.var_decl.data_type, "str") == 0) {
                    init_value = create_string_value("");
                } else {
                    fprintf(stderr, "Erro: Tipo desconhecido '%s'\n", 
                            node->data.var_decl.data_type);
                    exit(1);
                }
            }
            
            set_symbol(table, node->data.var_decl.name, node->data.var_decl.data_type, init_value);
            break;
        }
        
        case NODE_ASSIGN: {
            Value value = evaluate_expression(node->data.assign.value, table);
            Symbol* symbol = get_symbol(table, node->data.assign.name);
            
            if (symbol == NULL) {
                fprintf(stderr, "Erro: Variável '%s' não definida\n", 
                        node->data.assign.name);
                exit(1);
            }
            
            if ((strcmp(symbol->type, "i32") == 0 && value.type != VAL_INT) ||
                (strcmp(symbol->type, "bool") == 0 && value.type != VAL_BOOL) ||
                (strcmp(symbol->type, "str") == 0 && value.type != VAL_STRING)) {
                fprintf(stderr, "Erro: Tipo incompatível na atribuição de '%s'\n", 
                        node->data.assign.name);
                exit(1);
            }
            
            if (symbol->value.type == VAL_STRING && symbol->value.data.str_val != NULL) {
                free(symbol->value.data.str_val);
            }
            
            symbol->value = value;
            break;
        }
        
        case NODE_IF: {
            Value condition = evaluate_expression(node->data.if_stmt.condition, table);
            
            if (condition.type != VAL_BOOL) {
                fprintf(stderr, "Erro: Condição do ping deve ser booleana\n");
                exit(1);
            }
            
            if (condition.data.bool_val) {
                execute_statement(node->data.if_stmt.then_branch, table);
            } else if (node->data.if_stmt.else_branch != NULL) {
                execute_statement(node->data.if_stmt.else_branch, table);
            }
            break;
        }
        
        case NODE_WHILE: {
            while (true) {
                Value condition = evaluate_expression(node->data.while_stmt.condition, table);
                
                if (condition.type != VAL_BOOL) {
                    fprintf(stderr, "Erro: Condição do stream deve ser booleana\n");
                    exit(1);
                }
                
                if (!condition.data.bool_val) {
                    break;
                }
                
                execute_statement(node->data.while_stmt.body, table);
            }
            break;
        }
        
        case NODE_REPEAT: {
            do {
                execute_statement(node->data.repeat_stmt.body, table);
                
                Value condition = evaluate_expression(node->data.repeat_stmt.condition, table);
                
                if (condition.type != VAL_BOOL) {
                    fprintf(stderr, "Erro: Condição do repeat-until deve ser booleana\n");
                    exit(1);
                }
                
                if (condition.data.bool_val) {
                    break;
                }
            } while (true);
            break;
        }
        
        case NODE_SWITCH: {
            Value condition = evaluate_expression(node->data.switch_stmt.condition, table);
            bool case_matched = false;
            
            for (int i = 0; i < node->data.switch_stmt.case_count; i++) {
                Node* case_node = node->data.switch_stmt.cases[i];
                Value case_value = evaluate_expression(case_node->data.case_stmt.value, table);
                
                if (!check_same_type(condition, case_value)) {
                    fprintf(stderr, "Erro: Tipo incompatível no select\n");
                    exit(1);
                }
                
                bool match = false;
                
                if (condition.type == VAL_INT && case_value.type == VAL_INT) {
                    match = (condition.data.int_val == case_value.data.int_val);
                } else if (condition.type == VAL_STRING && case_value.type == VAL_STRING) {
                    match = (strcmp(condition.data.str_val, case_value.data.str_val) == 0);
                } else if (condition.type == VAL_BOOL && case_value.type == VAL_BOOL) {
                    match = (condition.data.bool_val == case_value.data.bool_val);
                }
                
                if (match) {
                    execute_statement(case_node->data.case_stmt.body, table);
                    case_matched = true;
                    break;
                }
            }
            
            if (!case_matched && node->data.switch_stmt.default_case != NULL) {
                execute_statement(node->data.switch_stmt.default_case, table);
            }
            break;
        }
        
        case NODE_PRINT: {
            Value value = evaluate_expression(node->data.print_stmt.expr, table);
            
            switch (value.type) {
                case VAL_INT:
                    printf("%d\n", value.data.int_val);
                    break;
                case VAL_STRING:
                    printf("%s\n", value.data.str_val);
                    break;
                case VAL_BOOL:
                    printf("%s\n", value.data.bool_val ? "true" : "false");
                    break;
            }
            break;
        }
        
        default:
            evaluate_expression(node, table);
            break;
    }
}