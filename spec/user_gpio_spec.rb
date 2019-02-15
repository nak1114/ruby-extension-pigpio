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
    skip
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
