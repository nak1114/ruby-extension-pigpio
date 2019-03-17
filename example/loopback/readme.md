# I/F Loop back sample

![board_image](./uart.svg)

## UART(serial) loop back

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

## SPI loop back

[Script is here.](./spi.rb)

Console will echo back below.

```sh
$ sudo pigpiod
$ ruby spi.rb
"abcdef"
```
