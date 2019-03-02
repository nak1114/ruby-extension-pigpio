# I/F Loop back sample

## UART(serial) loop back

![board_image](./uart.svg)

[Script is here.](./uart.rb)

Console will echo back below.

```sh
$ sudo pigpiod
$ ruby uart.rb
"abcdef"
"z"
"y"
"x"
""
```
