#!/bin/bash

# -std=gnu99  ...  Access to the GCC extension 'Referring to a Type with typeof'.

CC=""
CC="$CC -g3 -std=c++11 -ggdb -fpermissive -fms-extensions -ffreestanding"
CC="$CC -Wno-pointer-arith -Wno-sign-compare -Wno-nonnull-compare -Winline -Wno-write-strings -Wno-unused-function -Wreturn-type -Wall"

g++ $CC -o ashp4c \
  ashp4c.cpp basic.cpp arena.cpp array.cpp strmap.cpp map.cpp \
  lex.cpp ast_tree.cpp ast_visitor.cpp scope.cpp \
  parse.cpp drypass.cpp builtin_methods.cpp scope_hierarchy.cpp declared_types.cpp name_bind.cpp \
  potential_types.cpp select_type.cpp \
  -lm

if [ $? -ne 0 ]; then
  exit 1
fi

exit 0
