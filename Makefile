CC = gcc
CFLAGS = -Wall -g
LIBS = `llvm-config --cflags --ldflags --libs core executionengine mcjit native bitwriter` -lfl

all: palco

palco: lex.yy.c palco.tab.c src/ast.c src/llvm_generator.c src/symbol_table.c src/main.c
	$(CC) $(CFLAGS) -o palco $^ $(LIBS)

lex.yy.c: src/palco.l palco.tab.h
	flex -o lex.yy.c src/palco.l

palco.tab.c palco.tab.h: src/palco.y
	bison -d src/palco.y

test: palco
	./test_suite.sh

clean:
	rm -f palco lex.yy.c palco.tab.c palco.tab.h output.bc *.o

.PHONY: all clean test