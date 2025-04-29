CC = gcc
CFLAGS = -Wall -g
LLVM_CFLAGS = $(shell llvm-config --cflags)
LLVM_LDFLAGS = $(shell llvm-config --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter)

SRC_DIR = src
BIN_DIR = bin
EXAMPLES_DIR = examples

all: check_dirs $(BIN_DIR)/techflow_compiler $(SRC_DIR)/runtime_support.o

check_dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(EXAMPLES_DIR)

$(BIN_DIR)/techflow_compiler: $(SRC_DIR)/main.o $(SRC_DIR)/parser.tab.o $(SRC_DIR)/lex.yy.o $(SRC_DIR)/interpreter.o $(SRC_DIR)/llvm_generator.o
	$(CC) $(CFLAGS) -o $@ $^ $(LLVM_LDFLAGS)

$(SRC_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/llvm_generator.h
	$(CC) $(CFLAGS) $(LLVM_CFLAGS) -c $< -o $@

$(SRC_DIR)/interpreter.o: $(SRC_DIR)/interpreter.c $(SRC_DIR)/llvm_generator.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/llvm_generator.o: $(SRC_DIR)/llvm_generator.c $(SRC_DIR)/llvm_generator.h
	$(CC) $(CFLAGS) $(LLVM_CFLAGS) -c $< -o $@

$(SRC_DIR)/runtime_support.o: $(SRC_DIR)/runtime_support.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(SRC_DIR)/parser.tab.c $(SRC_DIR)/parser.tab.h: $(SRC_DIR)/parser.y
	cd $(SRC_DIR) && bison -d parser.y

$(SRC_DIR)/lex.yy.c: $(SRC_DIR)/lexer.l $(SRC_DIR)/parser.tab.h
	cd $(SRC_DIR) && flex lexer.l

$(SRC_DIR)/parser.tab.o: $(SRC_DIR)/parser.tab.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/lex.yy.o: $(SRC_DIR)/lex.yy.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BIN_DIR)/techflow_compiler $(SRC_DIR)/*.o $(SRC_DIR)/lex.yy.c $(SRC_DIR)/parser.tab.c $(SRC_DIR)/parser.tab.h *.bc programa

.PHONY: all clean check_dirs

test-interpret: $(BIN_DIR)/techflow_compiler
	$(BIN_DIR)/techflow_compiler $(EXAMPLES_DIR)/teste.tf --interpret

test-compile: $(BIN_DIR)/techflow_compiler
	$(BIN_DIR)/techflow_compiler $(EXAMPLES_DIR)/teste.tf --compile

test-run: test-compile
	@echo "Compilando bitcode para executável..."
	llc -relocation-model=pic -filetype=obj output.bc -o output.o
	$(CC) output.o $(SRC_DIR)/runtime_support.o -o programa
	@echo "Executando programa compilado:"
	./programa

python-run:
	python python/main.py $(EXAMPLES_DIR)/teste.tf