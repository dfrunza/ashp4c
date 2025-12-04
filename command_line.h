#pragma once
#include <arena.h>

struct CommandLineArg {
  char* name;
  char* value;
  CommandLineArg* next_arg;

  static CommandLineArg* parse_cmdline_args(Arena* storage, int arg_count, char* args[]);
  CommandLineArg* find_unnamed_arg();
  CommandLineArg* find_named_arg(char* name);
};

