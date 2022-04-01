#!/bin/sh

./mads starter128.asm -o:starter128.bin
./mads starter256.asm -o:starter256.bin

xxd -i starter128.bin > starter128.h
xxd -i starter256.bin > starter256.h

gcc -Wall -o atr2car atr2car.c
atr2car File.atr File.car -c -512
