#ifndef PALCO_H
#define PALCO_H

#include "../src/ast.h"
#include "../src/llvm_generator.h"
#include "../src/symbol_table.h"

int compile_palco_file(const char* filename, const char* output_filename);
int run_palco_file(const char* filename);

#endif