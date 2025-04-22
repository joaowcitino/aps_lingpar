#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser/ast.h"
#include "semantic/semantic.h"
#include "codegen/codegen.h"

extern FILE *yyin;
extern int yyparse(void);
extern ASTNode *ast_root;

void print_usage(char *program_name) {
    printf("Uso: %s <arquivo.tf>\n", program_name);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (!input_file) {
        fprintf(stderr, "Erro: Não foi possível abrir o arquivo '%s'\n", argv[1]);
        return 1;
    }

    yyin = input_file;

    printf("Analisando arquivo '%s'...\n", argv[1]);
    int parse_result = yyparse();

    fclose(input_file);
    
    if (parse_result != 0 || !ast_root) {
        fprintf(stderr, "Erro: Falha na análise sintática\n");
        return 1;
    }
    
    printf("Análise sintática concluída com sucesso\n");

    printf("Executando análise semântica...\n");
    if (!analyze_semantics(ast_root)) {
        fprintf(stderr, "Erro: Falha na análise semântica\n");
        free_ast(ast_root);
        return 1;
    }
    
    printf("Análise semântica concluída com sucesso\n");

    printf("Gerando código LLVM...\n");
    if (!generate_code(ast_root, argv[1])) {
        fprintf(stderr, "Erro: Falha na geração de código\n");
        free_ast(ast_root);
        return 1;
    }
    
    printf("Código LLVM gerado com sucesso\n");

    free_ast(ast_root);
    
    return 0;
}