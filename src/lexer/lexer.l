%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/ast.h"
#include "../parser/parser.tab.h"

int line_num = 1;
%}

%option noyywrap
%option yylineno

DIGITO      [0-9]
LETRA       [a-zA-Z_]
ID          {LETRA}({LETRA}|{DIGITO})*
NUMERO      {DIGITO}+
WHITESPACE  [ \t\r]+
STRING      \"[^\"]*\"

%%

"boot"       { return BOOT; }
"shutdown"   { return SHUTDOWN; }
"byte"       { return BYTE; }
"bit"        { return BIT; }
"chip"       { return CHIP; }
"scan_begin" { return SCAN_BEGIN; }
"scan_end"   { return SCAN_END; }
"ping"       { return PING; }
"process"    { return PROCESS; }
"ping_end"   { return PING_END; }
"pong"       { return PONG; }
"pong_end"   { return PONG_END; }
"loop"       { return LOOP; }
"as"         { return AS; }
"loop_end"   { return LOOP_END; }
"stream"     { return STREAM; }
"stream_end" { return STREAM_END; }
"module"     { return MODULE; }
"module_end" { return MODULE_END; }
"return"     { return RETURN; }
"log"        { return LOG; }

"+"          { return PLUS; }
"-"          { return MINUS; }
"*"          { return MULTIPLY; }
"/"          { return DIVIDE; }
"=="         { return EQUAL; }
"!="         { return NOT_EQUAL; }
">"          { return GREATER; }
"<"          { return LESS; }
">="         { return GREATER_EQUAL; }
"<="         { return LESS_EQUAL; }
"&&"         { return AND; }
"||"         { return OR; }
"="          { return ASSIGN; }
"("          { return LPAREN; }
")"          { return RPAREN; }
"["          { return LBRACKET; }
"]"          { return RBRACKET; }
","          { return COMMA; }

{ID}         { 
                yylval.sval = strdup(yytext);
                return IDENTIFIER; 
            }

{NUMERO}     { 
                yylval.ival = atoi(yytext);
                return NUMBER; 
            }

{STRING}     { 
                yylval.sval = strdup(yytext);
                return STRING; 
            }

\n           { line_num++; }
{WHITESPACE} { }

.            { printf("Erro léxico: caractere desconhecido '%s' na linha %d\n", yytext, line_num); }

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro na linha %d: %s\n", line_num, s);
}