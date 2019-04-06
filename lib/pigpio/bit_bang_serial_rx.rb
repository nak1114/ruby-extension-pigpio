class Pigpio
  # Set a UserGPIO pin as a serial Rx. 
  class BitBangSerialRx

    # UserGPIO#.
    #
    # This attribute is set at the time of ::new.
    attr_reader :rx

    # Set 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_WAVE_BAUD, PI_GPIO_IN_USE, PI_NOT_SERIAL_GPIO or PI_BAD_SER_INVERT.
    #
    # This attribute is set at the time of ::new.
    attr_reader :valid

    # Set a UserGPIO pin as a serial Rx. 
    #
    # The serial data recieves in a internal cyclic buffer and is read using #read method.
    # It is the caller's responsibility to read data from the cyclic buffer in a timely fashion.
    #
    # [Parameter]
    #  [rx [ Pigpio::UserGPIO ]] Set This pgio as a serial Rx.
    #  [baud [ Integer ]] 50-250000 [bps]. default 9600.
    #  [data_bits [ Integer ]] 1-32 [bit/word]. default 8.
    #
    # See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_serial_read_open]
    def initialize(rx,baud=9600,data_bits=8)
      @rx=rx
      @valid=IF.bb_serial_read_open(@rx.pi,@rx.gpio,baud,data_bits)
      if @valid==0
        @valid=IF.bb_serial_invert(@rx.pi,@rx.gpio,0)
      end
    end

    # This method closes a GPIO for bit bang reading of serial data.
    # 
    # [Return] [Integer] Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_NOT_SERIAL_GPIO.
    # 
    # See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_serial_read_close]
    def close
      IF.bb_serial_read_close(@rx.pi,@rx.gpio)
    end

    # This method copies up to bufSize bytes of data read from the bit bang serial cyclic buffer to the buffer starting at returned value.
    # 
    # [Parameter] [bufSize [ Integer ]] How many bytes receive.
    # [Return] [ Pigpio::Buffer ] A copied buffer and status.
    # 
    # The details of the returned status is the number of bytes copied if OK, otherwise PI_BAD_USER_GPIO or PI_NOT_SERIAL_GPIO.
    # 
    # The bytes returned for each character depend upon the number of data bits +data_bits+ specified in the ::new method.
    # 
    # * For +data_bits+ 1-8 there will be one byte per character. 
    # * For +data_bits+ 9-16 there will be two bytes per character. 
    # * For +data_bits+ 17-32 there will be four bytes per character.
    # 
    # See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_serial_read]
    def read(bufsize=1)
      IF.bb_serial_read(@rx.pi,@rx.gpio,bufsize)
    end

    # This method inverts serial logic for big bang serial reads.
    # 
    # [Parameter] [Integer] invert 0-1.1 invert, 0 normal.
    # [Return] [Integer] Returns 0 if OK, otherwise PI_NOT_SERIAL_GPIO or PI_BAD_SER_INVERT.
    # 
    # See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_serial_invert]
    def invert(invert=1)
      IF.bb_serial_invert(@rx.pi,@rx.gpio,invert)
    end
  end
end
