class Pigpio
  class BitBangSerialTx
    attr_reader :pi,:tx
    def initialize(pi,tx,baud,data_bits,stop_half_bits)
      @pi=pi
      @tx=tx
      @baud=baud
      @data_bits=data_bits
      @stop_half_bits=stop_half_bits
    end
    def close
      IF.wave_tx_stop(@pi) if busy?
      IF.wave_clear(@pi)
    end
    def write(buf)
      IF.wave_clear(@pi)
      IF.wave_add_new(@pi)
      ret=IF.wave_add_serial(@pi,@tx,@baud,@data_bits,@stop_half_bits,0,buf)
      return ret if ret < 0
      wid=IF.wave_create(@pi)
      return wid if wid < 0
      IF.wave_send_once(@pi,wid)
    end
    def sync
      while busy?
        sleep 0.1
      end
    end
    def write_sync(buf)
      ret=write(buf)
      return ret if ret < 0
      sync
      IF.wave_clear(@pi)
    end
    def busy?
      IF.wave_tx_busy(@pi)
    end
    def stop
      IF.wave_tx_stop(@pi)
    end
  end
end
__END__
int wave_tx_busy(int pi){return 123;}
int wave_tx_stop(int pi){return 123;}
int wave_clear(int pi){return 123;}
int wave_add_new(int pi){return 123;}
int wave_add_serial(int pi, unsigned user_gpio, unsigned baud, unsigned data_bits, unsigned stop_bits, unsigned offset, unsigned numBytes, char *str){return 123;}
int wave_create(int pi){return 123;}
int wave_delete(int pi, unsigned wave_id){return 123;}
int wave_send_once(int pi, unsigned wave_id){return 123;}
