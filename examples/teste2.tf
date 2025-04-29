boot
    byte contador: i32 = 0;
    byte soma: i32 = 0;
    
    stream(contador < 5) then
        contador = contador + 1;
        soma = soma + contador;
        log("Iteração: " ++ contador);
        
        ping(contador % 2 == 0) then
            log("Contador é par");
        end
    end
    
    log("Soma final: " ++ soma);
    
    byte i: i32 = 0;
    stream(i < 3) then
        log("Loop while: " ++ i);
        i = i + 1;
    end
shutdown