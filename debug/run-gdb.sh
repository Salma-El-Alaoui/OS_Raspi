#!/bin/sh

arm-none-eabi-gdb ../bin/kernel.elf -x gdbinit

reset
