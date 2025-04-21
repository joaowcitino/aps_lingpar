#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init_palco_stdlib() {
    srand(time(NULL));
}

int palco_improviso(int max) {
    return rand() % max;
}

void palco_fala_int(int value) {
    printf("%d\n", value);
}

void palco_fala_float(float value) {
    printf("%f\n", value);
}

void palco_fala_string(const char* value) {
    printf("%s\n", value);
}

void palco_narrar_int(int value) {
    printf("DEBUG: %d\n", value);
}

void palco_narrar_float(float value) {
    printf("DEBUG: %f\n", value);
}

void palco_narrar_string(const char* value) {
    printf("DEBUG: %s\n", value);
}

void* palco_entra(const char* name, size_t size) {
    printf("Entrando: %s\n", name);
    return malloc(size);
}

void palco_sai(const char* name, void* ptr) {
    printf("Saindo: %s\n", name);
    free(ptr);
}