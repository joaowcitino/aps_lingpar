boot main
  module contar_ate(n) process
    loop n as i process
      log i
    loop_end
  module_end

  contar_ate(3)
shutdown main