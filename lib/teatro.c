#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* nome;
    void* conteudo;
    size_t tamanho;
} Cenario;

typedef struct {
    char* nome;
    int (*acao)(void*);
} Ator;

Cenario* criar_cenario(const char* nome, size_t tamanho) {
    Cenario* cenario = malloc(sizeof(Cenario));
    cenario->nome = strdup(nome);
    cenario->conteudo = malloc(tamanho);
    cenario->tamanho = tamanho;
    return cenario;
}

void destruir_cenario(Cenario* cenario) {
    free(cenario->nome);
    free(cenario->conteudo);
    free(cenario);
}

Ator* criar_ator(const char* nome, int (*acao)(void*)) {
    Ator* ator = malloc(sizeof(Ator));
    ator->nome = strdup(nome);
    ator->acao = acao;
    return ator;
}

void destruir_ator(Ator* ator) {
    free(ator->nome);
    free(ator);
}

int encenar(Ator* ator, void* dados) {
    printf("Ator '%s' está encenando...\n", ator->nome);
    return ator->acao(dados);
}

void mudar_cenario(Cenario* cenario) {
    printf("Cenário mudado para '%s'\n", cenario->nome);
}

void descrever_cenario(Cenario* cenario) {
    printf("Cenário: %s\n", cenario->nome);
}

void descrever_ator(Ator* ator) {
    printf("Ator: %s\n", ator->nome);
}