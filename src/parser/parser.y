%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int yylex();
extern void yyerror(const char *s);
extern int line_num;

ASTNode *ast_root = NULL;
%}

%union {
    int ival;
    char *sval;
    struct ASTNode *node;
}

%token BOOT SHUTDOWN
%token BYTE BIT CHIP
%token SCAN_BEGIN SCAN_END
%token PING PROCESS PING_END
%token PONG PONG_END
%token LOOP AS LOOP_END
%token STREAM STREAM_END
%token MODULE MODULE_END RETURN
%token LOG

%token PLUS MINUS MULTIPLY DIVIDE
%token EQUAL NOT_EQUAL GREATER LESS GREATER_EQUAL LESS_EQUAL
%token AND OR ASSIGN
%token LPAREN RPAREN LBRACKET RBRACKET COMMA

%token <ival> NUMBER
%token <sval> IDENTIFIER STRING

%type <node> programa declaracoes declaracao
%type <node> declaracao_variavel tipo
%type <node> estrutura_controle condicional bloco_condicional bloco_if bloco_else
%type <node> loop_for loop_while
%type <node> definicao_funcao parametros retorno chamada_funcao argumentos
%type <node> expressao termo operador saida
%type <node> literal array

%left OR
%left AND
%left EQUAL NOT_EQUAL
%left GREATER LESS GREATER_EQUAL LESS_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE

%start programa

%%

programa
    : BOOT IDENTIFIER declaracoes SHUTDOWN IDENTIFIER
    {
        $$ = create_program_node($2, $3, $5);
        ast_root = $$;
    }
    ;

declaracoes
    : 
    {
        $$ = create_declarations_node(NULL, NULL);
    }
    | declaracoes declaracao
    {
        $$ = add_declaration($1, $2);
    }
    ;

declaracao
    : declaracao_variavel { $$ = $1; }
    | estrutura_controle { $$ = $1; }
    | chamada_funcao { $$ = $1; }
    | definicao_funcao { $$ = $1; }
    | saida { $$ = $1; }
    ;

declaracao_variavel
    : tipo IDENTIFIER ASSIGN expressao
    {
        $$ = create_var_declaration_node($1, $2, $4);
    }
    ;

tipo
    : BYTE { $$ = create_type_node("byte"); }
    | BIT { $$ = create_type_node("bit"); }
    | CHIP { $$ = create_type_node("chip"); }
    ;

estrutura_controle
    : condicional { $$ = $1; }
    | loop_for { $$ = $1; }
    | loop_while { $$ = $1; }
    ;

condicional
    : SCAN_BEGIN bloco_condicional SCAN_END
    {
        $$ = create_conditional_node($2);
    }
    ;

bloco_condicional
    : bloco_if { $$ = create_if_blocks_node($1, NULL); }
    | bloco_condicional bloco_if { $$ = add_if_block($1, $2); }
    | bloco_condicional bloco_else { $$ = add_else_block($1, $2); }
    ;

bloco_if
    : PING expressao PROCESS declaracoes PING_END
    {
        $$ = create_if_block_node($2, $4);
    }
    ;

bloco_else
    : PONG PROCESS declaracoes PONG_END
    {
        $$ = create_else_block_node($3);
    }
    ;

loop_for
    : LOOP expressao AS IDENTIFIER PROCESS declaracoes LOOP_END
    {
        $$ = create_for_loop_node($2, $4, $6);
    }
    ;

loop_while
    : STREAM expressao PROCESS declaracoes STREAM_END
    {
        $$ = create_while_loop_node($2, $4);
    }
    ;

definicao_funcao
    : MODULE IDENTIFIER LPAREN parametros RPAREN PROCESS declaracoes retorno MODULE_END
    {
        $$ = create_function_def_node($2, $4, $7, $8);
    }
    | MODULE IDENTIFIER LPAREN parametros RPAREN PROCESS declaracoes MODULE_END
    {
        $$ = create_function_def_node($2, $4, $7, NULL);
    }
    ;

parametros
    : 
    {
        $$ = create_params_node(NULL);
    }
    | IDENTIFIER
    {
        $$ = create_params_node($1);
    }
    | parametros COMMA IDENTIFIER
    {
        $$ = add_param($1, $3);
    }
    ;

retorno
    : RETURN expressao
    {
        $$ = create_return_node($2);
    }
    ;

chamada_funcao
    : IDENTIFIER LPAREN argumentos RPAREN
    {
        $$ = create_function_call_node($1, $3);
    }
    | IDENTIFIER LPAREN RPAREN
    {
        $$ = create_function_call_node($1, NULL);
    }
    ;

argumentos
    : expressao
    {
        $$ = create_args_node($1);
    }
    | argumentos COMMA expressao
    {
        $$ = add_arg($1, $3);
    }
    ;

expressao
    : termo { $$ = $1; }
    | expressao operador termo
    {
        $$ = create_binary_expr_node($2, $1, $3);
    }
    ;

termo
    : IDENTIFIER { $$ = create_identifier_node($1); }
    | literal { $$ = $1; }
    | chamada_funcao { $$ = $1; }
    | LPAREN expressao RPAREN { $$ = $2; }
    ;

operador
    : PLUS { $$ = create_operator_node(OP_PLUS); }
    | MINUS { $$ = create_operator_node(OP_MINUS); }
    | MULTIPLY { $$ = create_operator_node(OP_MULTIPLY); }
    | DIVIDE { $$ = create_operator_node(OP_DIVIDE); }
    | EQUAL { $$ = create_operator_node(OP_EQUAL); }
    | NOT_EQUAL { $$ = create_operator_node(OP_NOT_EQUAL); }
    | GREATER { $$ = create_operator_node(OP_GREATER); }
    | LESS { $$ = create_operator_node(OP_LESS); }
    | GREATER_EQUAL { $$ = create_operator_node(OP_GREATER_EQUAL); }
    | LESS_EQUAL { $$ = create_operator_node(OP_LESS_EQUAL); }
    | AND { $$ = create_operator_node(OP_AND); }
    | OR { $$ = create_operator_node(OP_OR); }
    ;

saida
    : LOG expressao
    {
        $$ = create_log_node($2);
    }
    ;

literal
    : NUMBER { $$ = create_number_node($1); }
    | STRING { $$ = create_string_node($1); }
    | array { $$ = $1; }
    ;

array
    : LBRACKET RBRACKET
    {
        $$ = create_array_node(NULL);
    }
    | LBRACKET expressao RBRACKET
    {
        $$ = create_array_node($2);
    }
    | LBRACKET expressao argumentos RBRACKET
    {
        ASTNode *firstElem = create_args_node($2);
        $$ = create_array_node(add_args(firstElem, $3));
    }
    ;

%%