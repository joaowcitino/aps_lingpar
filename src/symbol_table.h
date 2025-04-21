#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <llvm-c/Core.h>

typedef struct Symbol {
    char* name;
    LLVMValueRef value;
    int type;
    struct Symbol* next;
} Symbol;

void init_symbol_table();
Symbol* find_symbol(const char* name);
void add_symbol(const char* name, LLVMValueRef value, int type);
void remove_symbol(const char* name);
void cleanup_symbol_table();

#endif