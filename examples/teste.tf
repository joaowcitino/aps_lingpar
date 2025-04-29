boot
    byte x: i32 = 10;
    byte y: i32 = 5;
    byte result: i32 = x + y * 2;
    log(result);
    
    ping(result > 15) then
        log("Resultado maior que 15");
    pong then
        log("Resultado menor ou igual a 15");
    end
    
    byte contador: i32 = 0;
    byte soma: i32 = 0;
    
    stream(contador < 5) then
        contador = contador + 1;
        soma = soma + contador;
        log("Iteração: " ++ contador);
    end
    
    log("Soma final: " ++ soma);
    
    byte opcao: i32 = 2;
    
    select(opcao) then
        when 1 then
            log("Opção 1 selecionada");
        end
        when 2 then
            log("Opção 2 selecionada");
        end
        when 3 then
            log("Opção 3 selecionada");
        end
        otherwise then
            log("Opção inválida");
        end
    end
    
    byte i: i32 = 0;
    repeat then
        i = i + 1;
        log("Repeat: " ++ i);
    until i >= 3;
shutdown