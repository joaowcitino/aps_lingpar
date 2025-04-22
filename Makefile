# Makefile para TechFlow

CC = gcc
CFLAGS = -Wall -Wextra -g
LLVM_CFLAGS = $(shell llvm-config --cflags)
LLVM_LDFLAGS = $(shell llvm-config --ldflags --libs)

# Diretórios
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
TEST_DIR = test

# Arquivos gerados pelo Flex e Bison
LEXER_SRC = $(SRC_DIR)/lexer/lexer.c
PARSER_SRC = $(SRC_DIR)/parser/parser.c
PARSER_HDR = $(SRC_DIR)/parser/parser.tab.h

# Fontes
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/semantic/semantic.c \
          $(SRC_DIR)/semantic/symbol_table.c \
          $(SRC_DIR)/codegen/codegen.c \
          $(SRC_DIR)/parser/ast.c \
          $(LEXER_SRC) \
          $(PARSER_SRC)

# Objetos
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Executável
EXECUTABLE = $(BUILD_DIR)/techflow

# Regra padrão
all: directories $(EXECUTABLE)

# Criar diretórios necessários
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/lexer
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/parser
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/semantic
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/codegen

# Compilar executável
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LLVM_LDFLAGS)

# Compilar arquivos objeto
$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(LLVM_CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Gerar lexer com Flex
$(LEXER_SRC): $(SRC_DIR)/lexer/lexer.l $(PARSER_HDR)
	flex -o $@ $(SRC_DIR)/lexer/lexer.l

# Gerar parser com Bison
$(PARSER_SRC) $(PARSER_HDR): $(SRC_DIR)/parser/parser.y
	bison -d -o $(PARSER_SRC) $(SRC_DIR)/parser/parser.y

# Limpar arquivos gerados
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(LEXER_SRC) $(PARSER_SRC) $(SRC_DIR)/parser/parser.h $(PARSER_HDR)

# Executar testes
test: all
	./scripts/test.sh

# Construir e testar
.PHONY: all directories clean test