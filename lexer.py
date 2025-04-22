import ply.lex as lex

tokens = [
    'IDENT',
    'NUMBER',
    'STRING',
    'PLUS', 'MINUS', 'TIMES', 'DIVIDE',
    'EQ', 'NEQ', 'GT', 'LT', 'GE', 'LE',
    'AND', 'OR',
    'ASSIGN', 'LPAREN', 'RPAREN', 'COMMA',
]

reserved = {
    'boot': 'BOOT',
    'shutdown': 'SHUTDOWN',
    'byte': 'BYTE',
    'bit': 'BIT',
    'chip': 'CHIP',
    'log': 'LOG',
    'ping': 'PING',
    'pong': 'PONG',
    'loop': 'LOOP',
    'loop_end': 'LOOP_END',
    'stream': 'STREAM',
    'stream_end': 'STREAM_END',
    'process': 'PROCESS',
    'as': 'AS',
    'module': 'MODULE',
    'module_end': 'MODULE_END',
    'return': 'RETURN'
}

tokens += list(reserved.values())

t_PLUS    = r'\+'
t_MINUS   = r'-'
t_TIMES   = r'\*'
t_DIVIDE  = r'/'
t_EQ      = r'=='
t_NEQ     = r'!='
t_GT      = r'>'
t_LT      = r'<'
t_GE      = r'>='
t_LE      = r'<='
t_AND     = r'&&'
t_OR      = r'\|\|'
t_ASSIGN  = r'='
t_LPAREN  = r'\('
t_RPAREN  = r'\)'
t_COMMA   = r','

def t_STRING(t):
    r'"([^"\\]|\\.)*"'
    t.value = t.value[1:-1]
    return t

def t_NUMBER(t):
    r'\d+'
    t.value = int(t.value)
    return t

def t_IDENT(t):
    r'[a-zA-Z_][a-zA-Z0-9_]*'
    t.type = reserved.get(t.value, 'IDENT')
    return t

def t_COMMENT(t):
    r'\#.*'
    pass

t_ignore = ' \t'

def t_newline(t):
    r'\n+'
    t.lexer.lineno += len(t.value)

def t_error(t):
    print(f"Caractere inválido '{t.value[0]}' na linha {t.lineno}")
    t.lexer.skip(1)

lexer = lex.lex()