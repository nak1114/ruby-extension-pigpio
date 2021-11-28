require "pigpio"
include Pigpio::Constant

pi = Pigpio.new
unless pi.connect
  exit(-1)
end

led = pi.gpio(4)
led.mode = PI_OUTPUT
led.pud = PI_PUD_OFF
3.times do |i|
  led.write 1
  sleep 1
  led.write 0
  sleep 1
end
pi.stop
