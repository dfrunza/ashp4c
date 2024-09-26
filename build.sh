#!/bin/bash

# -std=gnu99  .....  enables the GCC extension 'Referring to a Type with typeof'

CC=""
CC="$CC -g -ggdb -std=gnu99 -fms-extensions -ffreestanding"
CC="$CC -Winline -Wno-write-strings -Wno-unused-function -Wreturn-type -Wall"

gcc $CC -o ashp4c \
  ashp4c.c basic.c arena.c array.c strmap.c map.c \
  lex.c parse.c drypass.c builtin_methods.c scope_hierarchy.c declared_types.c name_bind.c \
  potential_types.c select_type.c \
  -lm

if [ $? -ne 0 ]; then
  exit 1
fi

exit 0
