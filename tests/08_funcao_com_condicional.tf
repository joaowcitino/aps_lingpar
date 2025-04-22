boot main
  module verificar(a) process
    ping a > 5 process
      log "Maior que 5"
    ping_end
    pong process
      log "Menor ou igual a 5"
    pong_end
  module_end

  verificar(7)
  verificar(2)
shutdown main