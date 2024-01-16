#include <memory.h>  /* memset */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   /* exit */
#include <stdarg.h>   /* va_list, va_start, va_end */
#include "foundation.h"
#include "frontend.h"
#include "ashp4c.h"

typedef struct CmdlineArg {
  char* name;
  char* value;
  struct CmdlineArg* next_arg;
} CmdlineArg;

SourceText
read_source_text(char* filename, Arena* storage)
{
  SourceText source_text = {};
  FILE* f_stream;
  char* text;

  f_stream = fopen(filename, "rb");
  fseek(f_stream, 0, SEEK_END);
  int text_size = ftell(f_stream);
  fseek(f_stream, 0, SEEK_SET);
  text = arena_malloc(storage, (text_size + 1)*sizeof(char));
  fread(text, sizeof(char), text_size, f_stream);
  text[text_size] = '\0';
  fclose(f_stream);
  source_text.text = text;
  source_text.text_size = text_size;
  source_text.filename = filename;
  return source_text;
}

static CmdlineArg*
find_unnamed_arg(CmdlineArg* args)
{
  CmdlineArg* unnamed_arg = 0;
  CmdlineArg* arg;
  
  arg = args;
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
parse_cmdline_args(int arg_count, char* args[], Arena* storage)
{
  CmdlineArg* arg_list = 0;
  CmdlineArg *prev_arg, *cmdline_arg;
  CmdlineArg sentinel_arg = {};
  char* raw_arg;

  if (arg_count <= 1) {
    return arg_list;
  }
  prev_arg = &sentinel_arg;
  int i = 1;
  while (i < arg_count) {
    cmdline_arg = arena_malloc(storage, sizeof(CmdlineArg));
    if (cstr_start_with(args[i], "--")) {
      raw_arg = args[i] + 2;  /* skip the `--` prefix */
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
  CmdlineArg* cmdline_args, *filename_arg;
  SourceText source_text;
  Arena text_storage = {}, main_storage = {};
  UnboundedArray* tokens;
  Scope* root_scope;
  Ast* program;
  Hashmap* opened_scopes, *type_table;
  UnboundedArray* type_array;

  reserve_page_memory(500*KILOBYTE);

  cmdline_args = parse_cmdline_args(arg_count, args, &main_storage);
  filename_arg = find_unnamed_arg(cmdline_args);
  if (!filename_arg) {
    printf("<filename> is required.\n");
    exit(1);
  }

  source_text = read_source_text(filename_arg->value, &text_storage);
  tokens = tokenize_source_text(&source_text, &main_storage);
  program = parse_program(tokens, &main_storage, &root_scope);
  arena_free(&text_storage);

  drypass(program);
  opened_scopes = build_open_scope(program, root_scope, &main_storage);
  build_symtable(program, root_scope, opened_scopes, &main_storage);
  type_table = build_type_table(program, root_scope, &type_array, opened_scopes, &main_storage);
  resolve_type_xref(type_table, type_array);

  Set set = {};
  Type* type;
  int N = 16;
  SetMember* m;

  if (type_array->elem_count < N) {
    N = type_array->elem_count;
  }
  for (int i = 0; i < N; i++) {
    type = (Type*)array_get_elem(type_array, i, sizeof(Type));
    set_add_member(&set, &main_storage, (uint64_t)type, 0);
  }
  m = set_get_member(&set, (uint64_t)array_get_elem(type_array, 4, sizeof(Type)));
  m = set_get_member(&set, (uint64_t)array_get_elem(type_array, 17, sizeof(Type)));

  arena_free(&main_storage);
  return 0;
}

