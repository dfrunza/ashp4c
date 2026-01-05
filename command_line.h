#pragma once
#include "adt/cstring.h"
#include "memory/arena.h"

struct CommandLineArg {
  char* name;
  char* value;
  CommandLineArg* next_arg;

  static CommandLineArg* parse_cmdline(Arena* storage, int arg_count, char* args[]);
  CommandLineArg* find_named_arg(char* name);
  CommandLineArg* find_unnamed_arg();
};
