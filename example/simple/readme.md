# Simple IO sample

## Board

![board_image](./board.svg)

## GPIO sample

[Script is here.](./led.rb)

LED will light 3 times.

```sh
$ sudo pigpiod
$ ruby led.rb
```

## PWM sample

[Script is here.](./pwm.rb)

LED brightness will increase while you press the button.

```sh
$ sudo pigpiod
$ ruby pwm.rb
```

## Callback sample

[Script is here.](./callback.rb)

LED will light, while you release the button.

```sh
$ sudo pigpiod
$ ruby callback.rb
```
