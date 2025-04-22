TEST_DIR="test"
BUILD_DIR="build"
EXECUTABLE="$BUILD_DIR/techflow"

if [ ! -f "$EXECUTABLE" ]; then
    echo "Erro: Compilador não encontrado. Execute 'make' primeiro."
    exit 1
fi

echo "Executando testes léxicos..."
for test_file in $TEST_DIR/lexer_tests/*.tf; do
    if [ -f "$test_file" ]; then
        echo "  Testando $test_file"
        $EXECUTABLE --lexer $test_file
    fi
done

echo "Executando testes sintáticos..."
for test_file in $TEST_DIR/parser_tests/*.tf; do
    if [ -f "$test_file" ]; then
        echo "  Testando $test_file"
        $EXECUTABLE --parser $test_file
    fi
done

echo "Executando testes semânticos..."
for test_file in $TEST_DIR/semantic_tests/*.tf; do
    if [ -f "$test_file" ]; then
        echo "  Testando $test_file"
        $EXECUTABLE --semantic $test_file
    fi
done

echo "Executando testes de geração de código..."
for test_file in $TEST_DIR/codegen_tests/*.tf; do
    if [ -f "$test_file" ]; then
        echo "  Testando $test_file"
        $EXECUTABLE $test_file

        output_file="${test_file%.tf}.bc"
        if [ -f "$output_file" ]; then
            echo "    Arquivo LLVM gerado: $output_file"
        else
            echo "    Erro: Arquivo LLVM não gerado para $test_file"
        fi
    fi
done

echo "Executando testes de integração..."
for test_file in $TEST_DIR/integration_tests/*.tf; do
    if [ -f "$test_file" ]; then
        echo "  Testando $test_file"
        $EXECUTABLE $test_file

        output_file="${test_file%.tf}.bc"
        if [ -f "$output_file" ]; then
            echo "    Executando programa gerado..."
            llc -filetype=obj $output_file -o "${output_file%.bc}.o"
            gcc "${output_file%.bc}.o" -o "${output_file%.bc}"
            "${output_file%.bc}"
        else
            echo "    Erro: Arquivo LLVM não gerado para $test_file"
        fi
    fi
done

echo "Testes concluídos!"