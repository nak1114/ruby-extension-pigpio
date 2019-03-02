require_relative "./bit_bang_serial_rx"
require_relative "./bit_bang_serial_tx"

class Pigpio
  class BitBangSerial
    attr_reader :rx,:tx
    def initialize(pi,rx,tx,baud=19200,data_bits=8,stop_bits=1,parity_type=:none)
      @rx=BitBangSerialRx.new(pi,rx,baud,data_bits)
      @tx=BitBangSerialTx.new(pi,tx,baud,data_bits,stop_bits)
    end
    def close
      @tx.close
      @rx.close
    end
    def read
      @rx.read
    end
    def write(buf)
      @tx.write(buf)
    end
  end
end
