#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

Symbol* symbol_table = NULL;

void init_symbol_table() {
    symbol_table = NULL;
}

Symbol* find_symbol(const char* name) {
    Symbol* current = symbol_table;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void add_symbol(const char* name, LLVMValueRef value, int type) {
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->value = value;
    symbol->type = type;
    symbol->next = symbol_table;
    symbol_table = symbol;
}

void remove_symbol(const char* name) {
    Symbol* current = symbol_table;
    Symbol* prev = NULL;
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                symbol_table = current->next;
            }
            
            free(current->name);
            free(current);
            return;
        }
        
        prev = current;
        current = current->next;
    }
}

void cleanup_symbol_table() {
    Symbol* current = symbol_table;
    Symbol* next;
    
    while (current) {
        next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    
    symbol_table = NULL;
}