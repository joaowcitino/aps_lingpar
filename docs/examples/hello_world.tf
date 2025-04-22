boot HelloWorld

  chip message = "Hello, TechFlow World!"
  
  log message
  
  module add(byte a, byte b) process
    return a + b
  module_end
  
  byte result = add(5, 3)
  log result
  
  scan_begin
    ping result > 7 process
      log "Result is greater than 7"
    ping_end
    
    pong process
      log "Result is less than or equal to 7"
    pong_end
  scan_end
  
  byte counter = 0
  stream counter < 5 process
    log counter
    counter = counter + 1
  stream_end
  
  loop 5 as i process
    log i
  loop_end

shutdown HelloWorld