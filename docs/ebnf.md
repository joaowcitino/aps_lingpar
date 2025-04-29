# Gramática EBNF da Linguagem TechFlow

A linguagem TechFlow segue a especificação EBNF (Extended Backus-Naur Form) detalhada abaixo.

## 1. Lexemas básicos

```ebnf
letter        = %x41-5A / %x61-7A          ; A–Z / a–z
digit         = %x30-39                    ; 0–9
alphanum      = letter / digit
underscore    = "_"
DQUOTE        = %x22                       ; "
SP            = %x20
HTAB          = %x09
LF            = %x0A
CR            = %x0D
WS            = SP / HTAB / LF / CR
S             = 1*WS
IDENTIFIER    = letter *( alphanum / underscore )
NUMBER        = 1*digit
STRING        = DQUOTE *( %x20-21 / %x23-7E ) DQUOTE
BOOLEAN       = "true" / "false"
```

## 2. Keywords

```ebnf
BOOT          = "boot"
SHUTDOWN      = "shutdown"
BYTE          = "byte"      ; substitui "var"
STREAM        = "stream"    ; substitui "while"
PING          = "ping"      ; substitui "if"
PONG          = "pong"      ; substitui "else"
LOG           = "log"       ; substitui "print"/"printf"
REPEAT        = "repeat"    ; para repeat-until
UNTIL         = "until"     ; para repeat-until
SELECT        = "select"    ; para switch case
WHEN          = "when"      ; para case
OTHERWISE     = "otherwise" ; para default case
THEN          = "then"
END           = "end"
TYPE          = "i32" / "bool" / "str"
ASSIGN        = "="
```

## 3. Operadores

```ebnf
PLUS          = "+"
MINUS         = "-"
ASTERISK      = "*"
SLASH         = "/"
PERCENT       = "%"
EQ            = "=="
NEQ           = "!="
LT            = "<"
GT            = ">"
LE            = "<="
GE            = ">="
AND           = "&&"
OR            = "||"
NOT           = "!"
CONCAT        = "++"
```

## 4. Delimitadores

```ebnf
SEMICOLON     = ";"
COMMA         = ","
LPAREN        = "("
RPAREN        = ")"
COLON         = ":"
```

## 5. Estrutura do programa

```ebnf
program       = BOOT S? statements S? SHUTDOWN
statements    = *( statement )
```

## 6. Tipos de declaração

```ebnf
statement     = var_decl
              / if_stmt
              / while_stmt
              / repeat_stmt
              / select_stmt
              / log_stmt
              / expr_stmt
var_decl      = BYTE S IDENTIFIER S? COLON S? TYPE [ S? ASSIGN S? expression ] SEMICOLON
```

## 7. Fluxo de controle

```ebnf
if_stmt       = PING S LPAREN expression RPAREN S? THEN S? statements [ S? PONG S? THEN S? statements ] S? END
while_stmt    = STREAM S LPAREN expression RPAREN S? THEN S? statements S? END
repeat_stmt   = REPEAT S? THEN S? statements S? UNTIL S expression SEMICOLON
select_stmt   = SELECT S LPAREN expression RPAREN S? THEN S? *( case_stmt ) [ S? default_stmt ] S? END
case_stmt     = WHEN S expression S? THEN S? statements S? END
default_stmt  = OTHERWISE S? THEN S? statements S? END
```

## 8. I/O

```ebnf
log_stmt      = LOG S LPAREN expression RPAREN [ SEMICOLON ]
```

## 9. Expressões

```ebnf
expr_stmt     = expression SEMICOLON
expression      = concat_expr
concat_expr     = logical_or *( S CONCAT S logical_or )
logical_or      = logical_and *( S OR S logical_and )
logical_and     = equality *( S AND S equality )
equality        = relational *( S (EQ / NEQ) S relational )
relational      = additive *( S (LT / GT / LE / GE) S additive )
additive        = term *( S (PLUS / MINUS) S term )
term            = factor *( S (ASTERISK / SLASH / PERCENT) S factor )
factor          = ( PLUS / MINUS / NOT ) S factor
                / primary
primary         = NUMBER
                / STRING
                / BOOLEAN
                / IDENTIFIER
                / LPAREN S expression S RPAREN
```

## 10. Fim de arquivo

```ebnf
EOF            =
```

## Exemplo

```
boot
    byte x: i32 = 10;
    byte y: i32 = 5;
    byte result: i32 = x + y * 2;
    log(result);
    
    ping(result > 15) then
        log("Resultado maior que 15");
    pong then
        log("Resultado menor ou igual a 15");
    end
shutdown
```

Este exemplo ilustra a sintaxe básica da linguagem TechFlow, incluindo declaração de variáveis, operações aritméticas, estruturas condicionais e saída.