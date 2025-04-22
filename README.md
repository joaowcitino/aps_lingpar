# 🧠 TechFlow – Uma Linguagem de Programação Inclusiva

TechFlow é uma linguagem de programação educacional e inclusiva, projetada para ensinar os conceitos fundamentais de programação com uma sintaxe acessível e divertida, inspirada em termos tecnológicos como `boot`, `ping`, `module`, `chip`, etc.

Ela suporta **variáveis**, **condicionais**, **laços**, **funções**, **expressões matemáticas** e gera **LLVM IR** real via `llvmlite`, cumprindo os critérios para nota **A+** com execução via `lli` ou compilação com `clang`.

---

## ✅ Funcionalidades implementadas

- ✅ Analisador léxico com PLY
- ✅ Analisador sintático e geração de AST
- ✅ Interpretador opcional em Python
- ✅ Compilador LLVM usando `llvmlite`
- ✅ Execução via `lli` ou `clang`
- ✅ Suporte completo a:
  - Variáveis: `byte`, `bit`, `chip`
  - Expressões: `+`, `-`, `*`, `/`, comparações
  - `log` (imprime valores)
  - Condicional: `ping` / `pong`
  - Laços: `loop`, `stream`
  - Funções: `module`, `return`, chamadas

---

## 🚀 Como executar

### 1. Clone o projeto
git clone https://github.com/joaowcitino/aps_lingpar.git
cd aps_lingpar

### 2. Instale o ambiente

Requer Python 3.10+ e LLVM instalado (com `lli` e `clang` no PATH)

sudo apt install llvm clang
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt

### 3. Compile um programa `.tf`

python3 main.py tests/01_variavel_log.tf

### 4. Execute com LLVM

lli out.ll

Ou, se preferir:

clang out.ll -o out
./out

---

## 🧪 Exemplos

Código: `tests/01_variavel_log.tf`

boot main
  byte x = 42
  log x
shutdown main

Código com função + condição + loop

boot main
  module soma(a, b) process
    return a + b
  module_end

  byte resultado = soma(2, 3)
  log resultado

  ping resultado == 5 process
    log "Correto"
  ping_end

  pong process
    log "Errado"
  pong_end

  loop 3 as i process
    log i
  loop_end
shutdown main

---

## 🔧 Estrutura do projeto

techflow/
├── lexer.py              # Analisador léxico (PLY)
├── parser.py             # Analisador sintático + AST
├── interpreter.py        # Interpretador opcional da AST
├── llvm_compiler.py      # Compilador LLVM via llvmlite
├── main.py               # Entrada principal
├── tests/                # Arquivos de teste (.tf)
└── out.ll                # Código LLVM IR gerado

---

## 🧠 Motivação e design

A linguagem TechFlow foi projetada para:
- Tornar a programação acessível com sintaxe inclusiva
- Ser divertida para iniciantes (termos como `ping`, `boot`, `chip`)
- Ensinar os fundamentos com estrutura real de compilação

---

## 💬 Sintaxe da linguagem

Elemento       | Exemplo
---------------|-------------------------------
Início/fim     | boot main ... shutdown main
Variável       | byte x = 10 + 5
Log            | log x
Condicional    | ping x == 5 process ... pong process
Loop for       | loop 3 as i process ... loop_end
Loop while     | stream x < 5 process ... stream_end
Função         | module nome(a, b) process ... module_end
Return         | return a + b
Chamada função | soma(2, 3)

---

## 🛠️ Como o compilador funciona

[Arquivo .tf] 
   ↓
[PLY Lexer & Parser]
   ↓
[AST Python]
   ↓
[LLVM Compiler via llvmlite]
   ↓
[LLVM IR (.ll)] 
   ↓
[lli ou clang] → Execução ou binário

---

## 📦 Requisitos

- Python 3.10+
- LLVM 12+ (com `lli`, `clang`)
- `llvmlite` (via `pip install llvmlite`)
- Sistema: Linux, WSL, macOS ou compatível

---

## 👨‍🏫 Ideal para...

- Ensino de compiladores
- Educação em linguagens de programação
- Entregas acadêmicas com foco em VM real (nota A+)

---

## ✨ Autor

Feito por Joao Whitaker Citino – para a disciplina de Linguagens e Paradigmas 🧪💻
