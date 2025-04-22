boot main
  loop 3 as i process
    ping i == 1 process
      log "Meio"
    ping_end
    pong process
      log i
    pong_end
  loop_end
shutdown main