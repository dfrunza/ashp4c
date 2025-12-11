#pragma once
#include <cstring.h>
#include <arena.h>

struct CommandLineArg {
  char* name;
  char* value;
  CommandLineArg* next_arg;

  static CommandLineArg* parse_cmdline_args(Arena* storage, int arg_count, char* args[])
  {
    CommandLineArg* arg_list = 0;
    CommandLineArg sentinel_arg = {};
    char* raw_arg;

    if (arg_count <= 1) {
      return arg_list;
    }
    CommandLineArg* prev_arg = &sentinel_arg;
    int i = 1;
    while (i < arg_count) {
      CommandLineArg* cmdline_arg = storage->allocate<CommandLineArg>();
      if (cstring::start_with(args[i], "-")) {
        raw_arg = args[i] + 1;  /* skip the `-` prefix */
        cmdline_arg->name = raw_arg;
      } else {
        cmdline_arg->value = args[i];
      }
      prev_arg->next_arg = cmdline_arg;
      prev_arg = cmdline_arg;
      i += 1;
    }
    arg_list = sentinel_arg.next_arg;
    return arg_list;
  }

  CommandLineArg* find_named_arg(char* name)
  {
    CommandLineArg* named_arg = 0;
    CommandLineArg* arg = this;

    while (arg) {
      if (arg->name && cstring::match(name, arg->name)) {
        named_arg = arg;
        break;
      }
      arg = arg->next_arg;
    }
    return named_arg;
  }

  CommandLineArg* find_unnamed_arg()
  {
    CommandLineArg* unnamed_arg = 0;
    CommandLineArg* arg = this;

    while (arg) {
      if (!arg->name) {
        unnamed_arg = arg;
        break;
      }
      arg = arg->next_arg;
    }
    return unnamed_arg;
  }
};

