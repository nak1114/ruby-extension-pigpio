class Pigpio
  class Serial
    attr_reader :pi,:handle
    TTY=%w(/dev/tty /dev/serial)
    Baud=[50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400]
    def initialize(pi,ser_tty,baud)
      @pi=pi
      @handle=IF.serial_open(pi,ser_tty,baud=19200,0)
    end
    def close
      IF.serial_close(@pi,@handle)
    end
    def byte=(bVal)
      IF.serial_write_byte(@pi,@handle,bVal)
    end
    def byte
      IF.serial_read_byte(@pi,@handle)
    end
    def write(buf)
      IF.serial_write(@pi,@handle,buf)
    end
    def read(count)
      IF.serial_read(@pi,@handle,count)
    end
    def size
      IF.serial_data_available(@pi,@handle)
    end
  end
end
