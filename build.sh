#!/bin/bash

CC_FLAGS="-g -ggdb -std=gnu99 -fms-extensions -ffreestanding"
CC_FLAGS="$CC_FLAGS -Winline -Wno-write-strings -Wno-unused-function -Wreturn-type -Wall"
LD_FLAGS="--nostdlib --unresolved-symbols=report-all --static"

MUSL_INCLUDE=/usr/local/musl/include
MUSL_LIB=/usr/local/musl/lib
SRC=`pwd`

mkdir -p build
rm -f build/*
pushd build

gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/basic.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/arena.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/array.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/hashmap.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/scope.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/lex.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/parse.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/node_id.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/name_decl.c 
#gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/type.c 
#gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/type_decl.c 
#gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/potential_type.c 
#gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/select_type.c 
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/ashp4c.c

ld $LD_FLAGS -L$MUSL_LIB -o ashp4c $MUSL_LIB/crt1.o \
  basic.o arena.o array.o hashmap.o scope.o lex.o parse.o \
  node_id.o name_decl.o ashp4c.o \
  -lc

if [ $? -ne 0 ]; then
  exit 1
fi

popd
exit 0
