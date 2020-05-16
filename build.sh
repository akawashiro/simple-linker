#! /bin/bash

set -eu

# rm *.o *.out my_system.s main.s

# gcc -c -v my_syscall.s -fno-builtin -fno-stack-protector
as --64 -o my_syscall.o my_syscall.s

# gcc -c -v my_system.c -fno-builtin -fno-stack-protector
/usr/lib/gcc/x86_64-linux-gnu/9/cc1 -fno-builtin -fno-stack-protector -o my_system.s my_system.c
as --64 -o my_system.o my_system.s

# gcc -c -v main.c -fno-builtin -fno-stack-protector
/usr/lib/gcc/x86_64-linux-gnu/9/cc1 -fno-builtin -fno-stack-protector -o main.s main.c
as --64 -o main.o main.s

# gcc -v main.o my_system.o my_syscall.o -nostdlib -o main.out
ld main.o my_system.o my_syscall.o -nostdlib -o main-ld.out
./simple-linker main.o my_system.o my_syscall.o

echo "Build finished!"
