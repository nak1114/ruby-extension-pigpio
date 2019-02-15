require "pigpio"
include Pigpio::Constant
pi=Pigpio.new()
unless pi.connect
  exit -1
end
counter=0
led=pi.gpio(4)
led.mode=PI_OUTPUT
led.pud=PI_PUD_OFF

button=pi.gpio(17)
button.mode=PI_INPUT
button.pud=PI_PUD_UP
button.glitch_filter(30)
button.callback(EITHER_EDGE){|tick,level|
  led.write level
  counter+=1
}

while(counter<10)do
end
led.write 0