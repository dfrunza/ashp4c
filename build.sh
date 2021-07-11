#!/bin/bash

C_FLAGS="-g -ggdb -std=gnu89 -Winline -Wno-write-strings -Wreturn-type -fms-extensions"
L_FLAGS="-static -static-libgcc -static-libstdc++"

SRC=`pwd`
mkdir -p build
rm -f build/*
pushd build
gcc $C_FLAGS -I . -c $SRC/basic.c
gcc $C_FLAGS -I . -c $SRC/arena.c
gcc $C_FLAGS -I . -c $SRC/hash.c
gcc $C_FLAGS -I . -c $SRC/symtable.c
gcc $C_FLAGS -I . -c $SRC/lex.c
gcc $C_FLAGS -I . -c $SRC/build_ast.c
gcc $C_FLAGS -I . -c $SRC/print_ast.c 
gcc $C_FLAGS -I. -o dp4c $SRC/dp4c.c $L_FLAGS \
  basic.o arena.o hash.o symtable.o lex.o build_ast.o print_ast.o -lm
popd
