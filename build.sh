#!/bin/bash

CC_FLAGS=""
CC_FLAGS="$CC_FLAGS -g -ggdb -std=c99 -fms-extensions -ffreestanding"
CC_FLAGS="$CC_FLAGS -Winline -Wno-write-strings -Wno-unused-function -Wreturn-type -Wall"
CC_FLAGS="$CC_FLAGS -D_GNU_SOURCE"

LD_FLAGS=""
LD_FLAGS="$LD_FLAGS --nostdlib --unresolved-symbols=report-all --static"

INC=/usr/local/musl/include
LIB=/usr/local/musl/lib
SRC=`pwd`

mkdir -p build
rm -f build/*
pushd build

gcc $CC_FLAGS -I$INC -c $SRC/basic.c
gcc $CC_FLAGS -I$INC -c $SRC/arena.c
gcc $CC_FLAGS -I$INC -c $SRC/array.c
gcc $CC_FLAGS -I$INC -c $SRC/hashmap.c
gcc $CC_FLAGS -I$INC -c $SRC/scoping.c
gcc $CC_FLAGS -I$INC -c $SRC/lex.c
gcc $CC_FLAGS -I$INC -c $SRC/parse.c
gcc $CC_FLAGS -I$INC -c $SRC/drypass.c
gcc $CC_FLAGS -I$INC -c $SRC/open_scope.c 
#gcc $CC_FLAGS -I$INC -c $SRC/name_decl.c 
gcc $CC_FLAGS -I$INC -c $SRC/type_decl.c
gcc $CC_FLAGS -I$INC -c $SRC/idref_type.c 
gcc $CC_FLAGS -I$INC -c $SRC/ashp4c.c

ld $LD_FLAGS -L$LIB -o ashp4c $LIB/crt1.o \
  basic.o arena.o array.o hashmap.o scoping.o lex.o parse.o \
  drypass.o open_scope.o type_decl.o idref_type.o \
  ashp4c.o \
  -lc

if [ $? -ne 0 ]; then
  exit 1
fi

popd
exit 0
