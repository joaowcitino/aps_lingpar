# Casos de Teste

## Teste básico

```techflow
boot main
  byte x = 42
  log x
shutdown main
```

## Teste com função e condição

```techflow
boot main
  module soma(a, b) process
    return a + b
  module_end

  byte r = soma(2, 3)
  log r

  ping r == 5 process
    log "Correto"
  ping_end

  pong process
    log "Errado"
  pong_end
shutdown main
```
