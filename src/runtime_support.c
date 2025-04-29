#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* bool_to_string(int boolean_value) {
    return boolean_value ? "true" : "false";
}

char* concat_strings(const char* str1, const char* str2) {
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char* result = (char*)malloc(len1 + len2 + 1);
    
    if (!result) {
        fprintf(stderr, "Erro: Falha na alocação de memória\n");
        exit(1);
    }
    
    strcpy(result, str1);
    strcat(result, str2);
    
    return result;
}

// Nova função: converte int para string
char* int_to_string(int int_value) {
    char* buffer = (char*)malloc(20); // Suficiente para qualquer int de 32 bits
    if (!buffer) {
        fprintf(stderr, "Erro: Falha na alocação de memória\n");
        exit(1);
    }
    sprintf(buffer, "%d", int_value);
    return buffer;
}