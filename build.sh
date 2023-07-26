#!/bin/bash

CC_FLAGS=""
CC_FLAGS="$CC_FLAGS -g -ggdb -std=c11 -fms-extensions -ffreestanding"
CC_FLAGS="$CC_FLAGS -Winline -Wno-write-strings -Wno-unused-function -Wreturn-type -Wall"
CC_FLAGS="$CC_FLAGS -D_GNU_SOURCE"

LD_FLAGS=""
LD_FLAGS="$LD_FLAGS --nostdlib --unresolved-symbols=report-all --static"

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
#gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/type.c 
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/parse.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/pass_ast_id.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/pass_name_decl.c 
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/pass_type_decl.c
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/pass_potential_type.c 
#gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/pass_select_type.c 
gcc $CC_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/ashp4c.c

ld $LD_FLAGS -L$MUSL_LIB -o ashp4c $MUSL_LIB/crt1.o \
  basic.o arena.o array.o hashmap.o scope.o lex.o parse.o \
  pass_ast_id.o pass_name_decl.o pass_type_decl.o pass_potential_type.o \
  ashp4c.o \
  -lc

if [ $? -ne 0 ]; then
  exit 1
fi

popd
exit 0
