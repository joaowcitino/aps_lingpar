#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

static unsigned int hash(const char *str, int size) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash * 31) + (*str++);
    }
    return hash % size;
}

SymbolTable *create_symbol_table(int size) {
    SymbolTable *st = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (!st) {
        fprintf(stderr, "Erro: Falha ao alocar memória para tabela de símbolos\n");
        exit(1);
    }
    
    st->size = size;
    st->current_scope = 0;
    
    st->table = (Symbol **)calloc(size, sizeof(Symbol *));
    if (!st->table) {
        fprintf(stderr, "Erro: Falha ao alocar memória para tabela de hash\n");
        free(st);
        exit(1);
    }
    
    return st;
}

void free_symbol_table(SymbolTable *st) {
    if (!st) return;
    
    for (int i = 0; i < st->size; i++) {
        Symbol *current = st->table[i];
        while (current) {
            Symbol *next = current->next;
            
            if (current->sym_type == SYM_FUNCTION && current->function.param_types) {
                free(current->function.param_types);
            }
            
            free(current->name);
            free(current);
            current = next;
        }
    }
    
    free(st->table);
    free(st);
}

void enter_scope(SymbolTable *st) {
    st->current_scope++;
}

void leave_scope(SymbolTable *st) {
    if (st->current_scope > 0) {
        for (int i = 0; i < st->size; i++) {
            Symbol *prev = NULL;
            Symbol *current = st->table[i];
            
            while (current) {
                if (current->scope_level == st->current_scope) {
                    Symbol *to_delete = current;
                    
                    if (prev) {
                        prev->next = current->next;
                        current = current->next;
                    } else {
                        st->table[i] = current->next;
                        current = st->table[i];
                    }
                    
                    if (to_delete->sym_type == SYM_FUNCTION && to_delete->function.param_types) {
                        free(to_delete->function.param_types);
                    }
                    
                    free(to_delete->name);
                    free(to_delete);
                } else {
                    prev = current;
                    current = current->next;
                }
            }
        }
        
        st->current_scope--;
    }
}

bool insert_symbol(SymbolTable *st, const char *name, SymbolType sym_type, DataType data_type) {
    if (lookup_symbol_current_scope(st, name)) {
        return false;
    }
    
    unsigned int index = hash(name, st->size);
    
    Symbol *new_symbol = (Symbol *)malloc(sizeof(Symbol));
    if (!new_symbol) {
        fprintf(stderr, "Erro: Falha ao alocar memória para símbolo\n");
        exit(1);
    }
    
    new_symbol->name = strdup(name);
    new_symbol->sym_type = sym_type;
    new_symbol->data_type = data_type;
    new_symbol->scope_level = st->current_scope;
    
    if (sym_type == SYM_FUNCTION) {
        new_symbol->function.param_count = 0;
        new_symbol->function.param_types = NULL;
        new_symbol->function.return_type = TYPE_VOID;
    }
    
    new_symbol->next = st->table[index];
    st->table[index] = new_symbol;
    
    return true;
}

bool insert_function(SymbolTable *st, const char *name, int param_count, DataType *param_types, DataType return_type) {
    if (!insert_symbol(st, name, SYM_FUNCTION, TYPE_VOID)) {
        return false;
    }
    
    Symbol *func = lookup_symbol_current_scope(st, name);
    if (!func) {
        return false;
    }
    
    func->function.param_count = param_count;
    func->function.return_type = return_type;
    
    if (param_count > 0) {
        func->function.param_types = (DataType *)malloc(param_count * sizeof(DataType));
        if (!func->function.param_types) {
            fprintf(stderr, "Erro: Falha ao alocar memória para tipos de parâmetros\n");
            exit(1);
        }
        
        memcpy(func->function.param_types, param_types, param_count * sizeof(DataType));
    }
    
    return true;
}

Symbol *lookup_symbol(SymbolTable *st, const char *name) {
    unsigned int index = hash(name, st->size);
    
    Symbol *current = st->table[index];
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

Symbol *lookup_symbol_current_scope(SymbolTable *st, const char *name) {
    unsigned int index = hash(name, st->size);
    
    Symbol *current = st->table[index];
    while (current) {
        if (strcmp(current->name, name) == 0 && current->scope_level == st->current_scope) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

const char *data_type_to_string(DataType type) {
    switch (type) {
        case TYPE_BYTE: return "byte";
        case TYPE_BIT: return "bit";
        case TYPE_CHIP: return "chip";
        case TYPE_ARRAY: return "array";
        case TYPE_VOID: return "void";
        default: return "unknown";
    }
}

DataType string_to_data_type(const char *type) {
    if (strcmp(type, "byte") == 0) return TYPE_BYTE;
    if (strcmp(type, "bit") == 0) return TYPE_BIT;
    if (strcmp(type, "chip") == 0) return TYPE_CHIP;
    if (strcmp(type, "array") == 0) return TYPE_ARRAY;
    if (strcmp(type, "void") == 0) return TYPE_VOID;
    
    return TYPE_VOID;
}