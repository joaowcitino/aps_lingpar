from parser import (
    DeclaracaoVariavel, ComandoLog, Condicional, BlocoIf, BlocoElse,
    LoopPara, LoopEnquanto, FuncaoDefinida, ChamadaFuncao, Retorno
)

class Interpretador:
    def __init__(self):
        self.contexto = {}
        self.funcoes = {}

    def executar_programa(self, programa):
        for instrucao in programa.corpo:
            resultado = self.executar(instrucao)
            if isinstance(resultado, Retorno):
                return resultado.valor

    def executar(self, instrucao):
        if isinstance(instrucao, DeclaracaoVariavel):
            valor = self.avaliar_expressao(instrucao.valor)
            self.contexto[instrucao.nome] = valor

        elif isinstance(instrucao, ComandoLog):
            print(f"[log] {self.avaliar_expressao(instrucao.valor)}")

        elif isinstance(instrucao, Condicional):
            cond = self.avaliar_expressao(instrucao.if_block.condicao)
            bloco = instrucao.if_block if cond else instrucao.else_block
            if bloco:
                for i in bloco.corpo:
                    self.executar(i)

        elif isinstance(instrucao, LoopPara):
            limite = self.avaliar_expressao(instrucao.inicio)
            for i in range(limite):
                self.contexto[instrucao.var] = i
                for cmd in instrucao.corpo:
                    self.executar(cmd)

        elif isinstance(instrucao, LoopEnquanto):
            while self.avaliar_expressao(instrucao.condicao):
                for cmd in instrucao.corpo:
                    self.executar(cmd)

        elif isinstance(instrucao, FuncaoDefinida):
            self.funcoes[instrucao.nome] = instrucao

        elif isinstance(instrucao, ChamadaFuncao):
            return self.executar_chamada(instrucao)

        elif isinstance(instrucao, Retorno):
            return Retorno(self.avaliar_expressao(instrucao.valor))

        else:
            raise Exception(f"Instrução não reconhecida: {instrucao}")

    def executar_chamada(self, chamada):
        funcao = self.funcoes.get(chamada.nome)
        if not funcao:
            raise Exception(f"Função '{chamada.nome}' não definida")

        contexto_antigo = self.contexto.copy()
        self.contexto = {}

        for nome, valor_expr in zip(funcao.parametros, chamada.argumentos):
            self.contexto[nome] = self.avaliar_expressao(valor_expr)

        retorno = None
        for cmd in funcao.corpo:
            resultado = self.executar(cmd)
            if isinstance(resultado, Retorno):
                retorno = resultado.valor
                break

        self.contexto = contexto_antigo
        return retorno

    def avaliar_expressao(self, expr):
        if isinstance(expr, (int, str)):
            return expr
        if isinstance(expr, tuple):
            tipo = expr[0]
            if tipo == 'var':
                return self.contexto.get(expr[1], 0)
            elif tipo == 'binop':
                op, esq, dir = expr[1], expr[2], expr[3]
                v1, v2 = self.avaliar_expressao(esq), self.avaliar_expressao(dir)
                return eval(f"{v1} {op} {v2}")
            elif tipo == 'cmp':
                op, esq, dir = expr[1], expr[2], expr[3]
                v1, v2 = self.avaliar_expressao(esq), self.avaliar_expressao(dir)
                return eval(f"{v1} {op} {v2}")
            elif isinstance(expr[0], ChamadaFuncao):
                return self.executar(expr)
        if isinstance(expr, ChamadaFuncao):
            return self.executar_chamada(expr)
        raise Exception(f"Expressão inválida: {expr}")