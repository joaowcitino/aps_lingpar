%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
extern int yylineno;
extern char* yytext;
void yyerror(const char* s);

typedef enum {
    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_REPEAT,
    NODE_SWITCH,
    NODE_CASE,
    NODE_PRINT,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_INT_VAL,
    NODE_STRING_VAL,
    NODE_BOOL_VAL,
    NODE_IDENTIFIER
} NodeType;

typedef struct Node {
    NodeType type;
    union {
        int int_value;
        char* str_value;
        int bool_value;
        struct {
            char* operator;
            struct Node* left;
            struct Node* right;
        } binary_op;
        struct {
            char* operator;
            struct Node* operand;
        } unary_op;
        struct {
            char* name;
            char* data_type;
            struct Node* init_expr;
        } var_decl;
        struct {
            char* name;
            struct Node* value;
        } assign;
        struct {
            struct Node* condition;
            struct Node* then_branch;
            struct Node* else_branch;
        } if_stmt;
        struct {
            struct Node* condition;
            struct Node* body;
        } while_stmt;
        struct {
            struct Node* body;
            struct Node* condition;
        } repeat_stmt;
        struct {
            struct Node* expr;
        } print_stmt;
        struct {
            struct Node* condition;
            struct Node** cases;
            int case_count;
            struct Node* default_case;
        } switch_stmt;
        struct {
            struct Node* value;
            struct Node* body;
        } case_stmt;
        struct {
            struct Node** statements;
            int stmt_count;
        } block;
        struct {
            struct Node* body;
        } program;
    } data;
    struct Node* next;
} Node;

Node* create_program_node(Node* body);
Node* create_block_node();
void add_statement_to_block(Node* block, Node* statement);
Node* create_var_decl_node(char* name, char* type, Node* init_expr);
Node* create_assign_node(char* name, Node* value);
Node* create_if_node(Node* condition, Node* then_branch, Node* else_branch);
Node* create_while_node(Node* condition, Node* body);
Node* create_repeat_node(Node* body, Node* condition);
Node* create_switch_node(Node* condition);
Node* create_case_node(Node* value, Node* body);
void add_case_to_switch(Node* switch_node, Node* case_node);
void set_default_case(Node* switch_node, Node* default_case);
Node* create_print_node(Node* expr);
Node* create_binary_op_node(char* op, Node* left, Node* right);
Node* create_unary_op_node(char* op, Node* operand);
Node* create_int_val_node(int value);
Node* create_string_val_node(char* value);
Node* create_bool_val_node(int value);
Node* create_identifier_node(char* name);

Node* ast_root = NULL;
%}

%union {
    int intval;
    char* strval;
    int boolval;
    struct Node* node;
}

%token BOOT SHUTDOWN
%token BYTE STREAM PING PONG LOG REPEAT UNTIL SELECT WHEN OTHERWISE THEN END
%token <strval> TYPE
%token <strval> IDENTIFIER
%token <intval> NUMBER
%token <strval> STRING
%token <boolval> BOOLEAN
%token PLUS MINUS MULTIPLY DIVIDE MODULO
%token EQ NEQ LT GT LE GE
%token AND OR NOT
%token CONCAT
%token ASSIGN SEMICOLON COLON COMMA LPAREN RPAREN

%type <node> program statements statement var_decl if_stmt while_stmt repeat_stmt
%type <node> select_stmt case_stmt default_stmt log_stmt expr_stmt case_list
%type <node> expression concat_expr logical_or logical_and equality relational
%type <node> additive term factor primary

%start program

%%

program
    : BOOT statements SHUTDOWN
        { ast_root = create_program_node($2); }
    ;

statements
    : statement
        { 
            $$ = create_block_node();
            add_statement_to_block($$, $1);
        }
    | statements statement
        {
            $$ = $1;
            add_statement_to_block($$, $2);
        }
    |
        { $$ = create_block_node(); }
    ;

statement
    : var_decl
        { $$ = $1; }
    | if_stmt
        { $$ = $1; }
    | while_stmt
        { $$ = $1; }
    | repeat_stmt
        { $$ = $1; }
    | select_stmt
        { $$ = $1; }
    | log_stmt
        { $$ = $1; }
    | IDENTIFIER ASSIGN expression SEMICOLON
        { $$ = create_assign_node($1, $3); }
    | expr_stmt
        { $$ = $1; }
    | SEMICOLON
        { $$ = NULL; }
    ;

var_decl
    : BYTE IDENTIFIER COLON TYPE SEMICOLON
        { $$ = create_var_decl_node($2, $4, NULL); }
    | BYTE IDENTIFIER COLON TYPE ASSIGN expression SEMICOLON
        { $$ = create_var_decl_node($2, $4, $6); }
    ;

if_stmt
    : PING LPAREN expression RPAREN THEN statements END
        { $$ = create_if_node($3, $6, NULL); }
    | PING LPAREN expression RPAREN THEN statements PONG THEN statements END
        { $$ = create_if_node($3, $6, $9); }
    ;

while_stmt
    : STREAM LPAREN expression RPAREN THEN statements END
        { $$ = create_while_node($3, $6); }
    ;

repeat_stmt
    : REPEAT THEN statements UNTIL expression SEMICOLON
        { $$ = create_repeat_node($3, $5); }
    ;

select_stmt
    : SELECT LPAREN expression RPAREN THEN case_list END
        { $$ = $6; ((Node*)$$)->data.switch_stmt.condition = $3; }
    ;

case_list
    : case_stmt
        {
            $$ = create_switch_node(NULL);
            add_case_to_switch($$, $1);
        }
    | case_list case_stmt
        {
            $$ = $1;
            add_case_to_switch($$, $2);
        }
    | case_list default_stmt
        {
            $$ = $1;
            set_default_case($$, $2);
        }
    ;

case_stmt
    : WHEN expression THEN statements END
        { $$ = create_case_node($2, $4); }
    ;

default_stmt
    : OTHERWISE THEN statements END
        { $$ = $3; }
    ;

log_stmt
    : LOG LPAREN expression RPAREN SEMICOLON
        { $$ = create_print_node($3); }
    | LOG LPAREN expression RPAREN
        { $$ = create_print_node($3); }
    ;

expr_stmt
    : expression SEMICOLON
        { $$ = $1; }
    ;

expression
    : concat_expr
        { $$ = $1; }
    ;

concat_expr
    : logical_or
        { $$ = $1; }
    | concat_expr CONCAT logical_or
        { $$ = create_binary_op_node("CONCAT", $1, $3); }
    ;

logical_or
    : logical_and
        { $$ = $1; }
    | logical_or OR logical_and
        { $$ = create_binary_op_node("OR", $1, $3); }
    ;

logical_and
    : equality
        { $$ = $1; }
    | logical_and AND equality
        { $$ = create_binary_op_node("AND", $1, $3); }
    ;

equality
    : relational
        { $$ = $1; }
    | equality EQ relational
        { $$ = create_binary_op_node("EQ", $1, $3); }
    | equality NEQ relational
        { $$ = create_binary_op_node("NEQ", $1, $3); }
    ;

relational
    : additive
        { $$ = $1; }
    | relational LT additive
        { $$ = create_binary_op_node("<", $1, $3); }
    | relational GT additive
        { $$ = create_binary_op_node(">", $1, $3); }
    | relational LE additive
        { $$ = create_binary_op_node("LE", $1, $3); }
    | relational GE additive
        { $$ = create_binary_op_node("GE", $1, $3); }
    ;

additive
    : term
        { $$ = $1; }
    | additive PLUS term
        { $$ = create_binary_op_node("+", $1, $3); }
    | additive MINUS term
        { $$ = create_binary_op_node("-", $1, $3); }
    ;

term
    : factor
        { $$ = $1; }
    | term MULTIPLY factor
        { $$ = create_binary_op_node("*", $1, $3); }
    | term DIVIDE factor
        { $$ = create_binary_op_node("/", $1, $3); }
    | term MODULO factor
        { $$ = create_binary_op_node("%", $1, $3); }
    ;

factor
    : primary
        { $$ = $1; }
    | PLUS factor
        { $$ = create_unary_op_node("+", $2); }
    | MINUS factor
        { $$ = create_unary_op_node("-", $2); }
    | NOT factor
        { $$ = create_unary_op_node("not", $2); }
    ;

primary
    : NUMBER
        { $$ = create_int_val_node($1); }
    | STRING
        { $$ = create_string_val_node($1); }
    | BOOLEAN
        { $$ = create_bool_val_node($1); }
    | IDENTIFIER
        { $$ = create_identifier_node($1); }
    | LPAREN expression RPAREN
        { $$ = $2; }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Erro (linha %d): %s próximo a '%s'\n", yylineno, s, yytext);
    exit(1);
}

Node* create_program_node(Node* body) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_PROGRAM;
    node->data.program.body = body;
    node->next = NULL;
    return node;
}

Node* create_block_node() {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_BLOCK;
    node->data.block.statements = NULL;
    node->data.block.stmt_count = 0;
    node->next = NULL;
    return node;
}

void add_statement_to_block(Node* block, Node* statement) {
    if (statement == NULL) return;
    
    block->data.block.stmt_count++;
    Node** new_statements = (Node**)realloc(
        block->data.block.statements, 
        block->data.block.stmt_count * sizeof(Node*)
    );
    
    if (new_statements == NULL) {
        fprintf(stderr, "Erro de alocação de memória\n");
        exit(1);
    }
    
    block->data.block.statements = new_statements;
    block->data.block.statements[block->data.block.stmt_count - 1] = statement;
}

Node* create_var_decl_node(char* name, char* type, Node* init_expr) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_VAR_DECL;
    node->data.var_decl.name = name;
    node->data.var_decl.data_type = type;
    node->data.var_decl.init_expr = init_expr;
    node->next = NULL;
    return node;
}

Node* create_assign_node(char* name, Node* value) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_ASSIGN;
    node->data.assign.name = name;
    node->data.assign.value = value;
    node->next = NULL;
    return node;
}

Node* create_if_node(Node* condition, Node* then_branch, Node* else_branch) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_IF;
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    node->next = NULL;
    return node;
}

Node* create_while_node(Node* condition, Node* body) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_WHILE;
    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;
    node->next = NULL;
    return node;
}

Node* create_repeat_node(Node* body, Node* condition) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_REPEAT;
    node->data.repeat_stmt.body = body;
    node->data.repeat_stmt.condition = condition;
    node->next = NULL;
    return node;
}

Node* create_switch_node(Node* condition) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_SWITCH;
    node->data.switch_stmt.condition = condition;
    node->data.switch_stmt.cases = NULL;
    node->data.switch_stmt.case_count = 0;
    node->data.switch_stmt.default_case = NULL;
    node->next = NULL;
    return node;
}

Node* create_case_node(Node* value, Node* body) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_CASE;
    node->data.case_stmt.value = value;
    node->data.case_stmt.body = body;
    node->next = NULL;
    return node;
}

void add_case_to_switch(Node* switch_node, Node* case_node) {
    switch_node->data.switch_stmt.case_count++;
    Node** new_cases = (Node**)realloc(
        switch_node->data.switch_stmt.cases, 
        switch_node->data.switch_stmt.case_count * sizeof(Node*)
    );
    
    if (new_cases == NULL) {
        fprintf(stderr, "Erro de alocação de memória\n");
        exit(1);
    }
    
    switch_node->data.switch_stmt.cases = new_cases;
    switch_node->data.switch_stmt.cases[switch_node->data.switch_stmt.case_count - 1] = case_node;
}

void set_default_case(Node* switch_node, Node* default_case) {
    switch_node->data.switch_stmt.default_case = default_case;
}

Node* create_print_node(Node* expr) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_PRINT;
    node->data.print_stmt.expr = expr;
    node->next = NULL;
    return node;
}

Node* create_binary_op_node(char* op, Node* left, Node* right) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_BINARY_OP;
    node->data.binary_op.operator = strdup(op);
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    node->next = NULL;
    return node;
}

Node* create_unary_op_node(char* op, Node* operand) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_UNARY_OP;
    node->data.unary_op.operator = strdup(op);
    node->data.unary_op.operand = operand;
    node->next = NULL;
    return node;
}

Node* create_int_val_node(int value) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_INT_VAL;
    node->data.int_value = value;
    node->next = NULL;
    return node;
}

Node* create_string_val_node(char* value) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_STRING_VAL;
    node->data.str_value = value;
    node->next = NULL;
    return node;
}

Node* create_bool_val_node(int value) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_BOOL_VAL;
    node->data.bool_value = value;
    node->next = NULL;
    return node;
}

Node* create_identifier_node(char* name) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_IDENTIFIER;
    node->data.str_value = name;
    node->next = NULL;
    return node;
}