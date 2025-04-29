class SymbolTable:
    def __init__(self):
        self.table = {}
        
    def get(self, identifier):
        if identifier in self.table:
            return self.table[identifier]
        else:
            raise ValueError(f"Variável não definida: {identifier}")
            
    def set(self, identifier, value, typ=None):
        if identifier in self.table:
            current_value, current_type = self.table[identifier]
            if typ is not None and current_type != typ:
                raise ValueError(f"Atribuição com tipos incompatíveis na variável '{identifier}'")
            self.table[identifier] = (value, current_type)
        else:
            if typ is None:
                self.table[identifier] = (value, None)
            else:
                self.table[identifier] = (value, typ)