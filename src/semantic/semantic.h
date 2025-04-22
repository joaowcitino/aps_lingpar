#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../parser/ast.h"
#include "symbol_table.h"

typedef struct {
    DataType type;
    bool is_valid;
} TypeInfo;

bool analyze_semantics(ASTNode *ast_root);

TypeInfo check_expression(SymbolTable *st, ASTNode *expr);
bool check_declarations(SymbolTable *st, ASTNode *declarations);
bool check_function_def(SymbolTable *st, ASTNode *func_def);
bool check_var_declaration(SymbolTable *st, ASTNode *var_decl);
bool check_conditional(SymbolTable *st, ASTNode *conditional);
bool check_for_loop(SymbolTable *st, ASTNode *for_loop);
bool check_while_loop(SymbolTable *st, ASTNode *while_loop);
TypeInfo check_function_call(SymbolTable *st, ASTNode *func_call);

#endif