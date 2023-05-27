#!/bin/bash

CC_FLAGS="-g -ggdb -std=gnu99 -fms-extensions -ffreestanding"
CC_FLAGS="$CC_FLAGS -Winline -Wno-write-strings -Wreturn-type -Wall"
LD_FLAGS="--nostdlib --unresolved-symbols=report-all --static"

MUSL_INCLUDE=/usr/local/musl/include
MUSL_LIB=/usr/local/musl/lib
SRC=`pwd`

mkdir -p build
rm -f build/*
pushd build

gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/ashp4c.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/basic.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/arena.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/hashmap.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/symbol_table.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/lex.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/parse.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/name_decl.c 
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/collect_type.c 
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/select_type.c 

ld $LD_FLAGS -L$MUSL_LIB -o ashp4c $MUSL_LIB/crt1.o \
  ashp4c.o basic.o arena.o hashmap.o symbol_table.o lex.o parse.o name_decl.o \
  collect_type.o select_type.o \
  -lc

if [ $? -ne 0 ]; then
  exit 1
fi

popd
exit 0
