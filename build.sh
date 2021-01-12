#!/bin/bash

C_FLAGS="-g -ggdb -std=gnu89 -Winline -Wno-write-strings -Wreturn-type -fms-extensions"
L_FLAGS=""

SRC=`pwd`
mkdir -p build
rm -f build/*
pushd build
gcc $C_FLAGS -I . -c $SRC/basic.c
gcc $C_FLAGS -I . -c $SRC/arena.c
gcc $C_FLAGS -I . -c $SRC/lex.c
gcc $C_FLAGS -I . -c $SRC/syntax.c 
gcc $C_FLAGS -I. -o dp4c $SRC/dp4c.c \
  basic.o arena.o lex.o syntax.o
popd
