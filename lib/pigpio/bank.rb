class Pigpio
  class Bank
    Set = {
      read: [:read_bank_1, :read_bank_2],
      clear: [:clear_bank_1, :clear_bank_2],
      set: [:set_bank_1, :set_bank_2]
    }
    def initialize(pi, num)
      @pi = pi
      @num = num
    end

    def read
      IF.send(Set[:read][@num], @pi)
    end

    def clear(bits)
      IF.send(Set[:clear][@num], @pi, bits)
    end

    def set(bits)
      IF.send(Set[:set][@num], @pi, bits)
    end
  end
end
