#!/bin/bash

C_FLAGS="-g -ggdb -std=gnu89 -Winline -Wno-write-strings -Wreturn-type -fms-extensions"
L_FLAGS=""

SRC=`pwd`
mkdir -p build
rm -fr build/*
pushd build
#gcc $C_FLAGS -I. -o test test.c -L . -lbpf -lelf -lz
#gcc $C_FLAGS -I. -o sock_example sock_example.c -L . -lbpf -lelf -lz

LINUX_INCLUDE=-I/usr/include/x86_64-linux-gnu

#clang $LINUX_INCLUDE -O2 -Wall -target bpf -c xdp_example_drop.c -o xdp_example_drop.o

clang $LINUX_INCLUDE -O2 -Wall -target bpf -c xdp_redirect_user_kern.c -o xdp_redirect_user_kern.o
clang -g -ggdb $LINUX_INCLUDE -O2 -Wall -c bpf_load.c -o bpf_load.o
clang -g -ggdb -static $LINUX_INCLUDE -O2 -Wall xdp_redirect_user.c -o xdp_redirect_user -L . bpf_load.o -lbpf -lelf -lz

clang $LINUX_INCLUDE -O2 -Wall -target bpf -c xdp_parse_kern.c -o xdp_parse_kern.o
popd
