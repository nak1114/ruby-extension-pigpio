RSpec.describe Pigpio do
  it "has a version number" do
    expect(Pigpio::VERSION).not_to be nil
  end
  let(:pi){Pigpio.new}

  it "error code" do
    read_args
    expect(Pigpio::IF.pigpio_error(-1)).to eq "ret"
    expect(read_args).to eq "pigpio_error : -1\n"
  end

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

  it "does something useful" do
    expect(false).to eq(true)
  end
end
