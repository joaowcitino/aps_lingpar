# Estrutura do Projeto

```
techflow/
├── lexer.py              # Analisador léxico com PLY
├── parser.py             # Parser e construção da AST
├── interpreter.py        # Interpretador direto da AST (opcional)
├── llvm_compiler.py      # Compilador LLVM com llvmlite
├── main.py               # Entrada principal do compilador
├── tests/                # Casos de teste (.tf)
├── out.ll                # LLVM IR gerado
└── docs/                 # Documentação
```

O fluxo de compilação segue a arquitetura:
```
[.tf] → Lexer/Parser → AST → LLVM Compiler → LLVM IR → [lli | clang]
```
