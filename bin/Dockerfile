FROM ruby:2.5.3
ENV LANG C.UTF-8

# for App
ENV APP_HOME /myapp
WORKDIR $APP_HOME
ADD . $APP_HOME

RUN cd spec/ext \
 && ruby extconf.rb && make \
 && cp libpigpiod_if2.so /usr/lib \
 && cp pigpio.h /usr/include \
 && cp pigpiod_if2.h /usr/include \
 && cd ../../ \
 && bundle install \
 && rake compile \
 && cp Gemfile.lock /tmp