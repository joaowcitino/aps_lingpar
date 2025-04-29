boot
    // Demonstração da linguagem TechFlow
    log("Bem-vindo à linguagem TechFlow!");
    
    // Variáveis e tipos
    byte contador: i32 = 0;
    byte pi: i32 = 314;
    byte nome: str = "TechFlow";
    byte status: bool = true;
    
    // Operações aritméticas
    byte resultado: i32 = (pi / 100) + 10 * 2;
    log("Resultado: " ++ resultado);
    
    // Estruturas condicionais (ping/pong)
    ping(resultado > 25) then
        log("Resultado é maior que 25");
    pong then
        log("Resultado é menor ou igual a 25");
    end
    
    // Loops (stream)
    log("Iniciando contagem:");
    stream(contador < 5) then
        contador = contador + 1;
        log("  Contador: " ++ contador);
    end
    
    // Estrutura de controle repeat/until
    byte i: i32 = 0;
    log("Demonstração repeat/until:");
    repeat then
        i = i + 1;
        log("  Valor de i: " ++ i);
    until i >= 3;
    
    // Estrutura de seleção select/when
    byte opcao: i32 = 2;
    log("Demonstração select/when:");
    select(opcao) then
        when 1 then
            log("  Opção 1 selecionada");
        end
        when 2 then
            log("  Opção 2 selecionada");
        end
        when 3 then
            log("  Opção 3 selecionada");
        end
        otherwise then
            log("  Opção inválida");
        end
    end
    
    // Concatenação de strings
    byte mensagem: str = "TechFlow" ++ " v1.0";
    log("Mensagem final: " ++ mensagem);
    
    log("Programa TechFlow finalizado com sucesso!");
shutdown