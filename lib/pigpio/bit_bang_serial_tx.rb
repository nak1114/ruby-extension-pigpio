class Pigpio
  class BitBangSerialTx
    attr_reader :tx
    def initialize(tx, baud = 9600, data_bits = 8, stop_bits = 1)
      @tx = tx
      @baud = baud
      @data_bits = data_bits
      @stop_half_bits = (stop_bits * 2).round.to_i
    end

    def close
      IF.wave_tx_stop(@tx.pi) if busy?
      IF.wave_clear(@tx.pi)
    end

    def write(buf)
      IF.wave_clear(@tx.pi)
      IF.wave_add_new(@tx.pi)
      ret = IF.wave_add_serial(@tx.pi, @tx.gpio, @baud, @data_bits, @stop_half_bits, 0, buf)
      return ret if ret < 0
      wid = IF.wave_create(@tx.pi)
      return wid if wid < 0
      IF.wave_send_once(@tx.pi, wid)
    end

    def sync
      while busy?
        sleep 0.1
      end
    end

    def write_sync(buf)
      ret = write(buf)
      return ret if ret < 0
      sync
      IF.wave_clear(@tx.pi)
    end

    def busy?
      IF.wave_tx_busy(@tx.pi)
    end

    def stop
      IF.wave_tx_stop(@tx.pi)
    end
  end
end
