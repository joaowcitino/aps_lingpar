#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdbool.h>
#include "../parser/ast.h"

bool generate_code(ASTNode *ast_root, const char *input_filename);

#endif