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

static void
read_source(SourceText* source_text, char* filename)
{
  FILE* f_stream;
  char* text;

  f_stream = fopen(filename, "rb");
  if (!f_stream) {
    error("Could not open file '%s'.", filename);
  }
  fseek(f_stream, 0, SEEK_END);
  int text_size = ftell(f_stream);
  fseek(f_stream, 0, SEEK_SET);
  text = arena_malloc(source_text->storage, (text_size + 1)*sizeof(char));
  fread(text, sizeof(char), text_size, f_stream);
  text[text_size] = '\0';
  fclose(f_stream);
  source_text->text = text;
  source_text->text_size = text_size;
  source_text->filename = filename;
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
parse_cmdline_args(Arena* storage, int arg_count, char* args[])
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
    if (cstr_start_with(args[i], "-")) {
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

int
main(int arg_count, char* args[])
{
  CmdlineArg* cmdline, *filename;
  SourceText source_text = {0};
  Arena storage = {0}, scratch_storage = {0};
  Lexer lexer = {0};
  Parser parser = {0};
  Array* type_array;
  Map* scope_map, *decl_map;
  Map* type_env, *potype_map;

  reserve_memory(500*KILOBYTE);

  cmdline = parse_cmdline_args(&storage, arg_count, args);
  filename = find_unnamed_arg(cmdline);
  if (!filename) {
    printf("<filename> is required.\n");
    exit(1);
  }

  source_text.storage = &scratch_storage;
  read_source(&source_text, filename->value);
  lexer.storage = &storage;
  tokenize(&lexer, &source_text);
  parser.storage = &storage;
  parser.source_file = source_text.filename;
  parser.tokens = lexer.tokens;
  parse(&parser);
  arena_free(&scratch_storage);

  drypass(source_text.filename, parser.program);
  builtin_methods(&storage, source_text.filename, parser.program);
  scope_map = scope_hierarchy(&storage, source_text.filename, parser.program, parser.root_scope);
  decl_map = name_bind(&storage, source_text.filename, parser.program, parser.root_scope,
      scope_map, &type_array);
  type_env = declared_types(&storage, source_text.filename, parser.program, parser.root_scope,
      type_array, scope_map, decl_map);
  potype_map = potential_types(&storage, source_text.filename, parser.program, parser.root_scope,
      scope_map, decl_map, type_env);
  select_type(&storage, source_text.filename, parser.program, parser.root_scope,
      type_array, scope_map, decl_map, type_env, potype_map);

  arena_free(&storage);
  return 0;
}

