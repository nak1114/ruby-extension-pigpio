class Pigpio
  class Wave
    def initialize(pi)
      @pi = pi
      IF.wave_clear(pi)
    end

    def clear
      IF.wave_clear(@pi)
    end

    def add_new
      IF.wave_add_new(@pi)
    end

    def add_generic(pulses)
      IF.wave_add_generic(@pi, pulses)
    end

    def add_serial(user_gpio, baud, data_bits, stop_bits, offset, str)
      IF.wave_add_serial(@pi, user_gpio, baud, data_bits, stop_bits, offset, str)
    end

    def create
      IF.wave_create(@pi)
    end

    def delete(id)
      IF.wave_delete(@pi, id)
    end

    def send_once(id)
      IF.wave_delete(@pi, id)
    end

    def send_repeat(id)
      IF.wave_delete(@pi, id)
    end

    def send_using_mode(id, mode)
      IF.wave_delete(@pi, id, mode)
    end

    def chain(buf)
      IF.wave_chain(@pi, buf)
    end

    def tx_at
      IF.wave_tx_at(@pi)
    end

    def tx_busy
      IF.wave_tx_busy(@pi)
    end

    def tx_stop
      IF.wave_tx_stop(@pi)
    end

    def micros
      IF.wave_get_micros(@pi)
    end

    def high_micros
      IF.wave_get_high_micros(@pi)
    end

    def max_micros
      IF.wave_get_max_micros(@pi)
    end

    def pulses
      IF.wave_get_pulses(@pi)
    end

    def high_pulses
      IF.wave_get_high_pulses(@pi)
    end

    def max_pulses
      IF.wave_get_max_pulses(@pi)
    end

    def cbs
      IF.wave_get_cbs(@pi)
    end

    def high_cbs
      IF.wave_get_high_cbs(@pi)
    end

    def max_cbs
      IF.wave_get_max_cbs(@pi)
    end

    def pulse(on, off, us)
      Pulse.make(on, off, us)
    end
  end
end
