class Token:
    def __init__(self, type: str, value) -> None:
        self.type = type
        self.value = value

class Tokenizer:
    def __init__(self, source: str) -> None:
        self.source = source
        self.position = 0
        self.next = None
        self.selectNext()
        
    def selectNext(self) -> None:
        while self.position < len(self.source) and self.source[self.position].isspace():
            self.position += 1
            
        if self.position >= len(self.source):
            self.next = Token('EOF', None)
            return
            
        char = self.source[self.position]
        
        if char.isdigit():
            num = ''
            while self.position < len(self.source) and self.source[self.position].isdigit():
                num += self.source[self.position]
                self.position += 1
            self.next = Token('NUMBER', int(num))
            
        elif char.isalpha():
            id_str = ''
            while self.position < len(self.source) and (self.source[self.position].isalnum() or self.source[self.position]=='_'):
                id_str += self.source[self.position]
                self.position += 1
                
            if id_str in ['boot', 'shutdown', 'byte', 'stream', 'ping', 'pong', 
                         'log', 'repeat', 'until', 'then', 'end', 'true', 'false',
                         'select', 'when', 'otherwise']:
                if id_str == 'byte':
                    self.next = Token('BYTE', id_str)
                elif id_str in ['true', 'false']:
                    self.next = Token('BOOLEAN', id_str)
                elif id_str == 'log':
                    self.next = Token('LOG', id_str)
                else:
                    self.next = Token(id_str.upper(), id_str)
            elif id_str in ['i32', 'bool', 'str']:
                self.next = Token('TYPE', id_str)
            else:
                self.next = Token('IDENTIFIER', id_str)
                
        elif char == '"':
            self.position += 1
            str_lit = ''
            while self.position < len(self.source) and self.source[self.position] != '"':
                str_lit += self.source[self.position]
                self.position += 1
                
            if self.position >= len(self.source):
                raise ValueError("String literal não fechado")
                
            self.position += 1
            self.next = Token('STRING', str_lit)
            
        elif char == '+':
            if self.position + 1 < len(self.source) and self.source[self.position+1] == '+':
                self.next = Token('CONCAT', '++')
                self.position += 2
            else:
                self.next = Token('+', '+')
                self.position += 1
                
        elif char in ['-', '*', '/', '(', ')', ';', '{', '}', ':', ',', '%']:
            self.next = Token(char, char)
            self.position += 1
            
        elif char == '&':
            if self.position+1 < len(self.source) and self.source[self.position+1] == '&':
                self.next = Token('AND', '&&')
                self.position += 2
            else:
                raise ValueError(f'Caractere inesperado: {char}')
                
        elif char == '|':
            if self.position+1 < len(self.source) and self.source[self.position+1] == '|':
                self.next = Token('OR', '||')
                self.position += 2
            else:
                raise ValueError(f'Caractere inesperado: {char}')
                
        elif char in ['=', '!', '<', '>']:
            if char == '=':
                if self.position + 1 < len(self.source) and self.source[self.position+1] == '=':
                    self.next = Token('EQ', '==')
                    self.position += 2
                else:
                    self.next = Token('=', '=')
                    self.position += 1
            elif char == '!':
                if self.position + 1 < len(self.source) and self.source[self.position+1] == '=':
                    self.next = Token('NEQ', '!=')
                    self.position += 2
                else:
                    self.next = Token('NOT', '!')
                    self.position += 1
            elif char == '<':
                if self.position + 1 < len(self.source) and self.source[self.position+1] == '=':
                    self.next = Token('LE', '<=')
                    self.position += 2
                else:
                    self.next = Token('<', '<')
                    self.position += 1
            elif char == '>':
                if self.position + 1 < len(self.source) and self.source[self.position+1] == '=':
                    self.next = Token('GE', '>=')
                    self.position += 2
                else:
                    self.next = Token('>', '>')
                    self.position += 1
        else:
            raise ValueError(f'Caractere inesperado: {char}')