boot
    byte n: i32 = 1;
    byte factorial: i32 = 1;
    
    repeat then
        factorial = factorial * n;
        log("Fatorial de " ++ n ++ " é " ++ factorial);
        n = n + 1;
    until n > 5;
    
    byte x: i32 = 10;
    
    ping(x % 2 == 0) then
        log("X é par");
    pong then
        log("X é ímpar");
    end
shutdown