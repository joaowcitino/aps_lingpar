boot
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
    
    byte dia: i32 = 3;
    
    select(dia) then
        when 1 then
            log("Domingo");
        end
        when 2 then
            log("Segunda-feira");
        end
        when 3 then
            log("Terça-feira");
        end
        otherwise then
            log("Outro dia");
        end
    end
shutdown