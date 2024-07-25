#!/bin/bash

# -std=gnu99  .....  enables the GCC extension 'Referring to a Type with typeof'

CC=""
CC="$CC -g -ggdb -std=gnu99 -fms-extensions -ffreestanding"
CC="$CC -Winline -Wno-write-strings -Wno-unused-function -Wreturn-type -Wall"

LD=""

gcc $CC -o ashp4c \
  basic.c arena.c array.c hashmap.c set.c scoping.c \
  lex.c parse.c drypass.c open_scope.c symtable.c type_table.c \
  potential_types.c ashp4c.c \
  $LD -lc -lm

if [ $? -ne 0 ]; then
  exit 1
fi

exit 0
