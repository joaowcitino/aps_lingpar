%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "palco.tab.h"

void yyerror(char *s);
%}

%%

"ESPETACULO_COMECA"    { return APRESENTAR; }
"ESPETACULO_TERMINA"   { return ENCERRAR; }
"personagem"        { return PERSONAGEM; }
"protagonista"      { return PROTAGONISTA; }
"coadjuvante"       { return COADJUVANTE; }
"cenario"           { return CENARIO; }
"ato"               { return ATO; }
"cena"              { return CENA; }
"se"                { return SE; }
"senao"             { return SENAO; }
"repetir"           { return REPETIR; }
"caminhar"          { return CAMINHAR; }
"fala"              { return FALA; }
"narrar"            { return NARRAR; }
"entra"             { return ENTRA; }
"sai"               { return SAI; }
"improviso"         { return IMPROVISO; }
"ABRE_CORTINA"      { return ABRE_CHAVE; }
"FECHA_CORTINA"     { return FECHA_CHAVE; }
"IGUAL_A"           { return IGUAL; }
"DIFERENTE_DE"      { return DIFERENTE; }
"MENOR_QUE"         { return MENOR; }
"MAIOR_QUE"         { return MAIOR; }
"MENOR_IGUAL_A"     { return MENOR_IGUAL; }
"MAIOR_IGUAL_A"     { return MAIOR_IGUAL; }
"E_TAMBEM"          { return E_LOGICO; }
"OU_ENTAO"          { return OU_LOGICO; }

[0-9]+              { yylval.num = atoi(yytext); return NUMERO; }
[0-9]+\.[0-9]+      { yylval.fnum = atof(yytext); return DECIMAL; }

\"[^\"]*\"          { 
                      yylval.str = strdup(yytext+1);
                      yylval.str[strlen(yylval.str)-1] = '\0';
                      return STRING; 
                    }

[a-zA-Z][a-zA-Z0-9_]* { yylval.str = strdup(yytext); return IDENTIFICADOR; }

"+"                 { return MAIS; }
"-"                 { return MENOS; }
"*"                 { return VEZES; }
"/"                 { return DIVIDE; }
"="                 { return ATRIB; }
"<<"                { return ABRE_PAREN; }
">>"                { return FECHA_PAREN; }
"!!"                { return PONTO_VIRGULA; }
","                 { return VIRGULA; }

[ \t\n]             ;
"//".*              ;
"/*"([^*]|"*"[^/])*"*/"  ;

.                   { printf("Caractere inválido: %s\n", yytext); }

%%

int yywrap(void) {
    return 1;
}