require "pigpio"
require "rmagick"

class SSD1306
  attr_reader :width, :height, :buf
  def initialize(i2c)
    @width = 128
    @height = 64
    @pages = (@height / 8)
    @i2c = i2c
    @buf = Array.new(@width * @pages, 0)
  end

  def image2buf(image)
    @buf = []
    (@height / 8).times do |row|
      @width.times do |x|
        c = 0
        8.times do |y|
          px = image.pixel_color(x, y + row * 8)
          c |= (px.red / (65536 / 2)) << y
        end
        @buf.push c
      end
    end
    @buf
  end

  def reset
    com(DISPLAYOFF)
    com(SETDISPLAYCLOCKDIV, 0x80)
    com(SETMULTIPLEX, 0x3F)
    com(SETDISPLAYOFFSET, 0x0)
    com(SETSTARTLINE | 0x0)
    com(CHARGEPUMP, 0x14)
    com(MEMORYMODE, 0x00)
    com(SEGREMAP | 0x1)
    com(COMSCANDEC)
    com(SETCOMPINS, 0x12)
    com(SETCONTRAST, 0xCF)
    com(SETPRECHARGE, 0xF1)
    com(SETVCOMDETECT, 0x40)
    com(DISPLAYALLON_RESUME)
    com(NORMALDISPLAY)
    com(DISPLAYON)
  end

  def display
    com(COLUMNADDR, 0, @width - 1) # Column end address.
    com(PAGEADDR, 0, @pages - 1) # Page end address.(height/8)
    control = 0x40 # Co = 0, DC = 0
    @buf.each_slice(32) { |b|
      @i2c.write_i2c_block_data(control, b.pack("c*"))
    }
  end

  def com(*c)
    control = c.size == 1 ? 0x80 : 0x00
    @i2c.write_i2c_block_data(control, c.pack("c*"))
  end

  I2C_ADDRESS = 0x3C # 011110+SA0+RW - 0x3C or 0x3D
  SETCONTRAST = 0x81
  DISPLAYALLON_RESUME = 0xA4
  DISPLAYALLON = 0xA5
  NORMALDISPLAY = 0xA6
  INVERTDISPLAY = 0xA7
  DISPLAYOFF = 0xAE
  DISPLAYON = 0xAF
  SETDISPLAYOFFSET = 0xD3
  SETCOMPINS = 0xDA
  SETVCOMDETECT = 0xDB
  SETDISPLAYCLOCKDIV = 0xD5
  SETPRECHARGE = 0xD9
  SETMULTIPLEX = 0xA8
  SETLOWCOLUMN = 0x00
  SETHIGHCOLUMN = 0x10
  SETSTARTLINE = 0x40
  MEMORYMODE = 0x20
  COLUMNADDR = 0x21
  PAGEADDR = 0x22
  COMSCANINC = 0xC0
  COMSCANDEC = 0xC8
  SEGREMAP = 0xA0
  CHARGEPUMP = 0x8D
  EXTERNALVCC = 0x1
  SWITCHCAPVCC = 0x2
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

i2c = pi.i2c(1, 0x3c)
if i2c.handle < 0
  p "i2c error" + i2c.handle.to_s
  pi.stop
  exit(-2)
end

begin
  d = SSD1306.new(i2c)
  d.reset

  image = Magick::Image.new(d.width, d.height) {
    self.background_color = "black"
  }
  draw = Magick::Draw.new
  draw.font = "../font/misaki_gothic.ttf" # Little Limit (http://littlelimit.net/)
  draw.pointsize = 8
  draw.fill = "white"
  draw.gravity = Magick::NorthWestGravity
  str = DATA.read
  draw.annotate(image, 0, 0, 0, 0, word_wrap(draw, str, d.width))

  d.image2buf(image)
  d.display
rescue => e
  puts e.full_message
ensure
  i2c.close
  pi.stop
end
__END__
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.