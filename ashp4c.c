#include <memory.h>  /* memset */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   /* exit */
#include <stdarg.h>   /* va_list, va_start, va_end */
#include "foundation.h"
#include "frontend.h"
#include "ashp4c.h"

static Arena main_storage = {};
static SourceText source_text = {};

typedef struct CmdlineArg {
  char* name;
  char* value;
  struct CmdlineArg* next_arg;
} CmdlineArg;

SourceText*
read_source_text(char* filename, Arena* storage)
{
  FILE* f_stream = fopen(filename, "rb");
  fseek(f_stream, 0, SEEK_END);
  int text_size = ftell(f_stream);
  fseek(f_stream, 0, SEEK_SET);
  char* text = arena_malloc(storage, (text_size + 1)*sizeof(char));
  fread(text, sizeof(char), text_size, f_stream);
  text[text_size] = '\0';
  fclose(f_stream);
  source_text.text = text;
  source_text.text_size = text_size;
  source_text.filename = filename;
  return &source_text;
}

static CmdlineArg*
find_unnamed_arg(CmdlineArg* args)
{
  CmdlineArg* unnamed_arg = 0;
  CmdlineArg* arg = args;
  while (arg) {
    if (!arg->name) {
      unnamed_arg = arg;
      break;
    }
    arg = arg->next_arg;
  }
  return unnamed_arg;
}

#if 0
static CmdlineArg*
find_named_arg(char* name, CmdlineArg* args)
{
  CmdlineArg* named_arg = 0;
  CmdlineArg* arg = args;
  while (arg) {
    if (arg->name && cstr_match(name, arg->name)) {
      named_arg = arg;
      break;
    }
    arg = arg->next_arg;
  }
  return named_arg;
}
#endif

static CmdlineArg*
parse_cmdline_args(int arg_count, char* args[])
{
  CmdlineArg* arg_list = 0;
  if (arg_count <= 1) {
    return arg_list;
  }
  
  CmdlineArg sentinel_arg = {};
  CmdlineArg* prev_arg = &sentinel_arg;
  int i = 1;
  while (i < arg_count) {
    CmdlineArg* cmdline_arg = arena_malloc(&main_storage, sizeof(CmdlineArg));
    if (cstr_start_with(args[i], "--")) {
      char* raw_arg = args[i] + 2;  /* skip the `--` prefix */
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

int
main(int arg_count, char* args[])
{
  reserve_page_memory(250*KILOBYTE);

  CmdlineArg* cmdline_args = parse_cmdline_args(arg_count, args);
  CmdlineArg* filename_arg = find_unnamed_arg(cmdline_args);
  if (!filename_arg) {
    printf("<filename> is required.\n");
    exit(1);
  }
  Arena text_storage = {};
  SourceText* source_text = read_source_text(filename_arg->value, &text_storage);
  UnboundedArray* tokens = tokenize_source_text(source_text, &main_storage);

  Scope* root_scope;
  Ast* ast = parse_program(tokens, &root_scope, &main_storage);
  arena_free(&text_storage);

  drypass(ast); /* sanity check */
  Hashmap* scope_map, *field_map;
  pass_name_decl(ast, root_scope, &scope_map, &field_map, &main_storage);
  Hashmap* type_table = pass_type_decl(ast, &main_storage);
  pass_potential_types(ast, root_scope, scope_map, type_table, &main_storage);

  arena_free(&main_storage);
  return 0;
}

