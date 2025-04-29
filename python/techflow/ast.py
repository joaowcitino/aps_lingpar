from abc import ABC, abstractmethod
from techflow.symboltable import SymbolTable

class Node(ABC):
    def __init__(self, value):
        self.value = value
        self.children = []
        
    @abstractmethod
    def Evaluate(self, symbol_table: SymbolTable):
        pass

class BinOp(Node):
    def __init__(self, op, left: Node, right: Node):
        super().__init__(op)
        self.children = [left, right]
        
    def Evaluate(self, symbol_table: SymbolTable):
        left_val, left_type = self.children[0].Evaluate(symbol_table)
        right_val, right_type = self.children[1].Evaluate(symbol_table)
        
        if self.value == '+':
            if left_type == "i32" and right_type == "i32":
                return (left_val + right_val, "i32")
            else:
                raise ValueError("Operação '+' requer operandos i32")
                
        elif self.value == '-':
            if left_type == "i32" and right_type == "i32":
                return (left_val - right_val, "i32")
            else:
                raise ValueError("Operação '-' requer operandos i32")
                
        elif self.value == '*':
            if left_type == "i32" and right_type == "i32":
                return (left_val * right_val, "i32")
            else:
                raise ValueError("Operação '*' requer operandos i32")
                
        elif self.value == '/':
            if left_type == "i32" and right_type == "i32":
                return (left_val // right_val, "i32")
            else:
                raise ValueError("Operação '/' requer operandos i32")
                
        elif self.value == '%':
            if left_type == "i32" and right_type == "i32":
                return (left_val % right_val, "i32")
            else:
                raise ValueError("Operação '%' requer operandos i32")
                
        elif self.value in ['<', '>', 'LE', 'GE']:
            if left_type == "i32" and right_type == "i32":
                if self.value == '<':
                    return (left_val < right_val, "bool")
                elif self.value == '>':
                    return (left_val > right_val, "bool")
                elif self.value == 'LE':
                    return (left_val <= right_val, "bool")
                elif self.value == 'GE':
                    return (left_val >= right_val, "bool")
            elif left_type == "str" and right_type == "str":
                if self.value == '<':
                    return (left_val < right_val, "bool")
                elif self.value == '>':
                    return (left_val > right_val, "bool")
                elif self.value == 'LE':
                    return (left_val <= right_val, "bool")
                elif self.value == 'GE':
                    return (left_val >= right_val, "bool")
            else:
                raise ValueError("Operadores relacionais requerem operandos i32 ou str do mesmo tipo")
                
        elif self.value in ['EQ', 'NEQ']:
            if left_type != right_type:
                raise ValueError("Comparação de igualdade entre tipos diferentes")
            if self.value == 'EQ':
                return (left_val == right_val, "bool")
            else:
                return (left_val != right_val, "bool")
                
        elif self.value == 'AND':
            if left_type == "bool" and right_type == "bool":
                return (left_val and right_val, "bool")
            else:
                raise ValueError("Operador 'AND' requer operandos bool")
                
        elif self.value == 'OR':
            if left_type == "bool" and right_type == "bool":
                return (left_val or right_val, "bool")
            else:
                raise ValueError("Operador 'OR' requer operandos bool")
                
        elif self.value == 'CONCAT':
            if left_type == "bool":
                left_str = "true" if left_val else "false"
            else:
                left_str = str(left_val)
                
            if right_type == "bool":
                right_str = "true" if right_val else "false"
            else:
                right_str = str(right_val)
                
            return (left_str + right_str, "str")
            
        else:
            raise ValueError(f"Operador desconhecido: {self.value}")

class UnOp(Node):
    def __init__(self, op, child: Node):
        super().__init__(op)
        self.children = [child]
        
    def Evaluate(self, symbol_table: SymbolTable):
        child_val, child_type = self.children[0].Evaluate(symbol_table)
        
        if self.value == '+':
            if child_type == "i32":
                return (child_val, "i32")
            else:
                raise ValueError("Operador unário '+' requer operando i32")
                
        elif self.value == '-':
            if child_type == "i32":
                return (-child_val, "i32")
            else:
                raise ValueError("Operador unário '-' requer operando i32")
                
        elif self.value == 'not':
            if child_type == "bool":
                return (not child_val, "bool")
            else:
                raise ValueError("Operador 'not' requer operando bool")
                
        else:
            raise ValueError(f"Operador desconhecido: {self.value}")

class IntVal(Node):
    def __init__(self, value: int):
        super().__init__(value)
        
    def Evaluate(self, symbol_table: SymbolTable):
        return (self.value, "i32")

class StringVal(Node):
    def __init__(self, value: str):
        super().__init__(value)
        
    def Evaluate(self, symbol_table: SymbolTable):
        return (self.value, "str")

class BoolVal(Node):
    def __init__(self, value: bool):
        super().__init__(value)
        
    def Evaluate(self, symbol_table: SymbolTable):
        return (self.value, "bool")

class Identifier(Node):
    def __init__(self, name: str):
        super().__init__(name)
        
    def Evaluate(self, symbol_table: SymbolTable):
        return symbol_table.get(self.value)

class Assignment(Node):
    def __init__(self, identifier: Identifier, expression: Node):
        super().__init__('=')
        self.identifier = identifier
        self.expression = expression
        
    def Evaluate(self, symbol_table: SymbolTable):
        expr_val, expr_type = self.expression.Evaluate(symbol_table)
        try:
            current_val, current_type = symbol_table.get(self.identifier.value)
            if current_type != expr_type:
                raise ValueError(f"Atribuição com tipos incompatíveis: variável '{self.identifier.value}' do tipo {current_type} recebendo {expr_type}")
            symbol_table.set(self.identifier.value, expr_val, current_type)
            return (expr_val, current_type)
        except ValueError:
            symbol_table.set(self.identifier.value, expr_val, expr_type)
            return (expr_val, expr_type)

class VarDeclaration(Node):
    def __init__(self, identifier, declared_type, initial_expr=None):
        super().__init__('byte_decl')
        self.identifier = identifier
        self.declared_type = declared_type
        self.initial_expr = initial_expr
        
    def Evaluate(self, symbol_table: SymbolTable):
        if self.initial_expr is not None:
            val, val_type = self.initial_expr.Evaluate(symbol_table)
            if val_type != self.declared_type:
                raise ValueError(f"Tipo incompatível na inicialização de '{self.identifier}': esperado {self.declared_type}, obtido {val_type}")
            
            symbol_table.set(self.identifier, val, self.declared_type)
            return (val, self.declared_type)
        else:
            if self.declared_type == "i32":
                default_value = 0
            elif self.declared_type == "bool":
                default_value = False
            elif self.declared_type == "str":
                default_value = ""
            else:
                default_value = None
            symbol_table.set(self.identifier, default_value, self.declared_type)
            return (default_value, self.declared_type)

class Print(Node):
    def __init__(self, expression: Node):
        super().__init__('log')
        self.expression = expression
        
    def Evaluate(self, symbol_table: SymbolTable):
        val, typ = self.expression.Evaluate(symbol_table)
        if typ == "bool":
            print("true" if val else "false")
        else:
            print(val)
        return (val, typ)

class Block(Node):
    def __init__(self, statements: list):
        super().__init__('block')
        self.statements = statements
        
    def Evaluate(self, symbol_table: SymbolTable):
        result = (None, None)
        for stmt in self.statements:
            result = stmt.Evaluate(symbol_table)
        return result

class NoOp(Node):
    def __init__(self):
        super().__init__(None)
        
    def Evaluate(self, symbol_table: SymbolTable):
        return (None, None)

class If(Node):
    def __init__(self, condition: Node, then_branch: Node, else_branch: Node = None):
        super().__init__('ping')
        self.condition = condition
        self.then_branch = then_branch
        self.else_branch = else_branch
        
    def Evaluate(self, symbol_table: SymbolTable):
        cond_val, cond_type = self.condition.Evaluate(symbol_table)
        if cond_type != "bool":
            raise ValueError("Condição do ping deve ser booleana")
        if cond_val:
            return self.then_branch.Evaluate(symbol_table)
        elif self.else_branch is not None:
            return self.else_branch.Evaluate(symbol_table)
        return (None, None)

class While(Node):
    def __init__(self, condition: Node, body: Node):
        super().__init__('stream')
        self.condition = condition
        self.body = body
        
    def Evaluate(self, symbol_table: SymbolTable):
        while True:
            cond_val, cond_type = self.condition.Evaluate(symbol_table)
            if cond_type != "bool":
                raise ValueError("Condição do stream deve ser booleana")
            if not cond_val:
                break
            self.body.Evaluate(symbol_table)
        return (None, None)

class Repeat(Node):
    def __init__(self, body: Node, condition: Node):
        super().__init__('repeat')
        self.body = body
        self.condition = condition
        
    def Evaluate(self, symbol_table: SymbolTable):
        while True:
            self.body.Evaluate(symbol_table)
            cond_val, cond_type = self.condition.Evaluate(symbol_table)
            if cond_type != "bool":
                raise ValueError("Condição do repeat-until deve ser booleana")
            if cond_val:
                break
        return (None, None)

class SwitchCase(Node):
    def __init__(self, condition: Node, cases: list, default_case: Node = None):
        super().__init__('select')
        self.condition = condition
        self.cases = cases
        self.default_case = default_case
        
    def Evaluate(self, symbol_table: SymbolTable):
        cond_val, cond_type = self.condition.Evaluate(symbol_table)
        
        for case_value, case_block in self.cases:
            case_val, case_type = case_value.Evaluate(symbol_table)
            
            if cond_type != case_type:
                raise ValueError(f"Tipo incompatível no select: {cond_type} vs {case_type}")
                
            if cond_val == case_val:
                return case_block.Evaluate(symbol_table)
                
        if self.default_case is not None:
            return self.default_case.Evaluate(symbol_table)
            
        return (None, None)

class Reader(Node):
    def __init__(self):
        super().__init__('reader')
        
    def Evaluate(self, symbol_table: SymbolTable):
        try:
            return (int(input()), "i32")
        except:
            raise ValueError("Valor inválido para reader")

class Program(Node):
    def __init__(self, body: Node):
        super().__init__('program')
        self.body = body
        
    def Evaluate(self, symbol_table: SymbolTable):
        return self.body.Evaluate(symbol_table)