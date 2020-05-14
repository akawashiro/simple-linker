#! /bin/bash

make
rm -f a.out
./simple-linker main.o my_system.o my_syscall.o
chmod u+x a.out
readelf -l a.out

