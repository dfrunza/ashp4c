#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   /* exit */
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
  if (!f_stream) {
    error("Could not open file '%s'\n", filename);
  }
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

static Ast*
syntactic_analysis(CmdlineArg* filename, Scope** root_scope, Arena* storage, Arena* tmp_storage)
{
  SourceText source_text;
  UnboundedArray* tokens;
  Ast* program;

  source_text = read_source_text(filename->value, tmp_storage);
  tokens = tokenize_source_text(&source_text, storage);
  program = parse_program(tokens, storage, root_scope);
  return program;
}

static void
semantic_analysis(Ast* program, Scope* root_scope, Arena* storage)
{
  Set* opened_scopes, *enclosing_scopes;
  Set* type_table;
  UnboundedArray* type_array;

  drypass(program);

  opened_scopes = build_open_scope(program, root_scope, storage);
  enclosing_scopes = build_symtable(program, root_scope, opened_scopes, storage);
  type_table = build_type_table(program, root_scope, &type_array,
          opened_scopes, enclosing_scopes, storage);
  build_potential_types(program, root_scope, opened_scopes,
          enclosing_scopes, type_table, storage);
}

int
main(int arg_count, char* args[])
{
  CmdlineArg* cmdline, *filename;
  Arena storage = {}, tmp_storage = {};
  Ast* program;
  Scope* root_scope;

  reserve_page_memory(500*KILOBYTE);

  cmdline = parse_cmdline_args(arg_count, args, &storage);
  filename = find_unnamed_arg(cmdline);
  if (!filename) {
    printf("<filename> is required.\n");
    exit(1);
  }

  program = syntactic_analysis(filename, &root_scope, &storage, &tmp_storage);
  arena_free(&tmp_storage);

  semantic_analysis(program, root_scope, &storage);

  arena_free(&storage);
  return 0;
}

