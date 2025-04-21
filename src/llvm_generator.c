#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "llvm_generator.h"
#include "symbol_table.h"

LLVMGenerator* init_llvm_generator(const char* module_name) {
    LLVMGenerator* generator = malloc(sizeof(LLVMGenerator));
    
    LLVMInitializeCore(LLVMGetGlobalPassRegistry());
    LLVMLinkInMCJIT();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    
    generator->context = LLVMContextCreate();
    generator->module = LLVMModuleCreateWithNameInContext(module_name, generator->context);
    generator->builder = LLVMCreateBuilderInContext(generator->context);
    
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    generator->current_function = LLVMAddFunction(generator->module, "main", main_type);
    
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(generator->current_function, "entry");
    LLVMPositionBuilderAtEnd(generator->builder, entry);
    
    LLVMTypeRef printf_arg_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(), printf_arg_types, 1, 1);
    LLVMAddFunction(generator->module, "printf", printf_type);
    
    LLVMTypeRef rand_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMAddFunction(generator->module, "rand", rand_type);
    
    return generator;
}

LLVMValueRef generate_literal(LLVMGenerator* generator, ASTNode* node) {
    switch (node->type) {
        case AST_LITERAL_INT:
            return LLVMConstInt(LLVMInt32Type(), node->data.int_value, 0);
        case AST_LITERAL_FLOAT:
            return LLVMConstReal(LLVMFloatType(), node->data.float_value);
        case AST_LITERAL_STRING:
            return LLVMBuildGlobalStringPtr(generator->builder, node->data.string_value, "str");
        default:
            fprintf(stderr, "Erro: Tipo de literal inválido\n");
            return NULL;
    }
}

LLVMValueRef generate_variable(LLVMGenerator* generator, ASTNode* node) {
    Symbol* symbol = find_symbol(node->data.variable.name);
    if (symbol) {
        return LLVMBuildLoad(generator->builder, symbol->value, node->data.variable.name);
    } else {
        fprintf(stderr, "Erro: Variável '%s' não declarada\n", node->data.variable.name);
        return NULL;
    }
}

LLVMValueRef generate_binary_op(LLVMGenerator* generator, ASTNode* node) {
    LLVMValueRef left = generate_code(generator, node->data.binary_op.left);
    LLVMValueRef right = generate_code(generator, node->data.binary_op.right);
    
    if (!left || !right) return NULL;
    
    switch (node->data.binary_op.op) {
        case '+':
            return LLVMBuildAdd(generator->builder, left, right, "addtmp");
        case '-':
            return LLVMBuildSub(generator->builder, left, right, "subtmp");
        case '*':
            return LLVMBuildMul(generator->builder, left, right, "multmp");
        case '/':
            return LLVMBuildSDiv(generator->builder, left, right, "divtmp");
        case IGUAL:
            return LLVMBuildICmp(generator->builder, LLVMIntEQ, left, right, "eqtmp");
        case DIFERENTE:
            return LLVMBuildICmp(generator->builder, LLVMIntNE, left, right, "netmp");
        case MENOR:
            return LLVMBuildICmp(generator->builder, LLVMIntSLT, left, right, "lttmp");
        case MAIOR:
            return LLVMBuildICmp(generator->builder, LLVMIntSGT, left, right, "gttmp");
        case MENOR_IGUAL:
            return LLVMBuildICmp(generator->builder, LLVMIntSLE, left, right, "letmp");
        case MAIOR_IGUAL:
            return LLVMBuildICmp(generator->builder, LLVMIntSGE, left, right, "getmp");
        case E_LOGICO:
            return LLVMBuildAnd(generator->builder, left, right, "andtmp");
        case OU_LOGICO:
            return LLVMBuildOr(generator->builder, left, right, "ortmp");
        default:
            fprintf(stderr, "Erro: Operador binário inválido\n");
            return NULL;
    }
}

LLVMValueRef generate_declaration(LLVMGenerator* generator, ASTNode* node) {
    LLVMTypeRef type;
    switch (node->data.declaration.var_type) {
        case 0: // protagonista (int)
            type = LLVMInt32Type();
            break;
        case 1: // coadjuvante (float)
            type = LLVMFloatType();
            break;
        case 2: // cenario (string)
            type = LLVMPointerType(LLVMInt8Type(), 0);
            break;
        default:
            fprintf(stderr, "Erro: Tipo de variável inválido\n");
            return NULL;
    }
    
    LLVMValueRef var = LLVMBuildAlloca(generator->builder, type, node->data.declaration.name);
    add_symbol(node->data.declaration.name, var, node->data.declaration.var_type);
    
    if (node->data.declaration.init_value) {
        LLVMValueRef init_val = generate_code(generator, node->data.declaration.init_value);
        LLVMBuildStore(generator->builder, init_val, var);
    }
    
    return var;
}

LLVMValueRef generate_assignment(LLVMGenerator* generator, ASTNode* node) {
    Symbol* symbol = find_symbol(node->data.assignment.name);
    if (!symbol) {
        fprintf(stderr, "Erro: Variável '%s' não declarada\n", node->data.assignment.name);
        return NULL;
    }
    
    LLVMValueRef value = generate_code(generator, node->data.assignment.value);
    LLVMBuildStore(generator->builder, value, symbol->value);
    
    return value;
}

LLVMValueRef generate_if_else(LLVMGenerator* generator, ASTNode* node) {
    LLVMValueRef cond_val = generate_code(generator, node->data.if_else.condition);
    if (!cond_val) return NULL;
    
    LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(generator->current_function, "then");
    LLVMBasicBlockRef else_block = NULL;
    if (node->data.if_else.else_block) {
        else_block = LLVMAppendBasicBlock(generator->current_function, "else");
    }
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(generator->current_function, "ifcont");
    
    if (else_block) {
        LLVMBuildCondBr(generator->builder, cond_val, then_block, else_block);
    } else {
        LLVMBuildCondBr(generator->builder, cond_val, then_block, merge_block);
    }
    
    LLVMPositionBuilderAtEnd(generator->builder, then_block);
    generate_code(generator, node->data.if_else.then_block);
    LLVMBuildBr(generator->builder, merge_block);
    
    if (else_block) {
        LLVMPositionBuilderAtEnd(generator->builder, else_block);
        generate_code(generator, node->data.if_else.else_block);
        LLVMBuildBr(generator->builder, merge_block);
    }
    
    LLVMPositionBuilderAtEnd(generator->builder, merge_block);
    
    return NULL;
}

LLVMValueRef generate_while_loop(LLVMGenerator* generator, ASTNode* node) {
    LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(generator->current_function, "while.cond");
    LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(generator->current_function, "while.body");
    LLVMBasicBlockRef end_block = LLVMAppendBasicBlock(generator->current_function, "while.end");
    
    LLVMBuildBr(generator->builder, cond_block);
    
    LLVMPositionBuilderAtEnd(generator->builder, cond_block);
    LLVMValueRef cond_val = generate_code(generator, node->data.while_loop.condition);
    LLVMBuildCondBr(generator->builder, cond_val, body_block, end_block);
    
    LLVMPositionBuilderAtEnd(generator->builder, body_block);
    generate_code(generator, node->data.while_loop.body);
    LLVMBuildBr(generator->builder, cond_block);
    
    LLVMPositionBuilderAtEnd(generator->builder, end_block);
    
    return NULL;
}

LLVMValueRef generate_for_loop(LLVMGenerator* generator, ASTNode* node) {
    generate_code(generator, node->data.for_loop.init);
    
    LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(generator->current_function, "for.cond");
    LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(generator->current_function, "for.body");
    LLVMBasicBlockRef update_block = LLVMAppendBasicBlock(generator->current_function, "for.update");
    LLVMBasicBlockRef end_block = LLVMAppendBasicBlock(generator->current_function, "for.end");
    
    LLVMBuildBr(generator->builder, cond_block);
    
    LLVMPositionBuilderAtEnd(generator->builder, cond_block);
    LLVMValueRef cond_val = generate_code(generator, node->data.for_loop.condition);
    LLVMBuildCondBr(generator->builder, cond_val, body_block, end_block);
    
    LLVMPositionBuilderAtEnd(generator->builder, body_block);
    generate_code(generator, node->data.for_loop.body);
    LLVMBuildBr(generator->builder, update_block);
    
    LLVMPositionBuilderAtEnd(generator->builder, update_block);
    generate_code(generator, node->data.for_loop.update);
    LLVMBuildBr(generator->builder, cond_block);
    
    LLVMPositionBuilderAtEnd(generator->builder, end_block);
    
    return NULL;
}

LLVMValueRef generate_print(LLVMGenerator* generator, ASTNode* node) {
    LLVMValueRef printf_func = LLVMGetNamedFunction(generator->module, "printf");
    LLVMValueRef format_str;
    LLVMValueRef expr_val = generate_code(generator, node->data.print.expression);
    
    if (LLVMTypeOf(expr_val) == LLVMInt32Type()) {
        format_str = LLVMBuildGlobalStringPtr(generator->builder, "%d\n", "int_fmt");
    } else if (LLVMTypeOf(expr_val) == LLVMFloatType()) {
        format_str = LLVMBuildGlobalStringPtr(generator->builder, "%f\n", "float_fmt");
    } else {
        format_str = LLVMBuildGlobalStringPtr(generator->builder, "%s\n", "str_fmt");
    }
    
    if (node->data.print.is_debug) {
        format_str = LLVMBuildGlobalStringPtr(generator->builder, "DEBUG: %s\n", "debug_fmt");
    }
    
    LLVMValueRef args[] = { format_str, expr_val };
    return LLVMBuildCall(generator->builder, printf_func, args, 2, "printf_call");
}

LLVMValueRef generate_block(LLVMGenerator* generator, ASTNode* node) {
    LLVMValueRef last_val = NULL;
    
    for (int i = 0; i < node->data.block.count; i++) {
        last_val = generate_code(generator, node->data.block.statements[i]);
    }
    
    return last_val;
}

LLVMValueRef generate_scene(LLVMGenerator* generator, ASTNode* node) {
    return generate_code(generator, node->data.scene.body);
}

LLVMValueRef generate_enter_exit(LLVMGenerator* generator, ASTNode* node) {
    LLVMValueRef printf_func = LLVMGetNamedFunction(generator->module, "printf");
    const char* action = node->data.enter_exit.is_enter ? "Entrando" : "Saindo";
    
    char message[100];
    sprintf(message, "%s: %s\n", action, node->data.enter_exit.resource_name);
    
    LLVMValueRef format_str = LLVMBuildGlobalStringPtr(generator->builder, message, "resource_fmt");
    LLVMValueRef args[] = { format_str };
    
    return LLVMBuildCall(generator->builder, printf_func, args, 1, "printf_call");
}

LLVMValueRef generate_improvise(LLVMGenerator* generator, ASTNode* node) {
    LLVMValueRef rand_func = LLVMGetNamedFunction(generator->module, "rand");
    
    LLVMValueRef rand_call = LLVMBuildCall(generator->builder, rand_func, NULL, 0, "rand_call");
    LLVMValueRef max_val = generate_code(generator, node->data.improvise.max_value);
    
    return LLVMBuildURem(generator->builder, rand_call, max_val, "rand_mod");
}

LLVMValueRef generate_code(LLVMGenerator* generator, ASTNode* node) {
    if (!node) return NULL;
    
    switch (node->type) {
        case AST_LITERAL_INT:
        case AST_LITERAL_FLOAT:
        case AST_LITERAL_STRING:
            return generate_literal(generator, node);
        case AST_VARIABLE:
            return generate_variable(generator, node);
        case AST_BINARY_OP:
            return generate_binary_op(generator, node);
        case AST_DECLARATION:
            return generate_declaration(generator, node);
        case AST_ASSIGNMENT:
            return generate_assignment(generator, node);
        case AST_IF_ELSE:
            return generate_if_else(generator, node);
        case AST_WHILE_LOOP:
            return generate_while_loop(generator, node);
        case AST_FOR_LOOP:
            return generate_for_loop(generator, node);
        case AST_PRINT:
        case AST_NARRATE:
            return generate_print(generator, node);
        case AST_BLOCK:
            return generate_block(generator, node);
        case AST_SCENE:
            return generate_scene(generator, node);
        case AST_ENTER_EXIT:
            return generate_enter_exit(generator, node);
        case AST_IMPROVISE:
            return generate_improvise(generator, node);
        default:
            fprintf(stderr, "Erro: Tipo de nó AST desconhecido\n");
            return NULL;
    }
}

void write_bitcode_to_file(LLVMGenerator* generator, const char* filename) {
    LLVMBuildRet(generator->builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    
    char* error = NULL;
    LLVMVerifyModule(generator->module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);
    
    if (LLVMWriteBitcodeToFile(generator->module, filename) != 0) {
        fprintf(stderr, "Erro ao escrever bitcode para arquivo %s\n", filename);
    }
}

void cleanup_llvm_generator(LLVMGenerator* generator) {
    LLVMDisposeBuilder(generator->builder);
    LLVMDisposeModule(generator->module);
    LLVMContextDispose(generator->context);
    free(generator);
}