class Pigpio
  class Buffer < String
    attr_reader :ret_code
    def initialize(ret_code,str=nil)
      super(str)
      @ret_code=ret_code
    end
    def valid?
      @ret_code>=0
    end
  end
end
