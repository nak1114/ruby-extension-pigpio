require "pigpio"

include Pigpio::Constant
pi=Pigpio.new()
unless pi.connect
  exit -1
end
counter=0
cs=pi.gpio(8)
cs.mode=PI_OUTPUT
cs.pud=PI_PUD_OFF
mosi=pi.gpio(10)
mosi.mode=PI_OUTPUT
mosi.pud=PI_PUD_OFF
clk=pi.gpio(11)
clk.mode=PI_OUTPUT
clk.pud=PI_PUD_OFF
miso=pi.gpio(9)
miso.mode=PI_INPUT
miso.pud=PI_PUD_OFF
spi=pi.spi()
p spi.xfer "abcdef"
spi.close
pi.stop
