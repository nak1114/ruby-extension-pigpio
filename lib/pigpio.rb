require "pigpio/version"
require "pigpio/constant"
require "pigpio/pigpio"
require "pigpio/gpio"
require "pigpio/user_gpio"
require "pigpio/pwm"
require "pigpio/bank"
require "pigpio/wave"
require "pigpio/bit_bang_serial_rx"
require "pigpio/bit_bang_serial_tx"
require "pigpio/bit_bang_serial"
require "pigpio/serial"

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
  def serial(rx,tx=nil,baud=19200,data_bits=8,stop_bits=1,parity_type=:none)
    return BitBangSerialRx(@pi,rx,baud,data_bits) if tx==nil
    return BitBangSerialTx(@pi,tx,baud,data_bits,stop_bits) if rx==nil
    return BitBangSerial(@pi,rx,tx,baud,data_bits,stop_bits)
  end
  
end
