# Guia de Compilação LLVM para TechFlow

Este guia mostra como usar o compilador TechFlow para gerar código LLVM e executá-lo.

## Pré-requisitos

Antes de começar, instale as bibliotecas de desenvolvimento do LLVM:

```bash
# Ubuntu/Debian
sudo apt-get install llvm-dev llvm-runtime

# Fedora
sudo dnf install llvm-devel

# macOS com Homebrew
brew install llvm
```

## Compilando um programa TechFlow para LLVM IR

Para compilar um programa TechFlow para LLVM IR, use a opção `--compile`:

```bash
./bin/techflow examples/teste.tf --compile
```

Por padrão, isso gera um arquivo `output.bc` no diretório atual. Você pode especificar um nome de arquivo diferente:

```bash
./bin/techflow examples/teste.tf --compile --output=meu_programa.bc
```

## Executando o código LLVM compilado

Existem várias maneiras de executar o código LLVM compilado:

### 1. Usando o interpretador LLVM

```bash
lli output.bc
```

### 2. Compilando para código nativo e executando

```bash
# Compilar para arquivo objeto
llc -filetype=obj output.bc -o output.o

# Vincular com as funções de runtime e criar executável
gcc output.o src/runtime_support.o -o meu_programa

# Executar
./meu_programa
```

### 3. Usando a regra do Makefile

```bash
make test-run
```

Esta regra compila seu programa, vincula com as funções de runtime necessárias e executa automaticamente.

## Depuração e Inspeção

Para examinar o código LLVM IR gerado:

```bash
# Ver código LLVM IR legível por humanos
llvm-dis output.bc -o output.ll
cat output.ll

# Verificar se o código LLVM é válido
llvm-as output.ll
```

## Otimizações

Para aplicar otimizações ao código LLVM:

```bash
# Aplicar otimizações de nível O2
opt -O2 output.bc -o output_optimized.bc
```

## Características Suportadas

A implementação atual suporta:

- **Tipos de dados**:
  - `i32`: Inteiros de 32 bits
  - `bool`: Valores booleanos (true/false)
  - `str`: Strings de texto

- **Estruturas de controle**:
  - `ping/pong`: Estruturas condicionais (if/else)
  - `stream`: Loops while
  - `repeat/until`: Loops do-while
  - `select/when/otherwise`: Estruturas de seleção múltipla (switch/case)

- **Operações**:
  - Aritméticas: `+`, `-`, `*`, `/`, `%`
  - Relacionais: `<`, `>`, `<=`, `>=`, `==`, `!=`
  - Lógicas: `&&`, `||`, `!`
  - Concatenação: `++`

- **I/O**:
  - `log()`: Para saída de dados

## Personalização e Extensão

Para estender a implementação, modifique o arquivo `src/llvm_generator.c` para adicionar suporte a novos recursos da linguagem. Áreas para possíveis extensões incluem:

- Funções e procedimentos
- Novos tipos de dados
- Arrays e estruturas
- Bibliotecas padrão
- Importação de módulos

## Detalhes de Implementação

O gerador LLVM converte cada construção da linguagem TechFlow em instruções LLVM IR:

- **Variáveis**: Alocadas na pilha usando `alloca`
- **Expressões**: Convertidas em operações LLVM correspondentes
- **Estruturas de controle**: Implementadas usando blocos básicos e instruções de branch
- **Strings**: Implementadas como ponteiros globais para arrays de caracteres
- **Concatenação**: Implementada através de chamadas a funções de runtime

## Solução de Problemas

- **Erro de segmentação**: Verifique a inicialização das variáveis
- **Erros de tipo**: Verifique se as operações são realizadas com tipos compatíveis
- **Erros de execução**: Compile com símbolos de depuração (`-g`) e use ferramentas como gdb

## Recursos Adicionais

- [Documentação LLVM](https://llvm.org/docs/)
- [Tutorial Kaleidoscope LLVM](https://llvm.org/docs/tutorial/index.html)
- [LLVM Language Reference](https://llvm.org/docs/LangRef.html)