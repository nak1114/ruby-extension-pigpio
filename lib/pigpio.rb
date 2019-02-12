require "pigpio/version"
require "pigpio/constant"
require "pigpio/pigpio"

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
  class Bank
    Set={
      read:  [:read_bank_1 ,:read_bank_2],
      clear: [:clear_bank_1,:clear_bank_2],
      set:   [:set_bank_1  ,:set_bank_2],
    }
    def initialize(pi,num)
      @pi=pi
      @num=num
    end
    def read
      IF.send(Set[:read][@num],@pi)
    end
    def clear(bits)
      IF.send(Set[:clear][@num],@pi,bits)
    end
    def set(bits)
      IF.send(Set[:set][@num],@pi,bits)
    end
  end
  def bank(num)
    Bank.new(@pi,num)
  end


  def gpio(gpio)
    if gpio<32
      UserGPIO.new(@pi,gpio)
    else
      GPIO.new(@pi,gpio)
    end
  end
  class GPIO
    def initialize(pi,gpio)
      @pi=pi #0-15
      @gpio=gpio #0-53
    end
    def gpio
      @gpio
    end
    def mode=(mode)
      ret=IF.set_mode(@pi,@gpio,mode)
    end
    def mode
      ret=IF.get_mode(@pi,@gpio)
    end
    def pud(pud)
      ret=IF.set_pull_up_down(@pi,@gpio,pud)
    end
    def read
      ret=IF.gpio_read(@pi,@gpio)
    end
    def write(level)
      ret=IF.gpio_write(@pi,@gpio,level)
    end
    def hardware_clock(clkfreq)
      ret=IF.hardware_clock(@pi,@gpio,clkfreq)
    end
    def hardware_PWM(vPWMfreq,vPWMduty)
      ret=IF.hardware_PWM(@pi,@gpio,vPWMfreq,vPWMduty)
    end
  end
  class UserGPIO < GPIO
    def watchdog(timeout)
      ret=IF.set_watchdog(@pi,@gpio,timeout)
    end
    def glitch_filter(steady)
      ret=IF.set_glitch_filter(@pi,@gpio,steady)
    end
    def noise_filter(steady,active)
      ret=IF.set_noise_filter(@pi,@gpio,steady,active)
    end
    def callback(edge,&blk)
      @cid=IF.callback(@pi,@gpio,edge,&blk)
    end
    def cancel_callback
      ret=IF.callback_cancel(@cid) if @cid && @cid >=0
      @cid=nil
      ret
    end
    def wait_for_edge(edge,timeout)
      ret=IF.wait_for_edge(@pi,@gpio,edge,timeout)
    end
    def trigger(pulseLen,level)
      ret=IF.gpio_trigger(@pi,@gpio,pulseLen,level)
    end
    def pwm
      PWM.new(@pi,@gpio)
    end
  end
  class PWM
    def initialize(pi,gpio)
      @pi=pi
      @gpio=gpio
    end
    def dutycycle=(dutycycle)
      ret=IF.set_PWM_dutycycle(@pi,@gpio,dutycycle)
    end
    def dutycycle
      ret=IF.get_PWM_dutycycle(@pi,@gpio)
    end
    def range=(range)
      ret=IF.set_PWM_range(@pi,@gpio,range)
    end
    def range
      ret=IF.get_PWM_range(@pi,@gpio)
    end
    def real_range
      ret=IF.get_PWM_real_range(@pi,@gpio)
    end
    def frequency=(frequency)
      ret=IF.set_PWM_frequency(@pi,@gpio,frequency)
    end
    def frequency
      ret=IF.get_PWM_frequency(@pi,@gpio)
    end
    def servo_pulsewidth=(pulsewidth)
      ret=IF.set_servo_pulsewidth(@pi,@gpio,pulsewidth)
    end
    def servo_pulsewidth
      ret=IF.get_servo_pulsewidth(@pi,@gpio)
    end
  end
  
end
