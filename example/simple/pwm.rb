require "pigpio"
include Pigpio::Constant
pi=Pigpio.new()
unless pi.connect
  exit -1
end
pin=pi.gpio(4)
pin.mode=PI_OUTPUT
pin.pud=PI_PUD_OFF
pwm=pin.pwm

button=pi.gpio(17)
button.mode=PI_INPUT
button.pud=PI_PUD_UP

i=128
while true do
  i=(i+1)%256   if button.read == 0
  pwm.dutycycle= i
  sleep 0.01
end
pi.stop
