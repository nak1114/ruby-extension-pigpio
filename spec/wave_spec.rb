RSpec.describe Pigpio::Wave do
  before do
    Pigpio::IF.time_time
  end
  let(:wave) { Pigpio.new.wave }
  it "#new" do
    skip
  end
  it "#clear" do
    skip
  end
  it "#add_new" do
    skip
  end
  it "#add_generic" do
    skip
  end
  it "#add_serial" do
    skip
  end
  it "#create" do
    skip
  end
  it "#delete" do
    skip
  end
  it "#send_once" do
    skip
  end
  it "#send_repeat" do
    skip
  end
  it "#send_using_mode" do
    skip
  end
  it "#chain" do
    write_seq "1"
    wave.chain([255, 0, 1, 255, 1, 3, 0, 2])
    expect(read_args).to eq "pigpio_start : (null),(null)\nwave_chain : 8 : ffffffff 0 1 ffffffff 1 3 0 2\n"
  end
  it "#tx_at" do
    skip
  end
  it "#tx_busy" do
    skip
  end
  it "#tx_stop" do
    skip
  end
  it "#micros" do
    skip
  end
  it "#high_micros" do
    skip
  end
  it "#max_micros" do
    skip
  end
  it "#high_pulses" do
    skip
  end
  it "#max_pulses" do
    skip
  end
  it "#cbs" do
    skip
  end
  it "#high_cbs" do
    skip
  end
  it "#max_cbs" do
    skip
  end
  it "#pulse" do
    skip
  end
end
