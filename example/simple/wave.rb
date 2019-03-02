require "pigpio"
include Pigpio::Constant
pi=Pigpio.new()
unless pi.connect
  exit -1
end

led=pi.gpio(4)
led.mode=PI_OUTPUT
led.pud=PI_PUD_OFF

wave=pi.wave
wave.add_new
wave.add_generic([
  wave.pulse(0x10,0x00,1000000),
  wave.pulse(0x00,0x10,1000000),
])
wid=wave.create
wave.chain([255,0,wid,255,1,3,0])
while wave.tx_busy
  sleep 0.1
end
wave.clear
pi.stop
