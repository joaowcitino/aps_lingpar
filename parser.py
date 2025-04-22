import ply.yacc as yacc
from lexer import tokens

# AST NODES

class Programa:
    def __init__(self, nome, corpo):
        self.nome = nome
        self.corpo = corpo

    def __repr__(self):
        return f"<Programa {self.nome} com {len(self.corpo)} instruções>"

class ComandoLog:
    def __init__(self, valor):
        self.valor = valor
    def __repr__(self):
        return f"<Log {self.valor}>"

class DeclaracaoVariavel:
    def __init__(self, tipo, nome, valor):
        self.tipo = tipo
        self.nome = nome
        self.valor = valor
    def __repr__(self):
        return f"<{self.tipo} {self.nome} = {self.valor}>"

class BlocoIf:
    def __init__(self, condicao, corpo):
        self.condicao = condicao
        self.corpo = corpo
    def __repr__(self):
        return f"<If {self.condicao} then {len(self.corpo)} instruções>"

class BlocoElse:
    def __init__(self, corpo):
        self.corpo = corpo
    def __repr__(self):
        return f"<Else {len(self.corpo)} instruções>"

class Condicional:
    def __init__(self, if_block, else_block=None):
        self.if_block = if_block
        self.else_block = else_block
    def __repr__(self):
        return f"<Condicional {self.if_block} else {self.else_block}>"

class LoopPara:
    def __init__(self, var, inicio, corpo):
        self.var = var
        self.inicio = inicio
        self.corpo = corpo
    def __repr__(self):
        return f"<Loop {self.var} from {self.inicio}>"

class LoopEnquanto:
    def __init__(self, condicao, corpo):
        self.condicao = condicao
        self.corpo = corpo
    def __repr__(self):
        return f"<While {self.condicao}>"

class FuncaoDefinida:
    def __init__(self, nome, parametros, corpo, retorno):
        self.nome = nome
        self.parametros = parametros
        self.corpo = corpo
        self.retorno = retorno
    def __repr__(self):
        return f"<Funcao {self.nome}({', '.join(self.parametros)})>"

class ChamadaFuncao:
    def __init__(self, nome, argumentos):
        self.nome = nome
        self.argumentos = argumentos
    def __repr__(self):
        return f"<Chamada {self.nome}()>"

class Retorno:
    def __init__(self, valor):
        self.valor = valor
    def __repr__(self):
        return f"<Retorno {self.valor}>"

# REGRAS DE PROGRAMA E DECLARAÇÕES

def p_programa(p):
    '''programa : BOOT IDENT declaracoes SHUTDOWN IDENT'''
    p[0] = Programa(p[2], p[3])

def p_declaracoes_lista(p):
    '''declaracoes : declaracoes declaracao'''
    p[0] = p[1] + [p[2]]

def p_declaracoes_unica(p):
    '''declaracoes : declaracao'''
    p[0] = [p[1]]

def p_tipo(p):
    '''tipo : BYTE
            | BIT
            | CHIP'''
    p[0] = p[1]

def p_declaracao_variavel(p):
    '''declaracao : tipo IDENT ASSIGN expressao'''
    p[0] = DeclaracaoVariavel(p[1], p[2], p[4])

def p_declaracao_log(p):
    '''declaracao : LOG expressao'''
    p[0] = ComandoLog(p[2])

def p_declaracao_condicional(p):
    '''declaracao : bloco_if
                  | bloco_if bloco_else'''
    p[0] = Condicional(p[1], p[2] if len(p) > 2 else None)

def p_bloco_if(p):
    '''bloco_if : PING expressao PROCESS declaracoes IDENT'''
    p[0] = BlocoIf(p[2], p[4])

def p_bloco_else(p):
    '''bloco_else : PONG PROCESS declaracoes IDENT'''
    p[0] = BlocoElse(p[3])

def p_declaracao_loop_para(p):
    '''declaracao : LOOP expressao AS IDENT PROCESS declaracoes LOOP_END'''
    p[0] = LoopPara(p[4], p[2], p[6])

def p_declaracao_loop_enquanto(p):
    '''declaracao : STREAM expressao PROCESS declaracoes STREAM_END'''
    p[0] = LoopEnquanto(p[2], p[4])

def p_declaracao_funcao(p):
    '''declaracao : MODULE IDENT LPAREN parametros RPAREN PROCESS declaracoes MODULE_END'''
    p[0] = FuncaoDefinida(p[2], p[4], p[7], None)

def p_declaracao_retorno(p):
    '''declaracao : RETURN expressao'''
    p[0] = Retorno(p[2])

def p_parametros(p):
    '''parametros : IDENT
                  | parametros COMMA IDENT'''
    p[0] = [p[1]] if len(p) == 2 else p[1] + [p[3]]

def p_retorno_opt(p):
    '''retorno_opt : RETURN expressao
                   | empty'''
    p[0] = Retorno(p[2]) if len(p) == 3 else None

def p_declaracao_chamada_funcao(p):
    '''declaracao : IDENT LPAREN argumentos RPAREN'''
    p[0] = ChamadaFuncao(p[1], p[3])

def p_argumentos(p):
    '''argumentos : expressao
                  | argumentos COMMA expressao'''
    p[0] = [p[1]] if len(p) == 2 else p[1] + [p[3]]

# EXPRESSÕES

def p_expressao_binaria(p):
    '''expressao : expressao PLUS expressao
                 | expressao MINUS expressao
                 | expressao TIMES expressao
                 | expressao DIVIDE expressao'''
    p[0] = ('binop', p[2], p[1], p[3])

def p_expressao_comparacao(p):
    '''expressao : expressao EQ expressao
                 | expressao NEQ expressao
                 | expressao GT expressao
                 | expressao LT expressao
                 | expressao GE expressao
                 | expressao LE expressao'''
    p[0] = ('cmp', p[2], p[1], p[3])

def p_expressao_valor(p):
    '''expressao : NUMBER
                 | STRING
                 | IDENT'''
    if isinstance(p[1], int):
        p[0] = p[1]
    elif isinstance(p[1], str) and p.slice[1].type == 'STRING':
        p[0] = p[1]
    else:
        p[0] = ('var', p[1])

def p_expressao_chamada(p):
    '''expressao : IDENT LPAREN argumentos RPAREN'''
    p[0] = ChamadaFuncao(p[1], p[3])

def p_empty(p):
    'empty :'
    pass

def p_error(p):
    if p:
        print(f"[ERRO SINTÁTICO] Token inesperado: {p.type} (valor: {p.value})")
    else:
        print("[ERRO SINTÁTICO] Fim de arquivo inesperado.")

parser = yacc.yacc()