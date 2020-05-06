#! /bin/bash

set -eu

gcc -c -v syscall.s -fno-builtin -fno-stack-protector
gcc -c -v main.c -fno-builtin -fno-stack-protector
gcc -c -v system.c -fno-builtin -fno-stack-protector
ld main.o system.o syscall.o -nostdlib -o main.out
