RSpec.describe Pigpio do
  before do
    Pigpio::IF.time_time
  end

  it "has a version number" do
    read_args
    expect(Pigpio::VERSION).not_to be nil
  end
  let(:pi){Pigpio.new}


  it "start and stop with success" do
    write_seq "1"
    expect(pi.connect).to be true
    pi.stop
    expect(pi.connect).to be false
    expect(read_args).to eq "pigpio_start : (null),(null)\npigpio_stop : 1\n"
  end

  it "start and stop with fail" do
    write_seq "-1"
    expect(pi.connect).to be false
    pi.stop
    expect(pi.connect).to be false
    expect(read_args).to eq "pigpio_start : (null),(null)\npigpio_stop : -1\n"
  end

  it "current_tick" do
    write_seq "1\n123"
    expect(pi.current_tick).to be 123
    expect(read_args).to eq "pigpio_start : (null),(null)\nget_current_tick : 1\n"
  end
  it "hardware_revision" do
    write_seq "1\n123"
    expect(pi.hardware_revision).to be 123
    expect(read_args).to eq "pigpio_start : (null),(null)\nget_hardware_revision : 1\n"
  end
  it "pigpio_version" do
    write_seq "1\n123"
    expect(pi.pigpio_version).to be 123
    expect(read_args).to eq "pigpio_start : (null),(null)\nget_pigpio_version : 1\n"
  end
end
RSpec.describe Pigpio::IF do
  before do
    Pigpio::IF.time_time
  end
  it "error code" do
    read_args
    write_seq "dummy_return"
    expect(Pigpio::IF.pigpio_error(-1)).to eq "dummy_return"
    expect(read_args).to eq "pigpio_error : -1\n"
  end

  it "pigpiod_if_version" do
    write_seq "12"
    expect(Pigpio::IF.pigpiod_if_version).to be 12
  end

  it "time_time" do
    expect(Pigpio::IF.time_time).to be 12.3
  end

end
