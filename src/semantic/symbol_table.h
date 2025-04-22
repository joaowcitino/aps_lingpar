#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

typedef enum {
    SYM_VARIABLE,
    SYM_FUNCTION,
    SYM_PARAMETER
} SymbolType;

typedef enum {
    TYPE_BYTE,
    TYPE_BIT,
    TYPE_CHIP,
    TYPE_ARRAY,
    TYPE_VOID
} DataType;

typedef struct Symbol {
    char *name;
    SymbolType sym_type;
    DataType data_type;
    int scope_level;
    
    struct {
        int param_count;
        DataType *param_types;
        DataType return_type;
    } function;
    
    struct Symbol *next;
} Symbol;

typedef struct {
    int size;
    struct Symbol **table;
    int current_scope;
} SymbolTable;

SymbolTable *create_symbol_table(int size);
void free_symbol_table(SymbolTable *st);

void enter_scope(SymbolTable *st);
void leave_scope(SymbolTable *st);

bool insert_symbol(SymbolTable *st, const char *name, SymbolType sym_type, DataType data_type);
bool insert_function(SymbolTable *st, const char *name, int param_count, DataType *param_types, DataType return_type);
Symbol *lookup_symbol(SymbolTable *st, const char *name);
Symbol *lookup_symbol_current_scope(SymbolTable *st, const char *name);

const char *data_type_to_string(DataType type);
DataType string_to_data_type(const char *type);

#endif