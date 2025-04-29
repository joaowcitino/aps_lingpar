#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "llvm_generator.h"

extern int yyparse();
extern FILE* yyin;
extern struct Node* ast_root;

void execute_ast(struct Node* node);

void print_usage(const char* program_name) {
    printf("Uso: %s <arquivo.tf> [opções]\n", program_name);
    printf("Opções:\n");
    printf("  --interpret    Interpretar o programa (padrão)\n");
    printf("  --compile      Compilar o programa para LLVM IR\n");
    printf("  --output=<arquivo>  Especificar arquivo de saída para compilação\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    char* input_file = NULL;
    char* output_file = "output.bc";
    bool do_compile = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--interpret") == 0) {
            do_compile = false;
        } else if (strcmp(argv[i], "--compile") == 0) {
            do_compile = true;
        } else if (strncmp(argv[i], "--output=", 9) == 0) {
            output_file = argv[i] + 9;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        } else {
            printf("Opção desconhecida: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (!input_file) {
        printf("Erro: Nenhum arquivo de entrada especificado\n");
        print_usage(argv[0]);
        return 1;
    }
    
    FILE* file = fopen(input_file, "r");
    if (!file) {
        fprintf(stderr, "Erro: não foi possível abrir o arquivo '%s'\n", input_file);
        return 1;
    }
    
    yyin = file;
    
    printf("Iniciando análise sintática...\n");
    int parse_result = yyparse();
    
    fclose(file);
    
    if (parse_result == 0) {
        printf("Análise sintática concluída com sucesso!\n");
        
        if (ast_root != NULL) {
            if (do_compile) {
                printf("Compilando programa para LLVM IR (%s)...\n", output_file);
                generate_llvm_code(ast_root, output_file);
                printf("Compilação concluída.\n");
                
                printf("\nPara compilar para um executável:\n");
                printf("clang %s -o programa\n", output_file);
                printf("./programa\n");
            } else {
                printf("Executando programa...\n");
                execute_ast(ast_root);
                printf("Execução concluída.\n");
            }
        } else {
            fprintf(stderr, "Erro: AST vazia\n");
            return 1;
        }
    } else {
        fprintf(stderr, "Erro durante a análise sintática\n");
        return 1;
    }
    
    return 0;
}