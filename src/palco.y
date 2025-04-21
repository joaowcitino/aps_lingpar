%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

extern int yylex();
extern int yyparse();
extern FILE *yyin;
void yyerror(const char *s);

LLVMContextRef context;
LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMValueRef function;

typedef struct {
    char *name;
    LLVMValueRef value;
    int type;
} Symbol;

#define MAX_SYMBOLS 100
Symbol symbols[MAX_SYMBOLS];
int symbol_count = 0;

void init_llvm();
LLVMValueRef find_symbol(char *name);
void add_symbol(char *name, LLVMValueRef value, int type);
%}

%union {
    int num;
    double fnum;
    char *str;
    LLVMValueRef val;
}

%token APRESENTAR ENCERRAR
%token PERSONAGEM PROTAGONISTA COADJUVANTE CENARIO
%token ATO CENA
%token SE SENAO REPETIR CAMINHAR
%token FALA NARRAR ENTRA SAI IMPROVISO
%token <num> NUMERO
%token <fnum> DECIMAL
%token <str> STRING IDENTIFICADOR
%token MAIS MENOS VEZES DIVIDE
%token ATRIB IGUAL DIFERENTE MENOR MAIOR MENOR_IGUAL MAIOR_IGUAL
%token E_LOGICO OU_LOGICO
%token ABRE_PAREN FECHA_PAREN ABRE_CHAVE FECHA_CHAVE
%token PONTO_VIRGULA VIRGULA

%type <val> expr termo fator declaracao atribuicao comando lista_comandos bloco programa

%%

programa
    : APRESENTAR bloco ENCERRAR {
        $$ = $2;
        printf("Programa analisado com sucesso!\n");
        LLVMDumpModule(module);
        LLVMWriteBitcodeToFile(module, "output.bc");
    }
    ;

bloco
    : ABRE_CHAVE lista_comandos FECHA_CHAVE {
        $$ = $2;
    }
    ;

lista_comandos
    : lista_comandos comando {
        $$ = $2;
    }
    | {
        $$ = NULL;
    }
    ;

comando
    : declaracao PONTO_VIRGULA {
        $$ = $1;
    }
    | atribuicao PONTO_VIRGULA {
        $$ = $1;
    }
    | condicional {
        $$ = NULL;
    }
    | loop {
        $$ = NULL;
    }
    | saida PONTO_VIRGULA {
        $$ = NULL;
    }
    | bloco_ato {
        $$ = NULL;
    }
    | bloco_cena {
        $$ = NULL;
    }
    | entra_sai PONTO_VIRGULA {
        $$ = NULL;
    }
    | improviso PONTO_VIRGULA {
        $$ = NULL;
    }
    ;

declaracao
    : PERSONAGEM IDENTIFICADOR {
        LLVMValueRef val = LLVMBuildAlloca(builder, LLVMInt32Type(), $2);
        add_symbol($2, val, 0);
        $$ = val;
    }
    | PERSONAGEM IDENTIFICADOR ATRIB expr {
        LLVMValueRef val = LLVMBuildAlloca(builder, LLVMInt32Type(), $2);
        LLVMBuildStore(builder, $4, val);
        add_symbol($2, val, 0);
        $$ = val;
    }
    | PROTAGONISTA IDENTIFICADOR {
        LLVMValueRef val = LLVMBuildAlloca(builder, LLVMInt32Type(), $2);
        add_symbol($2, val, 0);
        $$ = val;
    }
    | COADJUVANTE IDENTIFICADOR {
        LLVMValueRef val = LLVMBuildAlloca(builder, LLVMFloatType(), $2);
        add_symbol($2, val, 1);
        $$ = val;
    }
    | CENARIO IDENTIFICADOR {
        LLVMValueRef val = LLVMBuildAlloca(builder, LLVMPointerType(LLVMInt8Type(), 0), $2);
        add_symbol($2, val, 2);
        $$ = val;
    }
    ;

atribuicao
    : IDENTIFICADOR ATRIB expr {
        LLVMValueRef var = find_symbol($1);
        if (var != NULL) {
            LLVMBuildStore(builder, $3, var);
            $$ = var;
        } else {
            yyerror("Personagem não declarado");
            $$ = NULL;
        }
    }
    ;

condicional
    : SE ABRE_PAREN expr FECHA_PAREN bloco_ato {
    }
    | SE ABRE_PAREN expr FECHA_PAREN bloco_ato SENAO bloco_ato {
    }
    ;

loop
    : REPETIR ABRE_PAREN expr FECHA_PAREN bloco_ato {
    }
    | CAMINHAR ABRE_PAREN atribuicao PONTO_VIRGULA expr PONTO_VIRGULA atribuicao FECHA_PAREN bloco_ato {
    }
    ;

saida
    : FALA ABRE_PAREN expr FECHA_PAREN {
    }
    | NARRAR ABRE_PAREN expr FECHA_PAREN {
    }
    ;

bloco_ato
    : ATO bloco {
        $$ = $2;
    }
    | bloco {
        $$ = $1;
    }
    ;

bloco_cena
    : CENA IDENTIFICADOR bloco {
    }
    ;

entra_sai
    : ENTRA IDENTIFICADOR {
    }
    | SAI IDENTIFICADOR {
    }
    ;

improviso
    : IMPROVISO ABRE_PAREN expr FECHA_PAREN {
    }
    ;

expr
    : termo {
        $$ = $1;
    }
    | expr MAIS termo {
        $$ = LLVMBuildAdd(builder, $1, $3, "addtmp");
    }
    | expr MENOS termo {
        $$ = LLVMBuildSub(builder, $1, $3, "subtmp");
    }
    ;

termo
    : fator {
        $$ = $1;
    }
    | termo VEZES fator {
        $$ = LLVMBuildMul(builder, $1, $3, "multmp");
    }
    | termo DIVIDE fator {
        $$ = LLVMBuildSDiv(builder, $1, $3, "divtmp");
    }
    ;

fator
    : NUMERO {
        $$ = LLVMConstInt(LLVMInt32Type(), $1, 0);
    }
    | DECIMAL {
        $$ = LLVMConstReal(LLVMFloatType(), $1);
    }
    | IDENTIFICADOR {
        LLVMValueRef var = find_symbol($1);
        if (var != NULL) {
            $$ = LLVMBuildLoad(builder, var, $1);
        } else {
            yyerror("Personagem não declarado");
            $$ = LLVMConstInt(LLVMInt32Type(), 0, 0);
        }
    }
    | STRING {
        $$ = LLVMBuildGlobalStringPtr(builder, $1, "str");
    }
    | ABRE_PAREN expr FECHA_PAREN {
        $$ = $2;
    }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro: %s\n", s);
}

void init_llvm() {
    LLVMInitializeCore(LLVMGetGlobalPassRegistry());
    LLVMLinkInMCJIT();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    
    context = LLVMContextCreate();
    module = LLVMModuleCreateWithNameInContext("palco_module", context);
    builder = LLVMCreateBuilderInContext(context);
    
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    function = LLVMAddFunction(module, "main", main_type);
    
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
}

LLVMValueRef find_symbol(char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i].name, name) == 0) {
            return symbols[i].value;
        }
    }
    return NULL;
}

void add_symbol(char *name, LLVMValueRef value, int type) {
    if (symbol_count < MAX_SYMBOLS) {
        symbols[symbol_count].name = strdup(name);
        symbols[symbol_count].value = value;
        symbols[symbol_count].type = type;
        symbol_count++;
    } else {
        fprintf(stderr, "Erro: Tabela de símbolos cheia\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s arquivo.palco\n", argv[0]);
        return 1;
    }
    
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "Não foi possível abrir o arquivo %s\n", argv[1]);
        return 1;
    }
    
    yyin = f;
    init_llvm();
    yyparse();
    
    fclose(f);
    
    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    
    char *error = NULL;
    LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);
    
    return 0;
}