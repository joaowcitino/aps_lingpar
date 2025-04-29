from techflow.tokenizer import Tokenizer
from techflow.ast import (
    Node, BinOp, UnOp, IntVal, StringVal, BoolVal, 
    Identifier, Assignment, VarDeclaration, Print, 
    Block, NoOp, If, While, Repeat, SwitchCase, Reader, Program
)

class Parser:
    @staticmethod
    def parseDeclaration(tokenizer: Tokenizer) -> Node:
        tokenizer.selectNext()
        if tokenizer.next.type != 'IDENTIFIER':
            raise ValueError("Esperado identificador após 'byte'")
        id_token = tokenizer.next
        
        tokenizer.selectNext()
        if tokenizer.next.type != ':':
            raise ValueError("Esperado ':' após identificador na declaração")
        tokenizer.selectNext()
        if tokenizer.next.type != 'TYPE':
            raise ValueError("Esperado tipo após ':' na declaração")
        declared_type = tokenizer.next.value
        
        if declared_type not in ['i32', 'bool', 'str']:
            raise ValueError("Tipo inválido: " + declared_type)
        tokenizer.selectNext()
        initial_expr = None
        if tokenizer.next.type == '=':
            tokenizer.selectNext()
            
            if tokenizer.next.type == ';':
                raise ValueError(f"Inicialização vazia para variável '{id_token.value}'")
                
            initial_expr = Parser.parseExpression(tokenizer)
        
        if tokenizer.next.type != ';':
            raise ValueError("Esperado ';' no fim da declaração")
        tokenizer.selectNext()
        return VarDeclaration(id_token.value, declared_type, initial_expr)

    @staticmethod
    def parseStatement(tokenizer: Tokenizer) -> Node:
        if tokenizer.next.type == 'PING':
            tokenizer.selectNext()
            if tokenizer.next.type != '(':
                raise ValueError("Esperado '(' após 'ping'")
            tokenizer.selectNext()
            condition = Parser.parseExpression(tokenizer)
            if tokenizer.next.type != ')':
                raise ValueError("Esperado ')' após condição do ping")
            tokenizer.selectNext()
            if tokenizer.next.type != 'THEN':
                raise ValueError("Esperado 'then' após condição do ping")
            tokenizer.selectNext()
            then_branch = Parser.parseBlockThen(tokenizer)
            else_branch = None
            if tokenizer.next.type == 'PONG':
                tokenizer.selectNext()
                if tokenizer.next.type != 'THEN':
                    raise ValueError("Esperado 'then' após 'pong'")
                tokenizer.selectNext()
                else_branch = Parser.parseBlockThen(tokenizer)
            if tokenizer.next.type != 'END':
                raise ValueError("Esperado 'end' para fechar bloco ping-pong")
            tokenizer.selectNext()
            return If(condition, then_branch, else_branch)
            
        elif tokenizer.next.type == 'STREAM':
            tokenizer.selectNext()
            if tokenizer.next.type != '(':
                raise ValueError("Esperado '(' após 'stream'")
            tokenizer.selectNext()
            condition = Parser.parseExpression(tokenizer)
            if tokenizer.next.type != ')':
                raise ValueError("Esperado ')' após condição do stream")
            tokenizer.selectNext()
            if tokenizer.next.type != 'THEN':
                raise ValueError("Esperado 'then' após condição do stream")
            tokenizer.selectNext()
            body = Parser.parseBlockThen(tokenizer)
            if tokenizer.next.type != 'END':
                raise ValueError("Esperado 'end' para fechar bloco stream")
            tokenizer.selectNext()
            return While(condition, body)
            
        elif tokenizer.next.type == 'REPEAT':
            tokenizer.selectNext()
            if tokenizer.next.type != 'THEN':
                raise ValueError("Esperado 'then' após 'repeat'")
            tokenizer.selectNext()
            body = Parser.parseBlockThen(tokenizer)
            if tokenizer.next.type != 'UNTIL':
                raise ValueError("Esperado 'until' após bloco repeat")
            tokenizer.selectNext()
            condition = Parser.parseExpression(tokenizer)
            if tokenizer.next.type != ';':
                raise ValueError("Esperado ';' após condição do repeat-until")
            tokenizer.selectNext()
            return Repeat(body, condition)
            
        elif tokenizer.next.type == 'SELECT':
            tokenizer.selectNext()
            if tokenizer.next.type != '(':
                raise ValueError("Esperado '(' após 'select'")
            tokenizer.selectNext()
            condition = Parser.parseExpression(tokenizer)
            if tokenizer.next.type != ')':
                raise ValueError("Esperado ')' após condição do select")
            tokenizer.selectNext()
            if tokenizer.next.type != 'THEN':
                raise ValueError("Esperado 'then' após condição do select")
            tokenizer.selectNext()
            
            cases = []
            default_case = None
            
            while tokenizer.next.type == 'WHEN' or tokenizer.next.type == 'OTHERWISE':
                if tokenizer.next.type == 'WHEN':
                    tokenizer.selectNext()
                    case_value = Parser.parseExpression(tokenizer)
                    if tokenizer.next.type != 'THEN':
                        raise ValueError("Esperado 'then' após valor do case")
                    tokenizer.selectNext()
                    case_block = Parser.parseBlockThen(tokenizer)
                    if tokenizer.next.type != 'END':
                        raise ValueError("Esperado 'end' para fechar bloco case")
                    tokenizer.selectNext()
                    cases.append((case_value, case_block))
                else:
                    tokenizer.selectNext()
                    if tokenizer.next.type != 'THEN':
                        raise ValueError("Esperado 'then' após 'otherwise'")
                    tokenizer.selectNext()
                    default_case = Parser.parseBlockThen(tokenizer)
                    if tokenizer.next.type != 'END':
                        raise ValueError("Esperado 'end' para fechar bloco otherwise")
                    tokenizer.selectNext()
                    break
                    
            if tokenizer.next.type != 'END':
                raise ValueError("Esperado 'end' para fechar bloco select")
            tokenizer.selectNext()
            
            return SwitchCase(condition, cases, default_case)
            
        elif tokenizer.next.type == 'LOG':
            tokenizer.selectNext()
            if tokenizer.next.type != '(':
                raise ValueError("Esperado '(' após 'log'")
            tokenizer.selectNext()
            expr = Parser.parseExpression(tokenizer)
            if tokenizer.next.type != ')':
                raise ValueError("Esperado ')' após expressão em log")
            tokenizer.selectNext()
            if tokenizer.next.type == ';':
                tokenizer.selectNext()
            return Print(expr)
            
        elif tokenizer.next.type == 'BYTE':
            return Parser.parseDeclaration(tokenizer)
            
        elif tokenizer.next.type == 'IDENTIFIER':
            token = tokenizer.next
            tokenizer.selectNext()
            if tokenizer.next.type == '=':
                tokenizer.selectNext()
                expr = Parser.parseExpression(tokenizer)
                if tokenizer.next.type == ';':
                    tokenizer.selectNext()
                return Assignment(Identifier(token.value), expr)
            else:
                return Identifier(token.value)
                
        elif tokenizer.next.type == 'READER':
            reader_node = Reader()
            tokenizer.selectNext()
            if tokenizer.next.type == '(':
                tokenizer.selectNext()
                if tokenizer.next.type != ')':
                    raise ValueError("Esperado ')' após reader")
                tokenizer.selectNext()
            if tokenizer.next.type == ';':
                tokenizer.selectNext()
            return reader_node
            
        else:
            raise ValueError("Valor inesperado: " + tokenizer.next.type)

    @staticmethod
    def parseExprStatement(tokenizer: Tokenizer) -> Node:
        expr = Parser.parseExpression(tokenizer)
        if tokenizer.next.type == ';':
            tokenizer.selectNext()
        return expr

    @staticmethod
    def parseStatements(tokenizer: Tokenizer) -> list:
        statements = []
        while tokenizer.next.type not in ['END', 'PONG', 'UNTIL', 'WHEN', 'OTHERWISE', 'SHUTDOWN', 'EOF']:
            if tokenizer.next.type == ';':
                tokenizer.selectNext()
                statements.append(NoOp())
            else:
                try:
                    stmt = Parser.parseStatement(tokenizer)
                    statements.append(stmt)
                    valid_next_tokens = ['PING', 'STREAM', 'REPEAT', 'BYTE', 'IDENTIFIER', 'LOG', 'READER', 'SELECT',
                                         '(', ')', '+', '-', '*', '/', '%', '>', '<', 'LE', 'GE', 'EQ', 'NEQ', 'AND', 
                                         'OR', 'NOT', 'END', 'PONG', 'UNTIL', 'WHEN', 'OTHERWISE', 'SHUTDOWN', 'EOF']
                    if not isinstance(stmt, (If, While, Repeat, SwitchCase, Block)) and tokenizer.next.type not in valid_next_tokens:
                        if tokenizer.next.type == ';':
                            tokenizer.selectNext()
                        else:
                            raise ValueError(f"Esperado ';' ou um novo comando, encontrado: {tokenizer.next.type}")
                except Exception as e:
                    raise e
        return statements

    @staticmethod
    def parseBlockThen(tokenizer: Tokenizer) -> Node:
        stmts = Parser.parseStatements(tokenizer)
        return Block(stmts)

    @staticmethod
    def parseFactor(tokenizer: Tokenizer) -> Node:
        if tokenizer.next.type in ['+', '-', 'NOT']:
            op = tokenizer.next.type
            tokenizer.selectNext()
            
            if tokenizer.next.type in [';', 'EOF']:
                raise ValueError(f"Expressão incompleta: operador '{op}' sem operando")
                
            child = Parser.parseFactor(tokenizer)
            if op == 'NOT':
                return UnOp('not', child)
            else:
                return UnOp(op, child)
                
        if tokenizer.next.type == '(':
            tokenizer.selectNext()
            node = Parser.parseExpression(tokenizer)
            if tokenizer.next.type != ')':
                raise ValueError(f"Falta ')' para fechar a expressão, encontrado: {tokenizer.next.type}")
            tokenizer.selectNext()
            return node
            
        if tokenizer.next.type == 'NUMBER':
            node = IntVal(tokenizer.next.value)
            tokenizer.selectNext()
            return node
            
        if tokenizer.next.type == 'STRING':
            node = StringVal(tokenizer.next.value)
            tokenizer.selectNext()
            return node
            
        if tokenizer.next.type == 'BOOLEAN':
            value = True if tokenizer.next.value == 'true' else False
            node = BoolVal(value)
            tokenizer.selectNext()
            return node
            
        if tokenizer.next.type == 'IDENTIFIER':
            node = Identifier(tokenizer.next.value)
            tokenizer.selectNext()
            return node
            
        if tokenizer.next.type == 'READER':
            reader_node = Reader()
            tokenizer.selectNext()
            if tokenizer.next.type == '(':
                tokenizer.selectNext()
                if tokenizer.next.type != ')':
                    raise ValueError("Esperado ')' após reader/scanf")
                tokenizer.selectNext()
            return reader_node
            
        raise ValueError(f"Fator inesperado: {tokenizer.next.type}")

    @staticmethod
    def parseTerm(tokenizer: Tokenizer) -> Node:
        node = Parser.parseFactor(tokenizer)
        while tokenizer.next.type in ['*', '/', '%']:
            op = tokenizer.next.type
            tokenizer.selectNext()
            
            if tokenizer.next.type in [';', 'EOF']:
                raise ValueError(f"Expressão incompleta: operador '{op}' sem operando direito")
                
            right_node = Parser.parseFactor(tokenizer)
            node = BinOp(op, node, right_node)
        return node

    @staticmethod
    def parseAdditive(tokenizer: Tokenizer) -> Node:
        node = Parser.parseTerm(tokenizer)
        while tokenizer.next.type in ['+', '-']:
            op = tokenizer.next.type
            tokenizer.selectNext()
            
            if tokenizer.next.type in [';', 'EOF']:
                raise ValueError(f"Expressão incompleta: operador '{op}' sem operando direito")
                
            right_node = Parser.parseTerm(tokenizer)
            node = BinOp(op, node, right_node)
        return node

    @staticmethod
    def parseRelational(tokenizer: Tokenizer) -> Node:
        node = Parser.parseAdditive(tokenizer)
        while tokenizer.next.type in ['<', '>', 'LE', 'GE']:
            op = tokenizer.next.type
            tokenizer.selectNext()
            
            if tokenizer.next.type in [';', 'EOF']:
                raise ValueError(f"Expressão incompleta: operador '{op}' sem operando direito")
                
            right_node = Parser.parseAdditive(tokenizer)
            node = BinOp(op, node, right_node)
        return node

    @staticmethod
    def parseEquality(tokenizer: Tokenizer) -> Node:
        node = Parser.parseRelational(tokenizer)
        while tokenizer.next.type in ['EQ', 'NEQ']:
            op = tokenizer.next.type
            tokenizer.selectNext()
            
            if tokenizer.next.type in [';', 'EOF']:
                raise ValueError(f"Expressão incompleta: operador '{op}' sem operando direito")
                
            right_node = Parser.parseRelational(tokenizer)
            node = BinOp(op, node, right_node)
        return node

    @staticmethod
    def parseLogicalAnd(tokenizer: Tokenizer) -> Node:
        node = Parser.parseEquality(tokenizer)
        while tokenizer.next.type == 'AND':
            op = tokenizer.next.type
            tokenizer.selectNext()
            
            if tokenizer.next.type in [';', 'EOF']:
                raise ValueError(f"Expressão incompleta: operador '{op}' sem operando direito")
                
            right_node = Parser.parseEquality(tokenizer)
            node = BinOp(op, node, right_node)
        return node

    @staticmethod
    def parseLogicalOr(tokenizer: Tokenizer) -> Node:
        node = Parser.parseLogicalAnd(tokenizer)
        while tokenizer.next.type == 'OR':
            op = tokenizer.next.type
            tokenizer.selectNext()
            
            if tokenizer.next.type in [';', 'EOF']:
                raise ValueError(f"Expressão incompleta: operador '{op}' sem operando direito")
                
            right_node = Parser.parseLogicalAnd(tokenizer)
            node = BinOp(op, node, right_node)
        return node

    @staticmethod
    def parseConcat(tokenizer: Tokenizer) -> Node:
        node = Parser.parseLogicalOr(tokenizer)
        while tokenizer.next.type == 'CONCAT':
            op = tokenizer.next.type
            tokenizer.selectNext()
            
            if tokenizer.next.type in [';', 'EOF']:
                raise ValueError(f"Expressão incompleta: operador '++' sem operando direito")
                
            right_node = Parser.parseLogicalOr(tokenizer)
            node = BinOp(op, node, right_node)
        return node
        
    @staticmethod
    def parseExpression(tokenizer: Tokenizer) -> Node:
        try:
            return Parser.parseConcat(tokenizer)
        except Exception as e:
            raise e

    @staticmethod
    def run(code: str) -> Node:
        tokenizer = Tokenizer(code)
        
        if tokenizer.next.type == 'EOF':
            raise ValueError("Arquivo vazio")
            
        if tokenizer.next.type != 'BOOT':
            raise ValueError("Esperado 'boot' para iniciar o programa")
        tokenizer.selectNext()
        
        body = Parser.parseBlockThen(tokenizer)
        
        if tokenizer.next.type != 'SHUTDOWN':
            raise ValueError("Esperado 'shutdown' para finalizar o programa")
        tokenizer.selectNext()
        
        program = Program(body)
            
        if tokenizer.next.type != 'EOF':
            raise ValueError('Tokens não consumidos completamente')
        return program