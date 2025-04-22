CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lLLVM

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
TEST_DIR = test

LEXER_SRC = $(SRC_DIR)/lexer/lexer.c
PARSER_SRC = $(SRC_DIR)/parser/parser.c
PARSER_HDR = $(SRC_DIR)/parser/parser.tab.h

SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/semantic/semantic.c \
          $(SRC_DIR)/semantic/symbol_table.c \
          $(SRC_DIR)/codegen/codegen.c \
          $(LEXER_SRC) \
          $(PARSER_SRC)

OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

EXECUTABLE = $(BUILD_DIR)/techflow

all: directories $(EXECUTABLE)

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/lexer
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/parser
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/semantic
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/codegen

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(LEXER_SRC): $(SRC_DIR)/lexer/lexer.l $(PARSER_HDR)
	flex -o $@ $<

$(PARSER_SRC) $(PARSER_HDR): $(SRC_DIR)/parser/parser.y
	bison -d -o $(PARSER_SRC) $<

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(LEXER_SRC) $(PARSER_SRC) $(PARSER_HDR)

test: all
	./scripts/test.sh

.PHONY: all directories clean test