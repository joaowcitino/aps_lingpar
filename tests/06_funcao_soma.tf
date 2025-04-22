boot main
  module soma(a, b) process
    return a + b
  module_end

  byte resultado = soma(7, 8)
  log resultado
shutdown main