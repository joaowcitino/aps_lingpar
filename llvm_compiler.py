from llvmlite import ir
import platform
from parser import (
    Programa, DeclaracaoVariavel, ComandoLog, Condicional, BlocoIf, BlocoElse,
    LoopPara, LoopEnquanto, FuncaoDefinida, ChamadaFuncao, Retorno
)

module = ir.Module(name="techflow")
sistema = platform.system().lower()
if sistema == "linux":
    module.triple = "x86_64-pc-linux-gnu"
elif sistema == "darwin":
    module.triple = "x86_64-apple-darwin"
elif sistema == "windows":
    module.triple = "x86_64-pc-windows-msvc"
else:
    module.triple = "unknown-unknown-unknown"

int_type = ir.IntType(32)
void_type = ir.VoidType()
printf_ty = ir.FunctionType(ir.IntType(32), [ir.PointerType(ir.IntType(8))], var_arg=True)
printf = ir.Function(module, printf_ty, name="printf")

# String de formatação para printf
format_str = "%d\n\0"
fmt_type = ir.ArrayType(ir.IntType(8), len(format_str))
global_fmt = ir.GlobalVariable(module, fmt_type, name="fmt")
global_fmt.linkage = 'internal'
global_fmt.global_constant = True
global_fmt.initializer = ir.Constant(fmt_type, bytearray(format_str.encode("utf8")))

# Tabelas de contexto
variables = {}
functions = {}

def compilar_programa(programa: Programa):
    main_ty = ir.FunctionType(void_type, [])
    main_fn = ir.Function(module, main_ty, name="main")
    bloco = main_fn.append_basic_block(name="entry")
    builder = ir.IRBuilder(bloco)

    compilar_bloco(programa.corpo, builder)
    builder.ret_void()

def compilar_bloco(lista, builder):
    for instr in lista:
        resultado = compilar_instr(instr, builder)
        if resultado == 'retorno':
            break

def compilar_instr(instr, builder):
    if isinstance(instr, DeclaracaoVariavel):
        valor = compilar_expr(instr.valor, builder)
        ptr = builder.alloca(int_type, name=instr.nome)
        builder.store(valor, ptr)
        variables[instr.nome] = ptr

    elif isinstance(instr, ComandoLog):
        valor = compilar_expr(instr.valor, builder)
        fmt_ptr = builder.bitcast(global_fmt, ir.PointerType(ir.IntType(8)))
        builder.call(printf, [fmt_ptr, valor])

    elif isinstance(instr, Condicional):
        cond = compilar_expr(instr.if_block.condicao, builder)
        cond_val = builder.icmp_signed('!=', cond, ir.Constant(int_type, 0))

        with builder.if_else(cond_val) as (entao, senao):
            with entao:
                compilar_bloco(instr.if_block.corpo, builder)
            with senao:
                if instr.else_block:
                    compilar_bloco(instr.else_block.corpo, builder)

    elif isinstance(instr, LoopPara):
        inicio = compilar_expr(instr.inicio, builder)
        loop_var = builder.alloca(int_type, name=instr.var)
        builder.store(ir.Constant(int_type, 0), loop_var)

        loop_bb = builder.append_basic_block("loop")
        corpo_bb = builder.append_basic_block("loop_body")
        fim_bb = builder.append_basic_block("loop_end")

        builder.branch(loop_bb)
        builder.position_at_end(loop_bb)

        i_val = builder.load(loop_var)
        cond = builder.icmp_signed("<", i_val, inicio)
        builder.cbranch(cond, corpo_bb, fim_bb)

        builder.position_at_end(corpo_bb)
        variables[instr.var] = loop_var
        compilar_bloco(instr.corpo, builder)

        i_next = builder.add(i_val, ir.Constant(int_type, 1))
        builder.store(i_next, loop_var)
        builder.branch(loop_bb)

        builder.position_at_end(fim_bb)

    elif isinstance(instr, LoopEnquanto):
        cond_bb = builder.append_basic_block("while_cond")
        corpo_bb = builder.append_basic_block("while_body")
        fim_bb = builder.append_basic_block("while_end")

        builder.branch(cond_bb)
        builder.position_at_end(cond_bb)

        cond = compilar_expr(instr.condicao, builder)
        cond_val = builder.icmp_signed('!=', cond, ir.Constant(int_type, 0))
        builder.cbranch(cond_val, corpo_bb, fim_bb)

        builder.position_at_end(corpo_bb)
        compilar_bloco(instr.corpo, builder)
        builder.branch(cond_bb)

        builder.position_at_end(fim_bb)

    elif isinstance(instr, FuncaoDefinida):
        fn_ty = ir.FunctionType(int_type, [int_type] * len(instr.parametros))
        fn = ir.Function(module, fn_ty, name=instr.nome)
        bb = fn.append_basic_block(name="entry")
        fn_builder = ir.IRBuilder(bb)

        old_vars = variables.copy()
        for nome, arg in zip(instr.parametros, fn.args):
            ptr = fn_builder.alloca(int_type, name=nome)
            fn_builder.store(arg, ptr)
            variables[nome] = ptr

        retorno = None
        for i in instr.corpo:
            resultado = compilar_instr(i, fn_builder)
            if isinstance(resultado, ir.instructions.Ret):
                retorno = resultado
                break

        if instr.retorno:
            ret_val = compilar_expr(instr.retorno.valor, fn_builder)
            fn_builder.ret(ret_val)
        elif not retorno:
            fn_builder.ret(ir.Constant(int_type, 0))

        variables.clear()
        variables.update(old_vars)
        functions[instr.nome] = fn

    elif isinstance(instr, ChamadaFuncao):
        compilar_expr(instr, builder)

    elif isinstance(instr, Retorno):
        valor = compilar_expr(instr.valor, builder)
        builder.ret(valor)
        return 'retorno'

def compilar_expr(expr, builder):
    if isinstance(expr, int):
        return ir.Constant(int_type, expr)
    elif isinstance(expr, tuple):
        if expr[0] == 'binop':
            op, left, right = expr[1], expr[2], expr[3]
            l = compilar_expr(left, builder)
            r = compilar_expr(right, builder)
            if op == '+': return builder.add(l, r)
            if op == '-': return builder.sub(l, r)
            if op == '*': return builder.mul(l, r)
            if op == '/': return builder.sdiv(l, r)
        elif expr[0] == 'cmp':
            op, left, right = expr[1], expr[2], expr[3]
            l = compilar_expr(left, builder)
            r = compilar_expr(right, builder)
            if op == '==': return builder.icmp_signed('==', l, r)
            if op == '!=': return builder.icmp_signed('!=', l, r)
            if op == '>': return builder.icmp_signed('>', l, r)
            if op == '<': return builder.icmp_signed('<', l, r)
            if op == '>=': return builder.icmp_signed('>=', l, r)
            if op == '<=': return builder.icmp_signed('<=', l, r)
        elif expr[0] == 'var':
            ptr = variables.get(expr[1])
            if not ptr:
                raise Exception(f"Variável não declarada: {expr[1]}")
            return builder.load(ptr)
    elif isinstance(expr, ChamadaFuncao):
        fn = functions.get(expr.nome)
        if not fn:
            raise Exception(f"Função não definida: {expr.nome}")
        args = [compilar_expr(arg, builder) for arg in expr.argumentos]
        return builder.call(fn, args)
    raise Exception(f"Expressão inválida: {expr}")

def salvar_llvm_em_arquivo(nome="out.ll"):
    with open(nome, "w") as f:
        f.write(str(module))
    print(f"✅ LLVM IR salvo em {nome}")