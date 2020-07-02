#!/bin/bash

C_FLAGS="-g -ggdb -std=gnu89 -m32 -Winline -Wno-write-strings -Wreturn-type -fms-extensions"
L_FLAGS=""

rm -f *.o
gcc $C_FLAGS -I . -c basic.c
gcc $C_FLAGS -I . -c arena.c
gcc $C_FLAGS -I . -c lex.c
gcc $C_FLAGS -I . -c syntax.c
gcc $C_FLAGS -I . -c symtab.c 
gcc $C_FLAGS -I. -o dp4c dp4c.c basic.o arena.o lex.o syntax.o symtab.o
