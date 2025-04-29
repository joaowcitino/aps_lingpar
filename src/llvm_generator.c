#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/BitWriter.h>
#include "llvm_generator.h"

typedef struct {
    char* name;
    LLVMValueRef value;
    LLVMTypeRef type;
} Symbol;

typedef struct {
    Symbol* symbols;
    int symbol_count;
    int capacity;
} SymbolTable;

typedef struct {
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMValueRef function;
    SymbolTable* symbol_table;
} GeneratorContext;

static SymbolTable* create_symbol_table();
static void add_symbol(SymbolTable* table, const char* name, LLVMValueRef value, LLVMTypeRef type);
static Symbol* find_symbol(SymbolTable* table, const char* name);
static void free_symbol_table(SymbolTable* table);

static LLVMValueRef generate_node(Node* node, GeneratorContext* context);
static LLVMValueRef generate_block(Node* node, GeneratorContext* context);
static LLVMValueRef generate_var_decl(Node* node, GeneratorContext* context);
static LLVMValueRef generate_assignment(Node* node, GeneratorContext* context);
static LLVMValueRef generate_if_stmt(Node* node, GeneratorContext* context);
static LLVMValueRef generate_while_stmt(Node* node, GeneratorContext* context);
static LLVMValueRef generate_repeat_stmt(Node* node, GeneratorContext* context);
static LLVMValueRef generate_switch_stmt(Node* node, GeneratorContext* context);
static LLVMValueRef generate_print_stmt(Node* node, GeneratorContext* context);
static LLVMValueRef generate_binary_op(Node* node, GeneratorContext* context);
static LLVMValueRef generate_unary_op(Node* node, GeneratorContext* context);
static LLVMValueRef generate_expression(Node* node, GeneratorContext* context);

static LLVMValueRef int_to_string(GeneratorContext* context, LLVMValueRef int_val) {
    LLVMValueRef int_to_str_func = LLVMGetNamedFunction(context->module, "int_to_string");
    if (!int_to_str_func) {
        LLVMTypeRef param_types[] = { LLVMInt32Type() };
        LLVMTypeRef ret_type = LLVMPointerType(LLVMInt8Type(), 0);
        LLVMTypeRef func_type = LLVMFunctionType(ret_type, param_types, 1, 0);
        int_to_str_func = LLVMAddFunction(context->module, "int_to_string", func_type);
    }
    
    LLVMTypeRef func_type = LLVMGetElementType(LLVMTypeOf(int_to_str_func));
    LLVMValueRef args[] = { int_val };
    return LLVMBuildCall2(context->builder, func_type, int_to_str_func, args, 1, "int_str");
}

static SymbolTable* create_symbol_table() {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    table->symbols = NULL;
    table->symbol_count = 0;
    table->capacity = 0;
    return table;
}

static void add_symbol(SymbolTable* table, const char* name, LLVMValueRef value, LLVMTypeRef type) {
    for (int i = 0; i < table->symbol_count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            table->symbols[i].value = value;
            return;
        }
    }
    
    if (table->symbol_count >= table->capacity) {
        int new_capacity = table->capacity == 0 ? 8 : table->capacity * 2;
        Symbol* new_symbols = (Symbol*)realloc(table->symbols, new_capacity * sizeof(Symbol));
        
        if (new_symbols == NULL) {
            fprintf(stderr, "Erro: Falha na alocação de memória para tabela de símbolos\n");
            exit(1);
        }
        
        table->symbols = new_symbols;
        table->capacity = new_capacity;
    }
    
    table->symbols[table->symbol_count].name = strdup(name);
    table->symbols[table->symbol_count].value = value;
    table->symbols[table->symbol_count].type = type;
    table->symbol_count++;
}

static Symbol* find_symbol(SymbolTable* table, const char* name) {
    for (int i = 0; i < table->symbol_count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

static void free_symbol_table(SymbolTable* table) {
    if (table == NULL) return;
    
    for (int i = 0; i < table->symbol_count; i++) {
        free(table->symbols[i].name);
    }
    
    free(table->symbols);
    free(table);
}

void generate_llvm_code(Node* ast_root, const char* output_file) {
    LLVMInitializeCore(LLVMGetGlobalPassRegistry());
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    
    GeneratorContext context;
    context.module = LLVMModuleCreateWithName("techflow_module");
    context.builder = LLVMCreateBuilder();
    context.symbol_table = create_symbol_table();
    
    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(), printf_args, 1, true);
    LLVMAddFunction(context.module, "printf", printf_type);
    
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, false);
    context.function = LLVMAddFunction(context.module, "main", main_type);
    
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(context.function, "entry");
    LLVMPositionBuilderAtEnd(context.builder, entry);
    
    if (ast_root != NULL && ast_root->type == NODE_PROGRAM) {
        generate_node(ast_root->data.program.body, &context);
    }
    
    LLVMBuildRet(context.builder, LLVMConstInt(LLVMInt32Type(), 0, false));
    
    char* error = NULL;
    LLVMVerifyModule(context.module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);
    
    LLVMPassManagerRef pass_manager = LLVMCreatePassManager();

    LLVMAddInstructionCombiningPass(pass_manager);
    LLVMAddReassociatePass(pass_manager);
    LLVMAddGVNPass(pass_manager);
    LLVMAddCFGSimplificationPass(pass_manager);
    LLVMRunPassManager(pass_manager, context.module);
    LLVMDisposePassManager(pass_manager);
    
    if (LLVMWriteBitcodeToFile(context.module, output_file) != 0) {
        fprintf(stderr, "Erro ao escrever bitcode para arquivo %s\n", output_file);
    }
    
    LLVMDumpModule(context.module);
    
    free_symbol_table(context.symbol_table);
    LLVMDisposeBuilder(context.builder);
    LLVMDisposeModule(context.module);
}

static LLVMValueRef generate_node(Node* node, GeneratorContext* context) {
    if (node == NULL) return NULL;
    
    switch (node->type) {
        case NODE_BLOCK:
            return generate_block(node, context);
        case NODE_VAR_DECL:
            return generate_var_decl(node, context);
        case NODE_ASSIGN:
            return generate_assignment(node, context);
        case NODE_IF:
            return generate_if_stmt(node, context);
        case NODE_WHILE:
            return generate_while_stmt(node, context);
        case NODE_REPEAT:
            return generate_repeat_stmt(node, context);
        case NODE_SWITCH:
            return generate_switch_stmt(node, context);
        case NODE_PRINT:
            return generate_print_stmt(node, context);
        case NODE_BINARY_OP:
            return generate_binary_op(node, context);
        case NODE_UNARY_OP:
            return generate_unary_op(node, context);
        case NODE_INT_VAL:
            return LLVMConstInt(LLVMInt32Type(), node->data.int_value, false);
        case NODE_BOOL_VAL:
            return LLVMConstInt(LLVMInt1Type(), node->data.bool_value, false);
        case NODE_STRING_VAL:
            return LLVMBuildGlobalStringPtr(context->builder, node->data.str_value, "str");
        case NODE_IDENTIFIER: {
            Symbol* symbol = find_symbol(context->symbol_table, node->data.str_value);
            if (symbol == NULL) {
                fprintf(stderr, "Erro: Variável '%s' não definida\n", node->data.str_value);
                exit(1);
            }
            return LLVMBuildLoad2(context->builder, symbol->type, symbol->value, node->data.str_value);
        }
        default:
            fprintf(stderr, "Erro: Tipo de nó não suportado: %d\n", node->type);
            exit(1);
    }
    
    return NULL;
}

static LLVMValueRef generate_block(Node* node, GeneratorContext* context) {
    LLVMValueRef last_value = NULL;
    
    for (int i = 0; i < node->data.block.stmt_count; i++) {
        last_value = generate_node(node->data.block.statements[i], context);
    }
    
    return last_value;
}

static LLVMValueRef generate_var_decl(Node* node, GeneratorContext* context) {
    LLVMTypeRef type;
    
    if (strcmp(node->data.var_decl.data_type, "i32") == 0) {
        type = LLVMInt32Type();
    } else if (strcmp(node->data.var_decl.data_type, "bool") == 0) {
        type = LLVMInt1Type();
    } else if (strcmp(node->data.var_decl.data_type, "str") == 0) {
        type = LLVMPointerType(LLVMInt8Type(), 0);
    } else {
        fprintf(stderr, "Erro: Tipo de variável não suportado: %s\n", node->data.var_decl.data_type);
        exit(1);
    }
    
    LLVMValueRef alloca = LLVMBuildAlloca(context->builder, type, node->data.var_decl.name);
    
    add_symbol(context->symbol_table, node->data.var_decl.name, alloca, type);
    
    if (node->data.var_decl.init_expr != NULL) {
        LLVMValueRef init_val = generate_expression(node->data.var_decl.init_expr, context);
        LLVMBuildStore(context->builder, init_val, alloca);
    } else {
        if (strcmp(node->data.var_decl.data_type, "i32") == 0) {
            LLVMBuildStore(context->builder, LLVMConstInt(LLVMInt32Type(), 0, false), alloca);
        } else if (strcmp(node->data.var_decl.data_type, "bool") == 0) {
            LLVMBuildStore(context->builder, LLVMConstInt(LLVMInt1Type(), 0, false), alloca);
        } else if (strcmp(node->data.var_decl.data_type, "str") == 0) {
            LLVMBuildStore(context->builder, LLVMBuildGlobalStringPtr(context->builder, "", "empty_str"), alloca);
        }
    }
    
    return alloca;
}

static LLVMValueRef generate_expression(Node* node, GeneratorContext* context) {
    return generate_node(node, context);
}

static LLVMValueRef generate_assignment(Node* node, GeneratorContext* context) {
    Symbol* symbol = find_symbol(context->symbol_table, node->data.assign.name);
    
    if (symbol == NULL) {
        fprintf(stderr, "Erro: Variável '%s' não definida\n", node->data.assign.name);
        exit(1);
    }
    
    LLVMValueRef value = generate_expression(node->data.assign.value, context);
    return LLVMBuildStore(context->builder, value, symbol->value);
}

static LLVMValueRef generate_binary_op(Node* node, GeneratorContext* context) {
    LLVMValueRef left = generate_expression(node->data.binary_op.left, context);
    LLVMValueRef right = generate_expression(node->data.binary_op.right, context);
    
    if (strcmp(node->data.binary_op.operator, "+") == 0) {
        return LLVMBuildAdd(context->builder, left, right, "addtmp");
    } else if (strcmp(node->data.binary_op.operator, "-") == 0) {
        return LLVMBuildSub(context->builder, left, right, "subtmp");
    } else if (strcmp(node->data.binary_op.operator, "*") == 0) {
        return LLVMBuildMul(context->builder, left, right, "multmp");
    } else if (strcmp(node->data.binary_op.operator, "/") == 0) {
        return LLVMBuildSDiv(context->builder, left, right, "divtmp");
    } else if (strcmp(node->data.binary_op.operator, "%") == 0) {
        return LLVMBuildSRem(context->builder, left, right, "modtmp");
    }
    
    else if (strcmp(node->data.binary_op.operator, "<") == 0) {
        return LLVMBuildICmp(context->builder, LLVMIntSLT, left, right, "lttmp");
    } else if (strcmp(node->data.binary_op.operator, ">") == 0) {
        return LLVMBuildICmp(context->builder, LLVMIntSGT, left, right, "gttmp");
    } else if (strcmp(node->data.binary_op.operator, "LE") == 0) {
        return LLVMBuildICmp(context->builder, LLVMIntSLE, left, right, "letmp");
    } else if (strcmp(node->data.binary_op.operator, "GE") == 0) {
        return LLVMBuildICmp(context->builder, LLVMIntSGE, left, right, "getmp");
    } else if (strcmp(node->data.binary_op.operator, "EQ") == 0) {
        return LLVMBuildICmp(context->builder, LLVMIntEQ, left, right, "eqtmp");
    } else if (strcmp(node->data.binary_op.operator, "NEQ") == 0) {
        return LLVMBuildICmp(context->builder, LLVMIntNE, left, right, "netmp");
    }
    
    else if (strcmp(node->data.binary_op.operator, "AND") == 0) {
        return LLVMBuildAnd(context->builder, left, right, "andtmp");
    } else if (strcmp(node->data.binary_op.operator, "OR") == 0) {
        return LLVMBuildOr(context->builder, left, right, "ortmp");
    }
    
    else if (strcmp(node->data.binary_op.operator, "CONCAT") == 0) {
        LLVMValueRef concat_func = LLVMGetNamedFunction(context->module, "concat_strings");
        if (!concat_func) {
            LLVMTypeRef param_types[] = {
                LLVMPointerType(LLVMInt8Type(), 0),
                LLVMPointerType(LLVMInt8Type(), 0)
            };
            LLVMTypeRef ret_type = LLVMPointerType(LLVMInt8Type(), 0);
            LLVMTypeRef func_type = LLVMFunctionType(ret_type, param_types, 2, 0);
            concat_func = LLVMAddFunction(context->module, "concat_strings", func_type);
        }

        LLVMTypeRef left_type = LLVMTypeOf(left);
        if (LLVMGetTypeKind(left_type) == LLVMIntegerTypeKind && 
            LLVMGetIntTypeWidth(left_type) == 32) {
            left = int_to_string(context, left);
        }

        LLVMTypeRef right_type = LLVMTypeOf(right);
        if (LLVMGetTypeKind(right_type) == LLVMIntegerTypeKind && 
            LLVMGetIntTypeWidth(right_type) == 32) {
            right = int_to_string(context, right);
        }

        LLVMTypeRef func_type = LLVMGetElementType(LLVMTypeOf(concat_func));
        LLVMValueRef args[] = { left, right };
        return LLVMBuildCall2(context->builder, func_type, concat_func, args, 2, "concat_result");
    }
    
    fprintf(stderr, "Erro: Operador binário não suportado: %s\n", node->data.binary_op.operator);
    exit(1);
}

static LLVMValueRef generate_unary_op(Node* node, GeneratorContext* context) {
    LLVMValueRef operand = generate_expression(node->data.unary_op.operand, context);
    
    if (strcmp(node->data.unary_op.operator, "+") == 0) {
        return operand;
    } else if (strcmp(node->data.unary_op.operator, "-") == 0) {
        return LLVMBuildNeg(context->builder, operand, "negtmp");
    } else if (strcmp(node->data.unary_op.operator, "not") == 0) {
        return LLVMBuildNot(context->builder, operand, "nottmp");
    }
    
    fprintf(stderr, "Erro: Operador unário não suportado: %s\n", node->data.unary_op.operator);
    exit(1);
}

static LLVMValueRef generate_if_stmt(Node* node, GeneratorContext* context) {
    LLVMValueRef condition = generate_expression(node->data.if_stmt.condition, context);
    
    LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(context->function, "then");
    LLVMBasicBlockRef else_block = node->data.if_stmt.else_branch ? 
                                    LLVMAppendBasicBlock(context->function, "else") : NULL;
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(context->function, "ifcont");
    
    if (else_block) {
        LLVMBuildCondBr(context->builder, condition, then_block, else_block);
    } else {
        LLVMBuildCondBr(context->builder, condition, then_block, merge_block);
    }
    
    LLVMPositionBuilderAtEnd(context->builder, then_block);
    generate_node(node->data.if_stmt.then_branch, context);
    LLVMBuildBr(context->builder, merge_block);
    
    if (else_block) {
        LLVMPositionBuilderAtEnd(context->builder, else_block);
        generate_node(node->data.if_stmt.else_branch, context);
        LLVMBuildBr(context->builder, merge_block);
    }
    
    LLVMPositionBuilderAtEnd(context->builder, merge_block);
    
    return NULL;
}

static LLVMValueRef generate_while_stmt(Node* node, GeneratorContext* context) {
    LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(context->function, "while_cond");
    LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(context->function, "while_body");
    LLVMBasicBlockRef end_block = LLVMAppendBasicBlock(context->function, "while_end");
    
    LLVMBuildBr(context->builder, cond_block);
    
    LLVMPositionBuilderAtEnd(context->builder, cond_block);
    LLVMValueRef condition = generate_expression(node->data.while_stmt.condition, context);
    LLVMBuildCondBr(context->builder, condition, body_block, end_block);
    
    LLVMPositionBuilderAtEnd(context->builder, body_block);
    generate_node(node->data.while_stmt.body, context);
    LLVMBuildBr(context->builder, cond_block);
    
    LLVMPositionBuilderAtEnd(context->builder, end_block);
    
    return NULL;
}

static LLVMValueRef generate_repeat_stmt(Node* node, GeneratorContext* context) {
    LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(context->function, "repeat_body");
    LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(context->function, "repeat_cond");
    LLVMBasicBlockRef end_block = LLVMAppendBasicBlock(context->function, "repeat_end");
    
    LLVMBuildBr(context->builder, body_block);
    
    LLVMPositionBuilderAtEnd(context->builder, body_block);
    generate_node(node->data.repeat_stmt.body, context);
    LLVMBuildBr(context->builder, cond_block);
    
    LLVMPositionBuilderAtEnd(context->builder, cond_block);
    LLVMValueRef condition = generate_expression(node->data.repeat_stmt.condition, context);
    LLVMBuildCondBr(context->builder, condition, end_block, body_block);
    
    LLVMPositionBuilderAtEnd(context->builder, end_block);
    
    return NULL;
}

static LLVMValueRef generate_switch_stmt(Node* node, GeneratorContext* context) {
    LLVMValueRef condition = generate_expression(node->data.switch_stmt.condition, context);
    
    LLVMBasicBlockRef end_block = LLVMAppendBasicBlock(context->function, "switch_end");
    
    LLVMValueRef switch_inst = LLVMBuildSwitch(context->builder, condition, 
                                               node->data.switch_stmt.default_case ? 
                                               LLVMAppendBasicBlock(context->function, "default_case") : 
                                               end_block, 
                                               node->data.switch_stmt.case_count);
    
    for (int i = 0; i < node->data.switch_stmt.case_count; i++) {
        Node* case_node = node->data.switch_stmt.cases[i];
        LLVMValueRef case_value = generate_expression(case_node->data.case_stmt.value, context);
        LLVMBasicBlockRef case_block = LLVMAppendBasicBlock(context->function, "case");
        
        LLVMAddCase(switch_inst, case_value, case_block);
        
        LLVMPositionBuilderAtEnd(context->builder, case_block);
        generate_node(case_node->data.case_stmt.body, context);
        LLVMBuildBr(context->builder, end_block);
    }
    
    if (node->data.switch_stmt.default_case) {
        LLVMBasicBlockRef default_block = LLVMGetSwitchDefaultDest(switch_inst);
        LLVMPositionBuilderAtEnd(context->builder, default_block);
        generate_node(node->data.switch_stmt.default_case, context);
        LLVMBuildBr(context->builder, end_block);
    }
    
    LLVMPositionBuilderAtEnd(context->builder, end_block);
    
    return NULL;
}

static LLVMValueRef generate_print_stmt(Node* node, GeneratorContext* context) {
    LLVMValueRef printf_func = LLVMGetNamedFunction(context->module, "printf");
    
    if (!printf_func) {
        fprintf(stderr, "Erro: Função printf não encontrada\n");
        exit(1);
    }
    
    LLVMValueRef expr = generate_expression(node->data.print_stmt.expr, context);
    LLVMTypeRef expr_type = LLVMTypeOf(expr);
    
    const char* format;
    if (LLVMGetTypeKind(expr_type) == LLVMIntegerTypeKind) {
        if (LLVMGetIntTypeWidth(expr_type) == 1) {
            format = "%s\n";
            
            LLVMValueRef bool_to_str_func = LLVMGetNamedFunction(context->module, "bool_to_string");
            if (!bool_to_str_func) {
                LLVMTypeRef param_types[] = { LLVMInt1Type() };
                LLVMTypeRef ret_type = LLVMPointerType(LLVMInt8Type(), 0);
                LLVMTypeRef func_type = LLVMFunctionType(ret_type, param_types, 1, 0);
                bool_to_str_func = LLVMAddFunction(context->module, "bool_to_string", func_type);
            }

            LLVMTypeRef func_type = LLVMGetElementType(LLVMTypeOf(bool_to_str_func));
            LLVMValueRef args[] = { expr };
            expr = LLVMBuildCall2(context->builder, func_type, bool_to_str_func, args, 1, "bool_str");
        } else {
            format = "%d\n";
        }
    } else {
        format = "%s\n";
    }
    
    LLVMValueRef format_str = LLVMBuildGlobalStringPtr(context->builder, format, "format");

    LLVMTypeRef func_type = LLVMGetElementType(LLVMTypeOf(printf_func));
    LLVMValueRef args[] = { format_str, expr };
    return LLVMBuildCall2(context->builder, func_type, printf_func, args, 2, "printf_result");
}