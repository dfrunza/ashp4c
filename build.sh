#!/bin/bash

C_FLAGS="-g -ggdb -std=gnu89 -Winline -Wno-write-strings -Wreturn-type -fms-extensions"
L_FLAGS="-static -static-libgcc -static-libstdc++"

MUSL_INCLUDE=/usr/local/include
MUSL_LIB=/usr/local/lib
SRC=`pwd`
mkdir -p build
rm -f build/*
pushd build

gcc $C_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/ashp4c.c
gcc $C_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/basic.c
gcc $C_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/arena.c
gcc $C_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/hashmap.c
gcc $C_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/symtable.c
gcc $C_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/lex.c
gcc $C_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/build_ast.c
gcc $C_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/build_symtable.c 
gcc $C_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/scope_name_resolve.c 
gcc $C_FLAGS -I$MUSL_INCLUDE -I . -c $SRC/objdesc_name_resolve.c 

ld --nostdlib -L=$MUSL_LIB --unresolved-symbols=report-all --static -o ashp4c \
  $MUSL_LIB/crt1.o \
  ashp4c.o basic.o arena.o hashmap.o symtable.o lex.o build_ast.o build_symtable.o \
  scope_name_resolve.o objdesc_name_resolve.o \
  -lc
popd
