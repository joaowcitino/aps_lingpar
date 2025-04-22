boot Calculator

  byte option = 0
  byte num1 = 0
  byte num2 = 0
  byte result = 0
  chip message = "Welcome to TechFlow Calculator!"

  module add(byte a, byte b)
    process
      return a + b
    module_end

  module subtract(byte a, byte b)
    process
      return a - b
    module_end

  module multiply(byte a, byte b)
    process
      return a * b
    module_end

  module divide(byte a, byte b)
    process
      scan_begin
        ping b == 0 process
          log "Error: Division by zero!"
          return 0
        ping_end
        
        pong process
          return a / b
        pong_end
      scan_end
    module_end

  log message

  num1 = 10
  num2 = 5
  option = 3

  scan_begin
    ping option == 1 process
      result = add(num1, num2)
      log "Addition result:"
    ping_end
    
    ping option == 2 process
      result = subtract(num1, num2)
      log "Subtraction result:"
    ping_end
    
    ping option == 3 process
      result = multiply(num1, num2)
      log "Multiplication result:"
    ping_end
    
    ping option == 4 process
      result = divide(num1, num2)
      log "Division result:"
    ping_end
    
    pong process
      log "Invalid option!"
    pong_end
  scan_end

  log result

  log "Multiplication table for result:"
  loop 5 as i process
    byte mult_result = result * (i + 1)
    log mult_result
  loop_end

shutdown Calculator