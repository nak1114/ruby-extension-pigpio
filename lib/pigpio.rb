require "pigpio/version"
require "pigpio/constant"
require "pigpio/pigpio"
require "pigpio/gpio"
require "pigpio/user_gpio"
require "pigpio/pwm"
require "pigpio/bank"
require "pigpio/wave"

class Pigpio
  def initialize(addr=nil,port=nil,&blk)
    @pi=IF.pigpio_start(addr,port)
    if blk && connect
      blk.call(self) 
      IF.pigpio_stop(@pi)
    end
  end
  def connect
    @pi>=0
  end
  def pi
    @pi
  end
  def stop
    IF.pigpio_stop(@pi)
    @pi=-1
  end
  def current_tick
    IF.get_current_tick(@pi)
  end
  def hardware_revision
    IF.get_hardware_revision(@pi)
  end
  def pigpio_version
    IF.get_pigpio_version(@pi)
  end
  
  def bank(num)
    Bank.new(@pi,num)
  end
  def gpio(gpio)
    (gpio<32 ? UserGPIO : GPIO).new(@pi,gpio)
  end
  def wave()
    Wave.new(@pi)
  end
  
end
