class Pigpio
  class BitBangSerialRx
    attr_reader :pi,:gpio
    def initialize(pi,gpio,baud,data_bits)
      @pi=pi
      @gpio=gpio
      If.bb_serial_read_open(pi,gpio,baud,data_bits)
      IF.bb_serial_invert(@pi,@gpio,0)
    end
    def close
      IF.bb_serial_read_close(@pi,@gpio)
    end
    def read
      IF.bb_serial_read(@pi,@gpio)
    end
    def invert(invert=1)
      IF.bb_serial_invert(@pi,@gpio,invert)
    end
  end
end
__END__
int bb_serial_read_open(int pi, unsigned user_gpio, unsigned baud, unsigned data_bits){return 123;}
int bb_serial_read(int pi, unsigned user_gpio, void *buf, size_t bufSize){return 123;}
int bb_serial_read_close(int pi, unsigned user_gpio){return 123;}
int bb_serial_invert(int pi, unsigned user_gpio, unsigned invert){return 123;}
