class Pigpio
  class GPIO
    attr_reader :pi,:gpio
    def initialize(pi,gpio)
      @pi=pi #0-15
      @gpio=gpio #0-53
    end
    def mode=(mode)
      ret=IF.set_mode(@pi,@gpio,mode)
    end
    def mode
      ret=IF.get_mode(@pi,@gpio)
    end
    def pud=(pud)
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
end
