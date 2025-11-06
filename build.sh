#!/bin/bash

# -std=gnu99  ...  Access to the GCC extension 'Referring to a Type with typeof'.

CC=""
CC="$CC -g3 -std=c++11 -ggdb -fpermissive -fms-extensions -ffreestanding"
CC="$CC -Wno-pointer-arith -Wno-sign-compare -Winline -Wno-write-strings -Wno-unused-function -Wreturn-type -Wall"

g++ $CC -o ashp4c \
  ashp4c.c basic.c arena.c array.c strmap.c map.c \
  lex.c ast_tree.c ast_visitor.c scope.c \
  parse.c drypass.c builtin_methods.c scope_hierarchy.c declared_types.c name_bind.c \
  potential_types.c select_type.c \
  -lm

if [ $? -ne 0 ]; then
  exit 1
fi

exit 0
