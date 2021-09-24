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
gcc $C_FLAGS -I . -c $SRC/ast.c
gcc $C_FLAGS -I . -c $SRC/build_ast.c
gcc $C_FLAGS -I . -c $SRC/build_symtable.c 
gcc $C_FLAGS -I . -c $SRC/scope_name_resolve.c 
gcc $C_FLAGS -I. -o ashp4c $SRC/ashp4c.c $L_FLAGS \
  basic.o arena.o hash.o symtable.o lex.o ast.o build_ast.o build_symtable.o \
  scope_name_resolve.o \
  -lm
popd
