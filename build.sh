#!/bin/bash

# -std=gnu99  .....  enables the GCC extension 'Referring to a Type with typeof'

CC=""
CC="$CC -g -ggdb -std=gnu99 -fms-extensions -ffreestanding"
CC="$CC -Winline -Wno-write-strings -Wno-unused-function -Wreturn-type -Wall"

gcc $CC -o ashp4c \
  basic.c arena.c array.c hashmap.c map.c \
  lex.c parse.c drypass.c opened_scopes.c symtable.c type_env.c \
  potential_types.c ashp4c.c \
  -lm

if [ $? -ne 0 ]; then
  exit 1
fi

exit 0
