#include <stdio.h>
#include <stdlib.h>
#include "adt/basic.h"
#include "command_line.h"
#include "frontend/frontend.h"
#include "midend/midend.h"

int main(int arg_count, char* args[])
{
  Arena storage = {}, scratch = {};

  Memory::reserve(500 * KILOBYTE);

  CommandLineArg* cmdline_arg = CommandLineArg::parse_cmdline(&storage, arg_count, args);
  CommandLineArg* filename = cmdline_arg->find_unnamed_arg();
  if (!filename) {
    printf("<filename> is required.\n");
    exit(1);
  }

  SourceText source_text = {};
  source_text.read_source(&storage, &scratch, filename->value);

  Frontend frontend = {};
  frontend.do_analysis(&storage, &scratch, &source_text);

  Midend midend = {};
  midend.do_analysis(&storage, &scratch, &source_text, &frontend);

  return 0;
}
