require "pigpio"
require "rmagick"

class SharpMemoryDisplay
  attr_reader :width, :height, :buf
  CMD_BIT_WRITECMD = 0x80
  CMD_BIT_VCOM = 0x40
  CMD_BIT_CLEAR = 0x20
  DUMMY_DATA = 0x00

  BitRverseChar = 256.times.map do |v|
    8.times.inject(0) { |ret, i| ret |= (v & 0x01 << i) != 0 ? 0x80 >> i : 0 }
  end

  def initialize(spi, com, cs, width, height)
    @width = width
    @height = height
    @spi = spi
    @com = com
    @cs = cs
    @buf = Array.new((@width / 8) * height, 0)
    @com.mode = PI_OUTPUT
    @com.pud = PI_PUD_OFF
    @com.write 0
    @cs.mode = PI_OUTPUT
    @cs.pud = PI_PUD_OFF
    @cs.write 0
    @wave = generate_com @com
  end

  def stop
    @wave.tx_stop if @wave
    @wave.clear if @wave
    @com.write 0
  end

  def display(num = 0)
    com(*@buf)
  end

  def clear
    com(CMD_BIT_CLEAR, DUMMY_DATA)
  end

  def com(*data)
    @cs.write 1
    @spi.write data.pack("c*")
    @cs.write 0
  end

  def generate_com pin
    mark = 0x01 << pin.gpio
    wave = Pigpio::Wave.new(pin.pi)
    wave.add_new
    wave.add_generic([
      wave.pulse(mark, 0x00, 3),
      wave.pulse(0x00, mark, 1000000 - 10000)
    ])
    wid = wave.create
    wave.chain([255, 0, wid, 255, 3])
    wave
  end

  def image2buf(image)
    @buf = [CMD_BIT_WRITECMD]
    th = 65536 / 2
    @height.times do |y|
      @buf.push BitRverseChar[y + 1]
      (@width / 8).times do |col|
        ret = 8.times.inject(0) do |c, x|
          px = image.pixel_color(x + col * 8, y)
          c << 1 | (px.red < th ? 0 : 1)
        end
        @buf.push ret
      end
      @buf.push DUMMY_DATA
    end
    @buf.push DUMMY_DATA
  end
end

class LS013B4DN04 < SharpMemoryDisplay
  def initialize(spi, com, cs, width = 96, height = 96)
    super
  end
end

class LS013B7DH05 < SharpMemoryDisplay
  def initialize(spi, com, cs, width = 144, height = 168)
    super
  end
end

class LS027B4DH01 < SharpMemoryDisplay
  def initialize(spi, com, cs, width = 400, height = 240)
    super
  end
end

def word_wrap(draw, str, width)
  str.each_line.map do |v|
    v.chomp.size.times.with_object([0]) do |i, ins|
      ins.unshift(i) if draw.get_type_metrics(v[ins.first..i]).width > width
    end[0..-2].each { |i| v.insert(i, "\n") }
    v
  end.join("")
end

include Pigpio::Constant
pi = Pigpio.new
unless pi.connect
  p "error" + pi.pi.to_s
  exit(-1)
end

com = pi.gpio(17)
cs = pi.gpio(22)

spi = pi.spi(0)
if spi.handle < 0
  p "spi error" + spi.handle.to_s
  pi.stop
  exit(-2)
end

begin
  d = LS013B7DH05.new(spi, com, cs)

  image = Magick::Image.new(d.width, d.height) {
    self.background_color = "white"
  }
  draw = Magick::Draw.new
  draw.font = "./example/font/misaki_gothic.ttf" # Little Limit (http://littlelimit.net/)
  draw.pointsize = 8
  draw.fill = "black"
  draw.gravity = Magick::NorthWestGravity
  str = word_wrap(draw, DATA.read, d.width)
  draw.annotate(image, 0, 0, 0, 0, str)

  d.image2buf(image)
  d.clear
  d.display
rescue => e
  puts e.full_message
ensure
  d.stop if d
  spi.close
  pi.stop
end
__END__
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.