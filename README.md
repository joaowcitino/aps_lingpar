# TechFlow: Uma Linguagem de Programação Inspirada no Fluxo Tecnológico

## Visão Geral

TechFlow é uma linguagem de programação de propósito geral projetada para ser intuitiva e expressiva, com uma sintaxe que reflete conceitos tecnológicos modernos. Desenvolvida como parte do curso de Linguagens e Paradigmas do bacharelado em Ciência da Computação, a TechFlow reimagina os paradigmas de programação tradicionais através de uma terminologia inspirada no fluxo de dados e processos tecnológicos.

## Motivação

A inspiração para TechFlow surgiu da observação de que muitas linguagens de programação utilizam terminologia abstrata ou desconectada dos conceitos que os programadores modernos utilizam no dia a dia. Por exemplo, por que usar "if/else" quando podemos usar "ping/pong" para refletir a ideia de verificação e resposta? Por que "while" quando "stream" captura melhor a ideia de um fluxo contínuo de operações?

TechFlow busca criar essa ponte, tornando a sintaxe da linguagem mais alinhada com o vocabulário técnico contemporâneo, facilitando tanto o aprendizado quanto a expressão de lógica computacional.

## Características da Linguagem

### Palavras-chave Intuitivas

TechFlow reimagina os comandos tradicionais:

- `boot`/`shutdown` - Início e fim do programa (substitui main/return)
- `ping`/`pong` - Estruturas condicionais (substitui if/else)
- `stream` - Loops while
- `byte` - Declaração de variáveis
- `log` - Saída de dados (substitui print)
- `select`/`when`/`otherwise` - Estrutura de seleção (substitui switch/case)
- `repeat`/`until` - Loop do-while

### Tipagem Estática

A linguagem oferece três tipos básicos com nomes modernos:
- `i32` - Números inteiros de 32 bits
- `bool` - Valores booleanos
- `str` - Strings de texto

### Operadores Expressivos

Além dos operadores matemáticos e lógicos tradicionais, TechFlow traz:
- `++` - Operador de concatenação para strings
- Comparadores relacionais: `<`, `>`, `<=`, `>=`, `==`, `!=`
- Operadores lógicos: `&&`, `||`, `!`

## Tecnologias Utilizadas

TechFlow é implementado usando uma combinação de tecnologias modernas de compilação:

1. **Análise Léxica**: Implementada com **Flex** para tokenização eficiente do código fonte
2. **Análise Sintática**: Construída com **Bison** para parsing e geração da árvore sintática abstrata (AST)
3. **Interpretação**: Um interpretador implementado em C que executa a AST diretamente
4. **Compilação**: Geração de código LLVM IR, que permite compilação para múltiplas plataformas

## Estrutura do Projeto

```
techflow/
├── bin/                # Executáveis compilados
├── docs/               # Documentação
│   ├── ebnf.md         # Especificação EBNF da linguagem
│   └── llvm_guide.md   # Guia de compilação LLVM
├── examples/           # Exemplos de programas TechFlow
├── python/             # Implementação Python (versão inicial)
│   └── techflow/       # Módulos Python
├── src/                # Código-fonte C/LLVM
│   ├── lexer.l         # Analisador léxico (Flex)
│   ├── parser.y        # Analisador sintático (Bison)
│   ├── interpreter.c   # Interpretador
│   ├── llvm_generator.c # Gerador de código LLVM
│   └── runtime_support.c # Funções de runtime
└── Makefile            # Build system
```

## Instalação e Uso

### Pré-requisitos

```bash
# Ubuntu/Debian
sudo apt-get install gcc flex bison llvm-dev
```

### Compilação

```bash
# Clonar o repositório
git clone https://github.com/joaowcitino/techflow.git
cd techflow

# Compilar o projeto
make
```

### Executando Programas TechFlow

Existem três formas de executar programas TechFlow:

#### 1. Interpretação (mais rápida para desenvolvimento)

```bash
./bin/techflow_compiler examples/teste.tf --interpret
```

#### 2. Compilação para LLVM IR

```bash
./bin/techflow_compiler examples/teste.tf --compile
```

#### 3. Compilação completa e execução

```bash
make test-run
```

## Exemplos

### Hello World

```
boot
    log("Hello, TechFlow!");
shutdown
```

### Cálculo Fatorial

```
boot
    byte n: i32 = 5;
    byte factorial: i32 = 1;
    
    stream(n > 0) then
        factorial = factorial * n;
        n = n - 1;
    end
    
    log("O fatorial é: " ++ factorial);
shutdown
```

### Estrutura Condicional

```
boot
    byte idade: i32;
    log("Digite sua idade:");
    idade = reader();
    
    ping(idade >= 18) then
        log("Você é maior de idade");
    pong then
        log("Você é menor de idade");
    end
shutdown
```

## Sintaxe EBNF

A definição formal da linguagem TechFlow está disponível no arquivo [docs/ebnf.md](docs/ebnf.md).

## Resolução de Problemas

### Erro: Symbols not found ao usar lli

Se ao executar `lli output.bc` você receber o erro:

```
JIT session error: Symbols not found: [ concat_strings, int_to_string ]
```

Isso ocorre porque o LLVM Interpreter (lli) não consegue encontrar as funções de runtime. Este é um comportamento esperado, pois essas funções estão definidas em `runtime_support.c` e não estão vinculadas ao bitcode. Para executar o programa completo:

```bash
make test-run  # Compila e executa o programa
```

## Trabalhos Futuros

- Implementação de funções e procedimentos
- Suporte a arrays e estruturas de dados
- Biblioteca padrão para operações comuns
- Otimizações de compilação LLVM avançadas

## Sobre o Autor

**João Whitaker Citino**  
Estudante de Ciência da Computação  
Instituto de Ensino Superior - Insper  
Projeto desenvolvido para a disciplina de Linguagens e Paradigmas