#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "llvm_generator.h"
#include "symbol_table.h"

extern FILE* yyin;
extern int yyparse();
extern ASTNode* root;

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s arquivo.palco\n", argv[0]);
        return 1;
    }
    
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Erro ao abrir o arquivo %s\n", argv[1]);
        return 1;
    }
    
    yyin = file;
    
    init_symbol_table();
    
    int parse_result = yyparse();
    fclose(file);
    
    if (parse_result != 0 || !root) {
        fprintf(stderr, "Erro na análise sintática\n");
        return 1;
    }
    
    LLVMGenerator* generator = init_llvm_generator("palco_module");
    generate_code(generator, root);
    write_bitcode_to_file(generator, "output.bc");
    
    printf("Compilação concluída. Bitcode gerado em output.bc\n");
    
    cleanup_llvm_generator(generator);
    free_ast_node(root);
    cleanup_symbol_table();
    
    return 0;
}