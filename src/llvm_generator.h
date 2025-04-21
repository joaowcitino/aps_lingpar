#ifndef LLVM_GENERATOR_H
#define LLVM_GENERATOR_H

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include "ast.h"

typedef struct {
    LLVMContextRef context;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMValueRef current_function;
} LLVMGenerator;

LLVMGenerator* init_llvm_generator(const char* module_name);
LLVMValueRef generate_code(LLVMGenerator* generator, ASTNode* node);
void write_bitcode_to_file(LLVMGenerator* generator, const char* filename);
void cleanup_llvm_generator(LLVMGenerator* generator);

#endif