require_relative "./gpio"
class Pigpio
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
      return nil unless blk
      q=NativeQueue.make
      th=Thread.start do
        while true
          resource = q.pop
          blk.call(*resource)
        end
      end
      IF.callback(@pi,@gpio,edge,q,th)
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
end