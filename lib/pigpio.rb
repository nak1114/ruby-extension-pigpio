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
require "pigpio/spi"

class Pigpio
  attr_reader :pi
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
  def serial(rx,tx=nil,baud=9600,data_bits=8,stop_bits=1,parity_type=:none)
    return BitBangSerialRx.new(rx,baud,data_bits) if tx==nil
    return BitBangSerialTx.new(tx,baud,data_bits,stop_bits) if rx==nil
    return BitBangSerial.new(rx,tx,baud,data_bits,stop_bits)
  end
  def spi(spi_channel=0,enable_cex=1,baud=500000,
    bits_per_word: 8,first_MISO: false,first_MOSI: false,idol_bytes: 0,is_3wire: false,active_low_cex: 0,spi_mode: 0)
    SPI.new(@pi,spi_channel,enable_cex,baud,bits_per_word: bits_per_word,first_MISO:first_MISO,first_MOSI:first_MOSI,idol_bytes:idol_bytes,is_3wire:is_3wire,active_low_cex:active_low_cex,spi_mode:spi_mode)
  end
end
