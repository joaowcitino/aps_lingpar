boot main
  byte idade = 18
  ping idade == 18 process
    log "Maioridade"
  ping_end
  pong process
    log "Menor"
  pong_end
shutdown main