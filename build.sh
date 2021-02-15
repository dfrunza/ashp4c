#!/bin/bash

C_FLAGS="-g -ggdb -std=gnu89 -Winline -Wno-write-strings -Wreturn-type -fms-extensions"
L_FLAGS="-static -static-libgcc -static-libstdc++"

SRC=`pwd`
mkdir -p build
rm -f build/*
pushd build
gcc $C_FLAGS -I . -c $SRC/basic.c
gcc $C_FLAGS -I . -c $SRC/arena.c
gcc $C_FLAGS -I . -c $SRC/lex.c
gcc $C_FLAGS -I . -c $SRC/syntax.c 
gcc $C_FLAGS -I . -c $SRC/dump_cst.c 
gcc $C_FLAGS -I . -c $SRC/ast.c 
gcc $C_FLAGS -I. -o dp4c $SRC/dp4c.c $L_FLAGS \
  basic.o arena.o lex.o syntax.o dump_cst.o ast.o
popd
