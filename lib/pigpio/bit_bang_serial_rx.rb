class Pigpio
  class BitBangSerialRx
    attr_reader :pi,:gpio
    def initialize(rx,baud=9600,data_bits=8)
      @rx=rx
      IF.bb_serial_read_open(@rx.pi,@rx.gpio,baud,data_bits)
      IF.bb_serial_invert(@rx.pi,@rx.gpio,0)
    end
    def close
      IF.bb_serial_read_close(@rx.pi,@rx.gpio)
    end
    def read(bufsize=1)
      IF.bb_serial_read(@rx.pi,@rx.gpio,bufsize)
    end
    def invert(invert=1)
      IF.bb_serial_invert(@rx.pi,@rx.gpio,invert)
    end
  end
end
