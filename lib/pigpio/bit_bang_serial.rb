require_relative "./bit_bang_serial_rx"
require_relative "./bit_bang_serial_tx"

class Pigpio
  class BitBangSerial < BitBangSerialTx
    attr_reader :rx
    def initialize(rx, tx, baud = 9600, data_bits = 8, stop_bits = 1, parity_type = :none)
      super(tx, baud, data_bits, stop_bits)
      @rx = BitBangSerialRx.new(rx, baud, data_bits)
    end

    def close
      super
      @rx.close
    end

    def read(bufsize = 1)
      @rx.read(bufsize)
    end
  end
end
