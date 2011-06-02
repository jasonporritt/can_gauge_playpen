ARDUINO_DIR = /Applications/Arduino.app/Contents/Resources/Java

TARGET 								= loopback_test
MCU    								= atmega328p
F_CPU  								= 16000000
ARDUINO_PORT 					= /dev/tty.usbmodemfd131
AVRDUDE_ARD_BAUDRATE 	= 115200

include ../Arduino.mk
