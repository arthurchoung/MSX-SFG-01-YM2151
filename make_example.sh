#!/bin/bash

set -e

( cd external/z80asm-1.8 && make )

gcc -o convert convert.c
cat example_contra04.vgm | ./convert >data.asm

external/z80asm-1.8/z80asm -o example.rom player.asm

echo
echo "Done"
echo
echo "Use the command:"
echo
echo "openmsx -machine Yamaha_CX5M -cart example.rom -romtype ascii8"
echo
