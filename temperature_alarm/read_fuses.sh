# read all fuses and output in readable format
avrdude -c tigard -p attiny85 -v -P usb:0403:6010 -U hfuse:r:-:r -U lfuse:r:-:r -U efuse:r:-:r | od -t x1
