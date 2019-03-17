# Pigpio

This gem is a ruby binding to a [pigpio library](http://abyz.me.uk/rpi/pigpio/).

## Installation

First, install pigpio libary.

```sh
$ sudo apt-get update
$ sudo apt-get install pigpio
```

Next, add this line to your application's Gemfile:

```ruby
gem 'pigpio'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ sudo gem install pigpio

## Usage

First, create a circuit.

![board](./example/simple/board.svg)

Next, write a script.

```ruby
require "pigpio"
include Pigpio::Constant

pi=Pigpio.new
unless pi.connect
  exit -1
end

led = pi.gpio(4)
led.mode = PI_OUTPUT
led.pud = PI_PUD_OFF
3.times do |i|
  led.write 1
  sleep 1
  led.write 0
  sleep 1
end
pi.stop
```

Then, run deamon and script.

```sh
$ sudo pigpiod
$ ruby example_led.rb
```

More sample for this circuit. : [Hare](./example/simple/readme.md)
More sample for other circuit. : [Hare](./example/readme.md)

More document : [GitHub Pages](https://nak1114.github.io/ruby-extension-pigpio/)

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake spec` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/nak1114/pigpio. This project is intended to be a safe, welcoming space for collaboration, and contributors are expected to adhere to the [Contributor Covenant](http://contributor-covenant.org) code of conduct.

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).

## Code of Conduct

Everyone interacting in the Pigpio project’s codebases, issue trackers, chat rooms and mailing lists is expected to follow the [code of conduct](https://github.com/nak1114/pigpio/blob/master/CODE_OF_CONDUCT.md).
