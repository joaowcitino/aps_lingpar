from palco.parser import parser
from palco.interpreter import execute
import sys

def main():
    if len(sys.argv) < 2:
        print("Uso: python main.py <arquivo.palco>")
        return
    with open(sys.argv[1], "r", encoding="utf-8") as f:
        source_code = f.read()
    ast = parser.parse(source_code)
    if ast:
        execute(ast)

if __name__ == "__main__":
    main()
