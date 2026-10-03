#!/bin/sh
# compile program to hex ready for flashing
avr-gcc -Wall -Os -mmcu=attiny85 main.c -o the_program.elf
avr-objcopy -v -j .text -j .data -O ihex the_program.elf the_program.hex
rm the_program.elf
