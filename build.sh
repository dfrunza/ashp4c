#!/bin/bash

CC_FLAGS="-g -ggdb -std=gnu99 -Winline -Wno-write-strings -Wreturn-type -fms-extensions -ffreestanding"
LD_FLAGS="--nostdlib --unresolved-symbols=report-all --static"

MUSL_INCLUDE=/usr/local/include
MUSL_LIB=/usr/local/lib
SRC=`pwd`

mkdir -p build
rm -f build/*
pushd build

gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/ashp4c.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/basic.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/arena.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/hashmap.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/symtable.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/lex.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/build_ast.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/build_symtable.c 
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/build_type.c 
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/resolve_nameref.c 

ld $LD_FLAGS -L$MUSL_LIB -o ashp4c \
  $MUSL_LIB/crt1.o \
  ashp4c.o basic.o arena.o hashmap.o symtable.o lex.o build_ast.o build_symtable.o \
  build_type.o resolve_nameref.o \
  -lc

popd
