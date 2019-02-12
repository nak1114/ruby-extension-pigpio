RSpec.describe Pigpio do
  it "has a version number" do
    expect(Pigpio::VERSION).not_to be nil
  end
  let(:pi){Pigpio.new}
  let(:gpio1){pi.gpio(1)}
  let(:gpio32){pi.gpio(32)}

  it "start and connect" do
    expect(pi.connect).to be true
  end

  it "stop and disconnect" do
    pi.stop
    expect(pi.connect).to be false
  end

  it "does something useful" do
    expect(false).to eq(true)
  end
end
