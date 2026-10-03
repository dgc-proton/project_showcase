#!/bin/sh
# flash chip with the .hex program, then remove the file
avrdude -c tigard -p attiny85 -v -v -P usb:0403:6010 -U flash:w:the_program.hex
rm the_program.hex
# upload using tigard programmer, part number attiny85, verbose output, baud rate 9600, memory operation write to flash the .hex file
# avrdude -c arduino_as_isp -p attiny85 -v -v -P usb:2341:0043 -U flash:w:test_blink_led.hex
