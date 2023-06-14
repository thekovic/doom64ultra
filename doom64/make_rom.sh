#!/bin/sh

export N64_LIBGCCDIR="/opt/crashsdk/lib/gcc/mips64-elf/12.2.0"
export PATH=$PATH:/opt/crashsdk/bin

make
