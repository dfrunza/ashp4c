#!/bin/bash

C_FLAGS="-g -ggdb -std=gnu89 -Winline -Wno-write-strings -Wreturn-type -fms-extensions"
L_FLAGS=""

rm -f *.o
gcc $C_FLAGS -I. -o test test.c -L . -lbpf -lelf -lz
gcc $C_FLAGS -I. -o sock_example sock_example.c -L . -lbpf -lelf -lz
