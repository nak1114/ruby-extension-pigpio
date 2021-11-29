class Pigpio
  class PWM
    def initialize(pi, gpio)
      @pi = pi
      @gpio = gpio
    end

    def start(dutycycle)
      ret = IF.set_PWM_dutycycle(@pi, @gpio, dutycycle)
    end

    def dutycycle=(dutycycle)
      ret = IF.set_PWM_dutycycle(@pi, @gpio, dutycycle)
    end

    def dutycycle
      ret = IF.get_PWM_dutycycle(@pi, @gpio)
    end

    def range=(range)
      ret = IF.set_PWM_range(@pi, @gpio, range)
    end

    def range
      ret = IF.get_PWM_range(@pi, @gpio)
    end

    def real_range
      ret = IF.get_PWM_real_range(@pi, @gpio)
    end

    def frequency=(frequency)
      ret = IF.set_PWM_frequency(@pi, @gpio, frequency)
    end

    def frequency
      ret = IF.get_PWM_frequency(@pi, @gpio)
    end

    def servo_pulsewidth=(pulsewidth)
      ret = IF.set_servo_pulsewidth(@pi, @gpio, pulsewidth)
    end

    def servo_pulsewidth
      ret = IF.get_servo_pulsewidth(@pi, @gpio)
    end
  end
end
