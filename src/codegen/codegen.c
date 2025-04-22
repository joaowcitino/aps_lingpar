/* src/codegen/codegen.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/ExecutionEngine.h>
#include "codegen.h"
#include "../semantic/symbol_table.h"

typedef struct {
    char *name;
    LLVMValueRef value;
    int scope;
} VarEntry;

typedef struct {
    LLVMContextRef context;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    SymbolTable *symbol_table;

    VarEntry *variables;
    int var_count;
    int var_capacity;

    int current_scope;

    LLVMValueRef current_return;

    LLVMBasicBlockRef return_block;
} CodegenContext;

static LLVMValueRef codegen_expression(CodegenContext *ctx, ASTNode *expr);
static bool codegen_declaration(CodegenContext *ctx, ASTNode *decl);
static bool codegen_function(CodegenContext *ctx, ASTNode *func_def);
static bool codegen_conditional(CodegenContext *ctx, ASTNode *conditional);
static bool codegen_for_loop(CodegenContext *ctx, ASTNode *for_loop);
static bool codegen_while_loop(CodegenContext *ctx, ASTNode *while_loop);
static LLVMValueRef codegen_function_call(CodegenContext *ctx, ASTNode *func_call);
static bool codegen_var_declaration(CodegenContext *ctx, ASTNode *var_decl);
static bool codegen_log(CodegenContext *ctx, ASTNode *log);

static CodegenContext *init_codegen_context(const char *module_name) {
    CodegenContext *ctx = (CodegenContext *)malloc(sizeof(CodegenContext));
    if (!ctx) {
        fprintf(stderr, "Erro: Falha ao alocar memória para contexto de geração de código\n");
        return NULL;
    }
    
    ctx->context = LLVMContextCreate();
    ctx->module = LLVMModuleCreateWithNameInContext(module_name, ctx->context);
    ctx->builder = LLVMCreateBuilderInContext(ctx->context);
    
    ctx->symbol_table = create_symbol_table(101);
    
    ctx->var_capacity = 100;
    ctx->var_count = 0;
    ctx->variables = (VarEntry*)malloc(ctx->var_capacity * sizeof(VarEntry));
    
    if (!ctx->variables) {
        fprintf(stderr, "Erro: Falha ao alocar memória para mapeamento de variáveis\n");
        free(ctx);
        return NULL;
    }
    
    ctx->current_scope = 0;
    ctx->current_return = NULL;
    ctx->return_block = NULL;
    
    return ctx;
}

static void free_codegen_context(CodegenContext *ctx) {
    if (!ctx) return;
    
    for (int i = 0; i < ctx->var_count; i++) {
        free(ctx->variables[i].name);
    }
    free(ctx->variables);
    
    free_symbol_table(ctx->symbol_table);
    
    LLVMDisposeBuilder(ctx->builder);
    LLVMDisposeModule(ctx->module);
    LLVMContextDispose(ctx->context);
    
    free(ctx);
}

static void add_variable(CodegenContext *ctx, const char *name, LLVMValueRef value) {
    for (int i = 0; i < ctx->var_count; i++) {
        if (ctx->variables[i].scope == ctx->current_scope && 
            strcmp(ctx->variables[i].name, name) == 0) {
            ctx->variables[i].value = value;
            return;
        }
    }
    
    if (ctx->var_count >= ctx->var_capacity) {
        ctx->var_capacity *= 2;
        ctx->variables = (VarEntry*)realloc(ctx->variables, ctx->var_capacity * sizeof(VarEntry));
        
        if (!ctx->variables) {
            fprintf(stderr, "Erro: Falha ao realocar memória para mapeamento de variáveis\n");
            exit(1);
        }
    }
    
    ctx->variables[ctx->var_count].name = strdup(name);
    ctx->variables[ctx->var_count].value = value;
    ctx->variables[ctx->var_count].scope = ctx->current_scope;
    ctx->var_count++;
}

static LLVMValueRef get_variable(CodegenContext *ctx, const char *name) {
    for (int scope = ctx->current_scope; scope >= 0; scope--) {
        for (int i = 0; i < ctx->var_count; i++) {
            if (ctx->variables[i].scope == scope && 
                strcmp(ctx->variables[i].name, name) == 0) {
                return ctx->variables[i].value;
            }
        }
    }
    
    return NULL;
}

static void enter_scope_codegen(CodegenContext *ctx) {
    ctx->current_scope++;
    enter_scope(ctx->symbol_table);
}

static void leave_scope_codegen(CodegenContext *ctx) {
    for (int i = ctx->var_count - 1; i >= 0; i--) {
        if (ctx->variables[i].scope == ctx->current_scope) {
            free(ctx->variables[i].name);
            if (i < ctx->var_count - 1) {
                memmove(&ctx->variables[i], &ctx->variables[i + 1], 
                        (ctx->var_count - i - 1) * sizeof(*ctx->variables));
            }
            ctx->var_count--;
        }
    }
    
    ctx->current_scope--;
    leave_scope(ctx->symbol_table);
}

bool generate_code(ASTNode *ast_root, const char *input_filename) {
    if (!ast_root || ast_root->type != AST_PROGRAM) {
        fprintf(stderr, "Erro: AST inválida ou nó raiz não é um programa\n");
        return false;
    }
    
    const char *module_name = input_filename;
    const char *dot = strrchr(input_filename, '.');
    if (dot) {
        char *temp = strndup(input_filename, dot - input_filename);
        module_name = temp;
    }
    
    CodegenContext *ctx = init_codegen_context(module_name);
    if (!ctx) {
        return false;
    }
    
    LLVMTypeRef printf_arg_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(), printf_arg_types, 1, true);
    LLVMValueRef printf_func = LLVMAddFunction(ctx->module, "printf", printf_type);
    
    ASTNode *current = ast_root->data.program.declarations;
    bool result = true;
    
    while (current && current->type == AST_DECLARATIONS) {
        if (current->data.declarations.declaration) {
            result &= codegen_declaration(ctx, current->data.declarations.declaration);
        }
        current = current->data.declarations.next;
    }
    
    LLVMValueRef main_func = LLVMGetNamedFunction(ctx->module, "main");
    if (!main_func) {
        LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, false);
        main_func = LLVMAddFunction(ctx->module, "main", main_type);
        
        LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
        LLVMPositionBuilderAtEnd(ctx->builder, entry);
        
        LLVMBuildRet(ctx->builder, LLVMConstInt(LLVMInt32Type(), 0, false));
    }
    
    char *error = NULL;
    LLVMVerifyModule(ctx->module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);
    
    char output_filename[256];
    snprintf(output_filename, sizeof(output_filename), "%s.bc", module_name);
    
    if (LLVMWriteBitcodeToFile(ctx->module, output_filename) != 0) {
        fprintf(stderr, "Erro: Falha ao escrever arquivo de saída '%s'\n", output_filename);
        result = false;
    }
    
    free_codegen_context(ctx);
    
    return result;
}

static bool codegen_declaration(CodegenContext *ctx, ASTNode *decl) {
    if (!decl) {
        return false;
    }
    
    switch (decl->type) {
        case AST_VAR_DECLARATION:
            return codegen_var_declaration(ctx, decl);
        
        case AST_FUNCTION_DEF:
            return codegen_function(ctx, decl);
        
        case AST_CONDITIONAL:
            return codegen_conditional(ctx, decl);
        
        case AST_FOR_LOOP:
            return codegen_for_loop(ctx, decl);
        
        case AST_WHILE_LOOP:
            return codegen_while_loop(ctx, decl);
        
        case AST_FUNCTION_CALL:
            codegen_function_call(ctx, decl);
            return true;
        
        case AST_LOG:
            return codegen_log(ctx, decl);
        
        default:
            fprintf(stderr, "Erro: Tipo de declaração desconhecido para geração de código\n");
            return false;
    }
}

static bool codegen_var_declaration(CodegenContext *ctx, ASTNode *var_decl) {
    if (!var_decl || var_decl->type != AST_VAR_DECLARATION) {
        return false;
    }
    
    char *type_name = var_decl->data.var_declaration.type->data.type.name;
    LLVMTypeRef llvm_type;
    
    if (strcmp(type_name, "byte") == 0) {
        llvm_type = LLVMInt32Type();
    } else if (strcmp(type_name, "bit") == 0) {
        llvm_type = LLVMInt1Type();
    } else if (strcmp(type_name, "chip") == 0) {
        llvm_type = LLVMPointerType(LLVMInt8Type(), 0);
    } else {
        fprintf(stderr, "Erro: Tipo desconhecido '%s' na declaração de variável\n", type_name);
        return false;
    }
    
    LLVMValueRef current_func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(ctx->builder));
    if (!current_func) {
        LLVMValueRef global_var = LLVMAddGlobal(ctx->module, llvm_type, var_decl->data.var_declaration.name);
        
        if (!var_decl->data.var_declaration.value) {
            LLVMSetInitializer(global_var, LLVMConstNull(llvm_type));
        } else {
            LLVMValueRef init_val = codegen_expression(ctx, var_decl->data.var_declaration.value);
            if (!init_val) {
                return false;
            }
            LLVMSetInitializer(global_var, init_val);
        }
        
        add_variable(ctx, var_decl->data.var_declaration.name, global_var);
    } else {
        LLVMValueRef local_var = LLVMBuildAlloca(ctx->builder, llvm_type, var_decl->data.var_declaration.name);
        
        if (var_decl->data.var_declaration.value) {
            LLVMValueRef init_val = codegen_expression(ctx, var_decl->data.var_declaration.value);
            if (!init_val) {
                return false;
            }
            LLVMBuildStore(ctx->builder, init_val, local_var);
        }
        
        add_variable(ctx, var_decl->data.var_declaration.name, local_var);
    }
    
    return true;
}

static bool codegen_function(CodegenContext *ctx, ASTNode *func_def) {
    if (!func_def || func_def->type != AST_FUNCTION_DEF) {
        return false;
    }
    
    LLVMTypeRef return_type = LLVMVoidType();
    if (func_def->data.function_def.return_stmt) {
        return_type = LLVMInt32Type();
    }
    
    int param_count = 0;
    LLVMTypeRef *param_types = NULL;
    
    if (func_def->data.function_def.params) {
        ASTNode *current = func_def->data.function_def.params;
        while (current && current->type == AST_PARAMS) {
            if (current->data.params.name) {
                param_count++;
            }
            current = current->data.params.next;
        }
        
        param_types = (LLVMTypeRef *)malloc(param_count * sizeof(LLVMTypeRef));
        if (!param_types) {
            fprintf(stderr, "Erro: Falha ao alocar memória para tipos de parâmetros\n");
            return false;
        }
        
        for (int i = 0; i < param_count; i++) {
            param_types[i] = LLVMInt32Type();
        }
    }

    LLVMTypeRef func_type = LLVMFunctionType(return_type, param_types, param_count, false);

    LLVMValueRef function = LLVMAddFunction(ctx->module, func_def->data.function_def.name, func_type);

    if (param_types) {
        free(param_types);
    }

    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "entry");
    LLVMPositionBuilderAtEnd(ctx->builder, entry);

    if (func_def->data.function_def.return_stmt) {
        ctx->return_block = LLVMAppendBasicBlock(function, "return");
    } else {
        ctx->return_block = NULL;
    }

    enter_scope_codegen(ctx);

    if (func_def->data.function_def.params) {
        ASTNode *current = func_def->data.function_def.params;
        int i = 0;
        
        while (current && current->type == AST_PARAMS) {
            if (current->data.params.name) {
                LLVMValueRef param = LLVMGetParam(function, i);

                LLVMValueRef param_alloca = LLVMBuildAlloca(ctx->builder, LLVMTypeOf(param), current->data.params.name);

                LLVMBuildStore(ctx->builder, param, param_alloca);

                add_variable(ctx, current->data.params.name, param_alloca);
                
                i++;
            }
            current = current->data.params.next;
        }
    }

    bool body_valid = true;
    ASTNode *current = func_def->data.function_def.body;
    
    while (current && current->type == AST_DECLARATIONS) {
        if (current->data.declarations.declaration) {
            body_valid &= codegen_declaration(ctx, current->data.declarations.declaration);
        }
        current = current->data.declarations.next;
    }

    if (func_def->data.function_def.return_stmt) {
        LLVMValueRef return_val = codegen_expression(ctx, func_def->data.function_def.return_stmt->data.return_stmt.expression);
        if (!return_val) {
            body_valid = false;
        } else {
            ctx->current_return = return_val;

            LLVMBuildBr(ctx->builder, ctx->return_block);

            LLVMPositionBuilderAtEnd(ctx->builder, ctx->return_block);

            LLVMBuildRet(ctx->builder, ctx->current_return);
        }
    } else {
        LLVMBuildRetVoid(ctx->builder);
    }

    leave_scope_codegen(ctx);
    
    return body_valid;
}

static bool codegen_conditional(CodegenContext *ctx, ASTNode *conditional) {
    if (!conditional || conditional->type != AST_CONDITIONAL) {
        return false;
    }
    
    ASTNode *blocks = conditional->data.conditional.blocks;
    bool result = true;

    LLVMValueRef current_func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(ctx->builder));
    if (!current_func) {
        fprintf(stderr, "Erro: Condicional fora de uma função\n");
        return false;
    }

    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_func, "if.end");
    
    while (blocks && blocks->type == AST_IF_BLOCKS) {
        if (blocks->data.if_blocks.if_block) {
            LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_func, "if.then");
            LLVMBasicBlockRef else_block = NULL;
            
            if (blocks->data.if_blocks.else_block || blocks->data.if_blocks.next) {
                else_block = LLVMAppendBasicBlock(current_func, "if.else");
            } else {
                else_block = merge_block;
            }

            LLVMValueRef condition = codegen_expression(ctx, blocks->data.if_blocks.if_block->data.if_block.condition);
            if (!condition) {
                result = false;
                break;
            }

            LLVMBuildCondBr(ctx->builder, condition, then_block, else_block);

            LLVMPositionBuilderAtEnd(ctx->builder, then_block);
            enter_scope_codegen(ctx);
            
            ASTNode *then_body = blocks->data.if_blocks.if_block->data.if_block.body;
            while (then_body && then_body->type == AST_DECLARATIONS) {
                if (then_body->data.declarations.declaration) {
                    result &= codegen_declaration(ctx, then_body->data.declarations.declaration);
                }
                then_body = then_body->data.declarations.next;
            }
            
            leave_scope_codegen(ctx);

            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(ctx->builder))) {
                LLVMBuildBr(ctx->builder, merge_block);
            }

            if (blocks->data.if_blocks.else_block) {
                LLVMPositionBuilderAtEnd(ctx->builder, else_block);
                enter_scope_codegen(ctx);
                
                ASTNode *else_body = blocks->data.if_blocks.else_block->data.else_block.body;
                while (else_body && else_body->type == AST_DECLARATIONS) {
                    if (else_body->data.declarations.declaration) {
                        result &= codegen_declaration(ctx, else_body->data.declarations.declaration);
                    }
                    else_body = else_body->data.declarations.next;
                }
                
                leave_scope_codegen(ctx);

                if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(ctx->builder))) {
                    LLVMBuildBr(ctx->builder, merge_block);
                }
            } else if (blocks->data.if_blocks.next) {
                LLVMPositionBuilderAtEnd(ctx->builder, else_block);
            }
        }
        
        blocks = blocks->data.if_blocks.next;
    }

    LLVMPositionBuilderAtEnd(ctx->builder, merge_block);
    
    return result;
}

static bool codegen_for_loop(CodegenContext *ctx, ASTNode *for_loop) {
    if (!for_loop || for_loop->type != AST_FOR_LOOP) {
        return false;
    }

    LLVMValueRef current_func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(ctx->builder));
    if (!current_func) {
        fprintf(stderr, "Erro: Loop for fora de uma função\n");
        return false;
    }

    enter_scope_codegen(ctx);

    // LLVMBasicBlockRef preheader_block = LLVMGetInsertBlock(ctx->builder);
    LLVMBasicBlockRef loop_block = LLVMAppendBasicBlock(current_func, "loop");
    LLVMBasicBlockRef after_block = LLVMAppendBasicBlock(current_func, "loop.end");

    LLVMValueRef expr = codegen_expression(ctx, for_loop->data.for_loop.expression);
    if (!expr) {
        leave_scope_codegen(ctx);
        return false;
    }

    LLVMTypeRef expr_type = LLVMTypeOf(expr);

    if (LLVMGetTypeKind(expr_type) == LLVMArrayTypeKind || 
        LLVMGetTypeKind(expr_type) == LLVMPointerTypeKind) {
        fprintf(stderr, "Aviso: Iteração sobre arrays não implementada\n");
    } else {
        LLVMValueRef counter = LLVMBuildAlloca(ctx->builder, LLVMInt32Type(), "loop.counter");
        LLVMBuildStore(ctx->builder, LLVMConstInt(LLVMInt32Type(), 0, false), counter);

        LLVMValueRef iterator = LLVMBuildAlloca(ctx->builder, LLVMInt32Type(), for_loop->data.for_loop.iterator);

        LLVMBuildBr(ctx->builder, loop_block);

        LLVMPositionBuilderAtEnd(ctx->builder, loop_block);

        LLVMValueRef current_count = LLVMBuildLoad2(ctx->builder, LLVMTypeOf(counter), counter, "current.count");

        LLVMBuildStore(ctx->builder, current_count, iterator);

        add_variable(ctx, for_loop->data.for_loop.iterator, iterator);

        LLVMValueRef cond = LLVMBuildICmp(ctx->builder, LLVMIntSLT, current_count, expr, "loop.cond");

        LLVMBuildCondBr(ctx->builder, cond, loop_block, after_block);

        ASTNode *body = for_loop->data.for_loop.body;
        bool body_valid = true;
        
        while (body && body->type == AST_DECLARATIONS) {
            if (body->data.declarations.declaration) {
                body_valid &= codegen_declaration(ctx, body->data.declarations.declaration);
            }
            body = body->data.declarations.next;
        }

        LLVMValueRef next_count = LLVMBuildAdd(ctx->builder, current_count, 
                                              LLVMConstInt(LLVMInt32Type(), 1, false), "next.count");
        LLVMBuildStore(ctx->builder, next_count, counter);

        LLVMBuildBr(ctx->builder, loop_block);
    }

    LLVMPositionBuilderAtEnd(ctx->builder, after_block);

    leave_scope_codegen(ctx);
    
    return true;
}

static bool codegen_while_loop(CodegenContext *ctx, ASTNode *while_loop) {
    if (!while_loop || while_loop->type != AST_WHILE_LOOP) {
        return false;
    }

    LLVMValueRef current_func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(ctx->builder));
    if (!current_func) {
        fprintf(stderr, "Erro: Loop while fora de uma função\n");
        return false;
    }

    LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(current_func, "while.cond");
    LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(current_func, "while.body");
    LLVMBasicBlockRef end_block = LLVMAppendBasicBlock(current_func, "while.end");

    LLVMBuildBr(ctx->builder, cond_block);

    LLVMPositionBuilderAtEnd(ctx->builder, cond_block);

    LLVMValueRef condition = codegen_expression(ctx, while_loop->data.while_loop.condition);
    if (!condition) {
        return false;
    }

    LLVMBuildCondBr(ctx->builder, condition, body_block, end_block);

    LLVMPositionBuilderAtEnd(ctx->builder, body_block);
    enter_scope_codegen(ctx);
    
    ASTNode *body = while_loop->data.while_loop.body;
    bool body_valid = true;
    
    while (body && body->type == AST_DECLARATIONS) {
        if (body->data.declarations.declaration) {
            body_valid &= codegen_declaration(ctx, body->data.declarations.declaration);
        }
        body = body->data.declarations.next;
    }
    
    leave_scope_codegen(ctx);

    LLVMBuildBr(ctx->builder, cond_block);

    LLVMPositionBuilderAtEnd(ctx->builder, end_block);
    
    return body_valid;
}

static LLVMValueRef codegen_function_call(CodegenContext *ctx, ASTNode *func_call) {
    if (!func_call || func_call->type != AST_FUNCTION_CALL) {
        return NULL;
    }

    LLVMValueRef func = LLVMGetNamedFunction(ctx->module, func_call->data.function_call.name);
    if (!func) {
        fprintf(stderr, "Erro: Função '%s' não definida\n", func_call->data.function_call.name);
        return NULL;
    }

    int arg_count = 0;
    ASTNode *current = func_call->data.function_call.args;
    while (current && current->type == AST_ARGS) {
        arg_count++;
        current = current->data.args.next;
    }

    if (arg_count != (int)LLVMCountParams(func)) {
        fprintf(stderr, "Erro: Número incorreto de argumentos na chamada de '%s'. Esperado %u, recebido %d\n", 
                func_call->data.function_call.name, LLVMCountParams(func), arg_count);
        return NULL;
    }

    LLVMValueRef *args = NULL;
    if (arg_count > 0) {
        args = (LLVMValueRef *)malloc(arg_count * sizeof(LLVMValueRef));
        if (!args) {
            fprintf(stderr, "Erro: Falha ao alocar memória para argumentos\n");
            return NULL;
        }
        
        current = func_call->data.function_call.args;
        for (int i = 0; i < arg_count; i++) {
            args[i] = codegen_expression(ctx, current->data.args.arg);
            if (!args[i]) {
                free(args);
                return NULL;
            }
            current = current->data.args.next;
        }
    }

    LLVMTypeRef func_type = LLVMGetElementType(LLVMTypeOf(func));
    LLVMValueRef result = LLVMBuildCall2(ctx->builder, func_type, func, args, arg_count, "");

    if (args) {
        free(args);
    }
    
    return result;
}

static bool codegen_log(CodegenContext *ctx, ASTNode *log) {
    if (!log || log->type != AST_LOG) {
        return false;
    }

    LLVMValueRef expr = codegen_expression(ctx, log->data.log.expression);
    if (!expr) {
        return false;
    }

    LLVMValueRef printf_func = LLVMGetNamedFunction(ctx->module, "printf");
    if (!printf_func) {
        LLVMTypeRef printf_arg_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
        LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(), printf_arg_types, 1, true);
        printf_func = LLVMAddFunction(ctx->module, "printf", printf_type);
    }

    const char *format;
    LLVMTypeRef expr_type = LLVMTypeOf(expr);
    
    if (LLVMGetTypeKind(expr_type) == LLVMIntegerTypeKind) {
        if (LLVMGetIntTypeWidth(expr_type) == 1) {
            format = "%s\n";

            LLVMValueRef true_str = LLVMBuildGlobalStringPtr(ctx->builder, "true", "true_str");
            LLVMValueRef false_str = LLVMBuildGlobalStringPtr(ctx->builder, "false", "false_str");
            expr = LLVMBuildSelect(ctx->builder, expr, true_str, false_str, "bool_str");
        } else {
            format = "%d\n";
        }
    } else if (LLVMGetTypeKind(expr_type) == LLVMPointerTypeKind) {
        format = "%s\n";
    } else {
        format = "Valor desconhecido\n";
    }

    LLVMValueRef format_str = LLVMBuildGlobalStringPtr(ctx->builder, format, "format");

    LLVMValueRef args[] = { format_str, expr };
    LLVMBuildCall2(ctx->builder, LLVMGetElementType(LLVMTypeOf(printf_func)), printf_func, args, 2, "");
    
    return true;
}

static LLVMValueRef codegen_expression(CodegenContext *ctx, ASTNode *expr) {
    if (!expr) {
        return NULL;
    }
    
    switch (expr->type) {
        case AST_IDENTIFIER: {
            LLVMValueRef var = get_variable(ctx, expr->data.identifier.name);
            if (!var) {
                fprintf(stderr, "Erro: Variável '%s' não definida\n", expr->data.identifier.name);
                return NULL;
            }
            return LLVMBuildLoad2(ctx->builder, LLVMTypeOf(var), var, expr->data.identifier.name);
            break;
        }
        
        case AST_NUMBER:
            return LLVMConstInt(LLVMInt32Type(), expr->data.number.value, false);
            break;
        
        case AST_STRING:
            return LLVMBuildGlobalStringPtr(ctx->builder, expr->data.string.value, "string");
            break;
        
        case AST_ARRAY:
            fprintf(stderr, "Aviso: Arrays não implementados completamente\n");
            return LLVMConstNull(LLVMPointerType(LLVMInt32Type(), 0));
            break;
        
        case AST_FUNCTION_CALL:
            return codegen_function_call(ctx, expr);
            break;
        
        case AST_BINARY_EXPR: {
            LLVMValueRef left = codegen_expression(ctx, expr->data.binary_expr.left);
            LLVMValueRef right = codegen_expression(ctx, expr->data.binary_expr.right);
            
            if (!left || !right) {
                return NULL;
            }
            
            OperatorType op = expr->data.binary_expr.op->data.op.op_type;
            
            switch (op) {
                case OP_PLUS:
                    return LLVMBuildAdd(ctx->builder, left, right, "add");
                
                case OP_MINUS:
                    return LLVMBuildSub(ctx->builder, left, right, "sub");
                
                case OP_MULTIPLY:
                    return LLVMBuildMul(ctx->builder, left, right, "mul");
                
                case OP_DIVIDE:
                    return LLVMBuildSDiv(ctx->builder, left, right, "div");

                case OP_EQUAL:
                    return LLVMBuildICmp(ctx->builder, LLVMIntEQ, left, right, "eq");
                
                case OP_NOT_EQUAL:
                    return LLVMBuildICmp(ctx->builder, LLVMIntNE, left, right, "ne");

                case OP_GREATER:
                    return LLVMBuildICmp(ctx->builder, LLVMIntSGT, left, right, "gt");
                
                case OP_LESS:
                    return LLVMBuildICmp(ctx->builder, LLVMIntSLT, left, right, "lt");
                
                case OP_GREATER_EQUAL:
                    return LLVMBuildICmp(ctx->builder, LLVMIntSGE, left, right, "ge");
                
                case OP_LESS_EQUAL:
                    return LLVMBuildICmp(ctx->builder, LLVMIntSLE, left, right, "le");

                case OP_AND:
                    return LLVMBuildAnd(ctx->builder, left, right, "and");
                
                case OP_OR:
                    return LLVMBuildOr(ctx->builder, left, right, "or");
                
                default:
                    fprintf(stderr, "Erro: Operador desconhecido\n");
                    return NULL;
            }
            break;
        }
        
        default:
            fprintf(stderr, "Erro: Tipo de expressão desconhecido\n");
            return NULL;
    }
}