import sys
from techflow.prepro import PrePro
from techflow.parser import Parser
from techflow.symboltable import SymbolTable

def main():
    if len(sys.argv) != 2:
        print('Uso: python main.py <arquivo.tf>')
        sys.exit(1)
    filepath = sys.argv[1]
    try:
        with open(filepath, 'r') as file:
            code = file.read()
    except FileNotFoundError:
        print(f"Arquivo {filepath} não encontrado")
        sys.exit(1)
    
    code = PrePro.filter(code)
    try:
        ast = Parser.run(code)
        symbol_table = SymbolTable()
        ast.Evaluate(symbol_table)
    except ValueError as e:
        print(f"Erro: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()