# Linguagem PALCO 🎭

**PALCO** é uma linguagem de programação inclusiva, lúdica e criativa, baseada no teatro. Seu objetivo é tornar o código uma "apresentação teatral", onde variáveis são personagens com papéis específicos (protagonistas, coadjuvantes) e comandos representam ações no palco.

## ✨ Objetivo
- Criar uma linguagem de programação acessível e intuitiva.
- Utilizar metáforas teatrais completas para representar conceitos de programação.
- Implementar as estruturas básicas: variáveis, condicionais e loops.
- Gerar código LLVM-IR para execução eficiente.

## 🔤 Conceitos Teatrais na Linguagem

| Conceito Teatral | Conceito de Programação      |
|------------------|------------------------------|
| Apresentação     | Programa                     |
| Personagem       | Variável Genérica            |
| Protagonista     | Variável Inteira             |
| Coadjuvante      | Variável Float               |
| Cenário          | Variável String              |
| Ato              | Bloco de Código              |
| Cena             | Função Reutilizável          |
| Fala             | Print/Output                 |
| Narrar           | Logging/Debug Output         |
| Entra/Sai        | Inicialização/Finalização    |
| Improviso        | Geração de Número Aleatório  |

## 📋 Sintaxe Especial

| Elemento Sintático | Em PALCO | Em Linguagens Comuns |
|--------------------|----------|----------------------|
| Início do programa | ESPETACULO_COMECA | (varia) |
| Fim do programa | ESPETACULO_TERMINA | (varia) |
| Abertura de bloco | ABRE_CORTINA | { |
| Fechamento de bloco | FECHA_CORTINA | } |
| Término de comando | !! | ; |
| Abertura de expressão | << | ( |
| Fechamento de expressão | >> | ) |
| Igual a | IGUAL_A | == |
| Diferente de | DIFERENTE_DE | != |
| Menor que | MENOR_QUE | < |
| Maior que | MAIOR_QUE | > |
| E lógico | E_TAMBEM | && |
| OU lógico | OU_ENTAO | \|\| |

## 🚀 Instalação e Requisitos

### Requisitos
- Flex (analisador léxico)
- Bison (analisador sintático)
- LLVM (compilação e interpretação)
- GCC (compilador C)

### Instalação em sistemas baseados em Debian/Ubuntu

```bash
sudo apt-get update
sudo apt-get install flex bison llvm llvm-dev gcc make