# Detalhes do Compilador

O compilador da linguagem TechFlow é dividido em três etapas principais:

1. **Análise Léxica e Sintática**
   - Feita com a biblioteca PLY (Python Lex-Yacc)
   - Gera uma AST com classes para cada tipo de instrução

2. **Geração de LLVM IR**
   - Feita com `llvmlite`
   - Transforma a AST em instruções LLVM intermediárias

3. **Execução**
   - Pode ser feita com `lli out.ll` (interpretação)
   - Ou com `clang out.ll -o out` (geração de executável)

Exemplo LLVM IR gerado para `log x`:
```
%tmp = load i32, i32* %x
call i32 (i8*, ...) @printf(i8* %format_str, i32 %tmp)
```
