class Pigpio
  class SPI
    attr_reader :pi,:handle
    def initialize(pi,spi_channel=0,enable_cex=1,baud=500000,
      bits_per_word: 8,first_MISO: false,first_MOSI: false,idol_bytes: 0,is_3wire: false,active_low_cex: 0,spi_mode: 0)
      is_aux=(spi_channel!=0)
      flg=(spi_mode.to_i)&0x03
        | ((active_low_cex.to_i)&0x07 << 2)
        | ((enable_cex.to_i    )&0x07 << 5)
        | ((is_aux   ? 1 : 0) << 8)
        | ((is_3wire ? 1 : 0) << 9)
        | ((idol_bytes.to_i    )&0x0f << 10)
        | ((first_MOSI ? 1 : 0) << 14)
        | ((first_MISO ? 1 : 0) << 15)
        | ((bits_per_word.to_i    )&0x3f << 16)
      @pi=pi
      @byte_per_word=(bits_per_word/8.0).ceil
      @handle=IF.spi_open(@pi,spi_channel,baud,flg)
    end
    def close
      IF.spi_close(@pi,@handle)
    end
    def read(words=1)
      IF.spi_read(@pi,@handle,words*@byte_per_word)
    end
    def write(buf)
      IF.spi_write(@pi,@handle,buf)
    end
    def xfer(buf)
      IF.spi_xfer(@pi,@handle,buf)
    end
  end
end
