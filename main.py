import sys
from parser import parser
from llvm_compiler import compilar_programa, salvar_llvm_em_arquivo

if len(sys.argv) != 2:
    print("Uso: python3 main.py caminho/para/arquivo.tf")
    sys.exit(1)

caminho_arquivo = sys.argv[1]

try:
    with open(caminho_arquivo, "r", encoding="utf-8") as f:
        codigo = f.read()
except FileNotFoundError:
    print(f"❌ Arquivo não encontrado: {caminho_arquivo}")
    sys.exit(1)

print(f"🔍 Compilando {caminho_arquivo} para LLVM IR...")

ast = parser.parse(codigo)
if ast is None:
    print("❌ Falha na análise sintática.")
    sys.exit(1)

compilar_programa(ast)
salvar_llvm_em_arquivo("out.ll")

print("✅ Use `lli out.ll` ou `clang out.ll -o out && ./out` para executar.")