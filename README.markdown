# Canbus Gauge for a 2008 Mazdaspeed 3

## The Goal

A passive Canbus-based gauge with interesting information for a Mazdaspeed 3. While it would be easier to poll on the bus using OBDII, I found the refresh rate much better with a passive, bus-snooping solution.

There is an amazing amount of data flying around on our modern cars and I'm curious to learn what is present.

This depends on my version of the Canbus library hosted here on GitHub (NewCanbus).

## The Hardware

The hardware:
  * An Arduino Uno
  * [The CAN bus shield sold by SparkFun](http://www.sparkfun.com/products/10039)
  * [A serial-enabled 20x4 LCD](http://www.sparkfun.com/products/9568)
