require "pigpio"
require "rmagick"

class EPD2IN13B
  attr_reader :width, :height, :buf_red, :buf_black
  def initialize(spi, rst, dc, busy, cs, width = 104, height = 212)
    @width = width
    @height = height
    @spi = spi
    @rst = rst
    @dc = dc
    @cs = cs
    @busy = busy
    @buf_black = Array.new((@width / 8) * height, 0)
    @buf_red = Array.new((@width / 8) * height, 0)
    @rst.mode = PI_OUTPUT
    @rst.pud = PI_PUD_OFF
    @rst.write 1
    @dc.mode = PI_OUTPUT
    @dc.pud = PI_PUD_OFF
    @dc.write 1
    @cs.mode = PI_OUTPUT
    @cs.pud = PI_PUD_OFF
    @cs.write 1
    @busy.mode = PI_INPUT
    @busy.pud = PI_PUD_OFF
  end

  def reset
    @rst.write 1
    sleep 0.2
    @rst.write 0
    sleep 0.2
    @rst.write 1

    com(BOOSTER_SOFT_START, 0x17, 0x17, 0x17)
    com(POWER_ON)
    wait_busy
    com(PANEL_SETTING, 0x8F)
    com(VCOM_AND_DATA_INTERVAL_SETTING, 0xF0)
    com(RESOLUTION_SETTING, @width & 0xff, @height >> 8, @height & 0xff)
  end

  def display
    com(DATA_START_TRANSMISSION_1, *@buf_black)
    com(DATA_STOP)

    com(DATA_START_TRANSMISSION_2, *@buf_red)
    com(DATA_STOP)

    com(DISPLAY_REFRESH)
    wait_busy
  end

  def wait_busy
    sleep 0.1 while @busy.read == 0
  end

  def com(command, *arg)
    @cs.write 0

    @dc.write 0 # command
    @spi.write [command].pack("c*")

    @dc.write 1 # data
    @spi.write arg.pack("c*") if arg.size > 0

    @cs.write 1
  end

  def image2buf(image)
    @buf_black = []
    @buf_red = []
    th = 65536 / 2
    @height.times do |y|
      (@width / 8).times do |col|
        black, red = 8.times.with_object([0, 0]) do |x, c|
          px = image.pixel_color(x + col * 8, y)
          c[0] = c[0] << 1 | (px.red < th && px.green < th ? 0 : 1)
          c[1] = c[1] << 1 | (px.red >= th && px.green < th ? 0 : 1)
        end
        @buf_black.push black
        @buf_red.push red
      end
    end
  end

  # Display resolution
  EPD_WIDTH = 104
  EPD_HEIGHT = 212

  # EPD2IN13B commands
  PANEL_SETTING = 0x00
  POWER_SETTING = 0x01
  POWER_OFF = 0x02
  POWER_OFF_SEQUENCE_SETTING = 0x03
  POWER_ON = 0x04
  POWER_ON_MEASURE = 0x05
  BOOSTER_SOFT_START = 0x06
  DEEP_SLEEP = 0x07
  DATA_START_TRANSMISSION_1 = 0x10
  DATA_STOP = 0x11
  DISPLAY_REFRESH = 0x12
  DATA_START_TRANSMISSION_2 = 0x13
  VCOM_LUT = 0x20
  W2W_LUT = 0x21
  B2W_LUT = 0x22
  W2B_LUT = 0x23
  B2B_LUT = 0x24
  PLL_CONTROL = 0x30
  TEMPERATURE_SENSOR_CALIBRATION = 0x40
  TEMPERATURE_SENSOR_SELECTION = 0x41
  TEMPERATURE_SENSOR_WRITE = 0x42
  TEMPERATURE_SENSOR_READ = 0x43
  VCOM_AND_DATA_INTERVAL_SETTING = 0x50
  LOW_POWER_DETECTION = 0x51
  TCON_SETTING = 0x60
  RESOLUTION_SETTING = 0x61
  GET_STATUS = 0x71
  AUTO_MEASURE_VCOM = 0x80
  VCOM_VALUE = 0x81
  VCM_DC_SETTING = 0x82
  PARTIAL_WINDOW = 0x90
  PARTIAL_IN = 0x91
  PARTIAL_OUT = 0x92
  PROGRAM_MODE = 0xA0
  ACTIVE_PROGRAM = 0xA1
  READ_OTP_DATA = 0xA2
  POWER_SAVING = 0xE3
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

rst = pi.gpio(17)
dc = pi.gpio(25)
busy = pi.gpio(24)
cs = pi.gpio(8)

spi = pi.spi(0)
if spi.handle < 0
  p "spi error" + spi.handle.to_s
  pi.stop
  exit(-2)
end

begin
  d = EPD2IN13B.new(spi, rst, dc, busy, cs)
  d.reset

  image = Magick::Image.new(d.width, d.height) {
    self.background_color = "white"
  }
  draw = Magick::Draw.new
  draw.font = "../font/misaki_gothic.ttf" # Little Limit (http://littlelimit.net/)
  draw.pointsize = 8
  draw.fill = "black"
  draw.gravity = Magick::NorthWestGravity
  str = word_wrap(draw, DATA.read, d.width)
  draw.annotate(image, 0, 0, 0, 0, str)
  draw.fill = "red"
  draw.annotate(image, 0, 0, 0, str.lines.size * 8, str)

  d.image2buf(image)
  d.display
rescue => e
  puts e.full_message
ensure
  spi.close
  pi.stop
end
__END__
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.