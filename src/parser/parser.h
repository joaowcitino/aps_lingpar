/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SRC_PARSER_PARSER_H_INCLUDED
# define YY_YY_SRC_PARSER_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    BOOT = 258,                    /* BOOT  */
    SHUTDOWN = 259,                /* SHUTDOWN  */
    BYTE = 260,                    /* BYTE  */
    BIT = 261,                     /* BIT  */
    CHIP = 262,                    /* CHIP  */
    SCAN_BEGIN = 263,              /* SCAN_BEGIN  */
    SCAN_END = 264,                /* SCAN_END  */
    PING = 265,                    /* PING  */
    PROCESS = 266,                 /* PROCESS  */
    PING_END = 267,                /* PING_END  */
    PONG = 268,                    /* PONG  */
    PONG_END = 269,                /* PONG_END  */
    LOOP = 270,                    /* LOOP  */
    AS = 271,                      /* AS  */
    LOOP_END = 272,                /* LOOP_END  */
    STREAM = 273,                  /* STREAM  */
    STREAM_END = 274,              /* STREAM_END  */
    MODULE = 275,                  /* MODULE  */
    MODULE_END = 276,              /* MODULE_END  */
    RETURN = 277,                  /* RETURN  */
    LOG = 278,                     /* LOG  */
    PLUS = 279,                    /* PLUS  */
    MINUS = 280,                   /* MINUS  */
    MULTIPLY = 281,                /* MULTIPLY  */
    DIVIDE = 282,                  /* DIVIDE  */
    EQUAL = 283,                   /* EQUAL  */
    NOT_EQUAL = 284,               /* NOT_EQUAL  */
    GREATER = 285,                 /* GREATER  */
    LESS = 286,                    /* LESS  */
    GREATER_EQUAL = 287,           /* GREATER_EQUAL  */
    LESS_EQUAL = 288,              /* LESS_EQUAL  */
    AND = 289,                     /* AND  */
    OR = 290,                      /* OR  */
    ASSIGN = 291,                  /* ASSIGN  */
    LPAREN = 292,                  /* LPAREN  */
    RPAREN = 293,                  /* RPAREN  */
    LBRACKET = 294,                /* LBRACKET  */
    RBRACKET = 295,                /* RBRACKET  */
    COMMA = 296,                   /* COMMA  */
    NUMBER = 297,                  /* NUMBER  */
    IDENTIFIER = 298,              /* IDENTIFIER  */
    STRING = 299                   /* STRING  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 14 "src/parser/parser.y"

    int ival;
    char *sval;
    struct ASTNode *node;

#line 114 "src/parser/parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_SRC_PARSER_PARSER_H_INCLUDED  */
