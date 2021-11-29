require "pigpio"

include Pigpio::Constant
pi = Pigpio.new
unless pi.connect
  exit(-1)
end
counter = 0
tx = pi.gpio(27)
tx.mode = PI_OUTPUT
tx.pud = PI_PUD_OFF

rx = pi.gpio(22)
rx.mode = PI_INPUT
rx.pud = PI_PUD_OFF

uart = pi.serial(rx, tx)
uart.write_sync "abc"
uart.write_sync "def"
p uart.read(10)
uart.write_sync "zyx"
4.times { p uart.read(1) }
uart.close
pi.stop
