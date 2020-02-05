@SET SRCS=src/main.c

avr-gcc -Os -mmcu=attiny45 -o bin/main.o -c src/main.c
@echo -----------------------------------------------------------
avr-gcc -mmcu=attiny45 -o bin/main.elf bin/main.o
@echo -----------------------------------------------------------
avr-objcopy -j .text -j .data -O ihex bin/main.elf bin/main.hex
@echo -----------------------------------------------------------
