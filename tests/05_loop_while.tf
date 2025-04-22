boot main
  byte x = 0
  stream x < 3 process
    log x
    byte x = x + 1
  stream_end
shutdown main