%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

void yyerror(const char *s);
%}

%option noyywrap
%option yylineno

%%

[ \t\n\r]+                  { }
"//".*                      { }
"/*"([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+"/"  { }

"boot"                      { return BOOT; }
"shutdown"                  { return SHUTDOWN; }
"byte"                      { return BYTE; }
"stream"                    { return STREAM; }
"ping"                      { return PING; }
"pong"                      { return PONG; }
"log"                       { return LOG; }
"repeat"                    { return REPEAT; }
"until"                     { return UNTIL; }
"select"                    { return SELECT; }
"when"                      { return WHEN; }
"otherwise"                 { return OTHERWISE; }
"then"                      { return THEN; }
"end"                       { return END; }

"i32"                       { yylval.strval = strdup(yytext); return TYPE; }
"bool"                      { yylval.strval = strdup(yytext); return TYPE; }
"str"                       { yylval.strval = strdup(yytext); return TYPE; }

"true"                      { yylval.boolval = 1; return BOOLEAN; }
"false"                     { yylval.boolval = 0; return BOOLEAN; }

[a-zA-Z][a-zA-Z0-9_]*       { yylval.strval = strdup(yytext); return IDENTIFIER; }
[0-9]+                      { yylval.intval = atoi(yytext); return NUMBER; }
\"([^\"\n]|\\\")*\"         { 
                              yytext[strlen(yytext)-1] = '\0';
                              yylval.strval = strdup(yytext+1);
                              return STRING; 
                            }

"+"                         { return PLUS; }
"-"                         { return MINUS; }
"*"                         { return MULTIPLY; }
"/"                         { return DIVIDE; }
"%"                         { return MODULO; }
"++"                        { return CONCAT; }

"=="                        { return EQ; }
"!="                        { return NEQ; }
"<"                         { return LT; }
">"                         { return GT; }
"<="                        { return LE; }
">="                        { return GE; }
"&&"                        { return AND; }
"||"                        { return OR; }
"!"                         { return NOT; }

"="                         { return ASSIGN; }
";"                         { return SEMICOLON; }
":"                         { return COLON; }
","                         { return COMMA; }
"("                         { return LPAREN; }
")"                         { return RPAREN; }

.                           { yyerror("Caractere inválido"); }

%%