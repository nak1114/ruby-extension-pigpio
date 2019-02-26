RSpec.describe Pigpio::UserGPIO do
  before do
    Pigpio::IF.time_time
  end
  let(:pin){Pigpio.new.gpio(0)}
  it "#watchdog" do
    skip
  end
  it "#glitch_filter" do
    skip
  end
  it "#noise_filter" do
    skip
  end
  it "#callback" do
    write_seq "1"
    cb=nil
    begin
      ary=[]
      cb=pin.callback(Pigpio::Constant::RISING_EDGE){|tick,level| ary << [tick,level]}
      expect(cb.id).to be 123
      sleep 3
    rescue e
      put e.message
    ensure
      cb.cancel
    end
    expect(ary).to eq [[88, 99], [87, 98]]
    expect(cb.id).to be -1
  end
  it "#wait_for_edge" do
    skip
  end
  it "#trigger" do
    skip
  end
  it "#pwm" do
    skip
  end
end
