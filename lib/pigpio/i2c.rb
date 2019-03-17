class Pigpio
  class I2C
    attr_reader :pi,:handle
    def initialize(pi,i2c_bus,i2c_addr)
      @pi=pi
      @handle=IF.i2c_open(@pi,i2c_bus,i2c_addr,0)
    end
    def close()
      IF.i2c_close(@pi,@handle)
    end
    def write_quick(bit)
      IF.i2c_write_quick(@pi,@handle,bit)
    end
    def write_byte(bVal)
      IF.i2c_write_byte(@pi,@handle,bVal)
    end
    def read_byte()
      IF.i2c_read_byte(@pi,@handle)
    end
    def write_byte_data(i2c_reg,bVal)
      IF.i2c_write_byte_data(@pi,@handle,i2c_reg,bVal)
    end
    def write_word_data(i2c_reg,wVal)
      IF.i2c_write_word_data(@pi,@handle,i2c_reg,wVal)
    end
    def read_byte_data(i2c_reg)
      IF.i2c_read_byte_data(@pi,@handle,i2c_reg)
    end
    def read_word_data(i2c_reg)
      IF.i2c_read_word_data(@pi,@handle,i2c_reg)
    end
    def process_call(i2c_reg,wVal)
      IF.i2c_process_call(@pi,@handle,i2c_reg,wVal)
    end
    def read_block_data(i2c_reg)
      IF.i2c_read_block_data(@pi,@handle,i2c_reg)
    end
    def read_i2c_block_data(i2c_reg,count)
      IF.i2c_read_i2c_block_data(@pi,@handle,i2c_reg,count)
    end
    def write_block_data(i2c_reg,buf)
      IF.i2c_write_block_data(@pi,@handle,i2c_reg,buf)
    end
    def write_i2c_block_data(i2c_reg,buf)
      IF.i2c_write_i2c_block_data(@pi,@handle,i2c_reg,buf)
    end
    def block_process_call(i2c_reg,buf)
      IF.i2c_block_process_call(@pi,@handle,i2c_reg,buf)
    end
    def read_device(count)
      IF.i2c_read_device(@pi,@handle,count)
    end
    def write_device(buf)
      IF.i2c_write_device(@pi,@handle,buf)
    end
    def zip(inBuf,outLen)
      IF.i2c_zip(@pi,@handle,inBuf,outLen)
    end
  end
end
