require 'mkmf'
dir_config('pigpio')
if find_header('pigpiod_if2.h','/usr/include/') && find_library('pigpiod_if2','gpio_read','/usr/lib/')
  create_makefile('pigpio/pigpio')
end